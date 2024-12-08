#include "AppMsgQueue.h"

#include "common/common.h"

#include "SDL3/SDL_events.h"
#include "concurrentqueue.h"

struct AppMsgQueueImpl;

namespace
{
const uint32_t s_appQueueNotificationSdlEventType = SDL_RegisterEvents(1);
AppMsgQueueImpl* s_globalAppMsgQueueImpl{};
std::atomic<uint64_t> s_nextIndex;
} // namespace

struct Msg {
    uint64_t index;
    std::any payload;
};

struct AppMsgQueueImpl : public AppMsgQueue {
    std::thread::id mainThreadId;
    AppReceiverFn appReceiverFn;

    std::deque<Msg> mainThreadQueue; // Async messages to App sent from main thread.

    // Async message to App sent from non-main thread.
    moodycamel::ConcurrentQueue<Msg> concurrentQueue;

    // Async messages to App sent from other thread, already dequeued from concurrentQueue.
    std::deque<Msg> dequedConcurrentQueueMsgs;

    explicit AppMsgQueueImpl(AppReceiverFn appReceiverFnArg)
        : mainThreadId(this_thread::get_id())
        , appReceiverFn(MOVE(appReceiverFnArg))
    {
    }
    ~AppMsgQueueImpl() override
    {
        CHECK(this_thread::get_id() == mainThreadId);
        CHECK(s_globalAppMsgQueueImpl != this);
    }

    void makeThisGlobalAppQueue(bool b) override
    {
        CHECK(this_thread::get_id() == mainThreadId);
        if (b) {
            CHECK(!s_globalAppMsgQueueImpl);
            s_globalAppMsgQueueImpl = this;
        } else {
            CHECK(s_globalAppMsgQueueImpl == this);
            s_globalAppMsgQueueImpl = nullptr;
        }
    }

    void enqueue(std::any&& payload) override
    {
        auto msg = Msg{.index = s_nextIndex++, .payload = MOVE(payload)};
        if (this_thread::get_id() == mainThreadId) {
            mainThreadQueue.push_back(MOVE(msg));
        } else {
            concurrentQueue.enqueue(MOVE(msg));
        }

        SDL_Event e;
        SDL_zero(e); /* SDL will copy this entire struct! Initialize to keep memory checkers happy. */
        e.type = s_appQueueNotificationSdlEventType;
        LOG_IF(FATAL, !SDL_PushEvent(&e)) << fmt::format("SDL_PushEvent failed: {}", SDL_GetError());
    }

    optional<std::any> dequeue() override
    {
        CHECK(this_thread::get_id() == mainThreadId);
        if (dequedConcurrentQueueMsgs.empty()) {
            Msg msg;
            if (concurrentQueue.try_dequeue(msg)) {
                dequedConcurrentQueueMsgs.push_back(MOVE(msg));
            }
        }
        if (!mainThreadQueue.empty()
            && (dequedConcurrentQueueMsgs.empty() || mainThreadQueue.front().index <= dequedConcurrentQueueMsgs.front().index)) {
            auto msg = MOVE(mainThreadQueue.front());
            mainThreadQueue.pop_front();
            return optional(MOVE(msg.payload));
        }
        if (!dequedConcurrentQueueMsgs.empty()) {
            auto msg = MOVE(dequedConcurrentQueueMsgs.front());
            dequedConcurrentQueueMsgs.pop_front();
            return optional(MOVE(msg.payload));
        }
        return nullopt;
    }
};

unique_ptr<AppMsgQueue> AppMsgQueue::make(AppReceiverFn appReceiverFn)
{
    return make_unique<AppMsgQueueImpl>(MOVE(appReceiverFn));
}

bool isThisTheMainThread()
{
    CHECK(s_globalAppMsgQueueImpl);
    return this_thread::get_id() == s_globalAppMsgQueueImpl->mainThreadId;
}

void sendToApp(std::any&& payload)
{
    s_globalAppMsgQueueImpl->enqueue(MOVE(payload));
}

void sendToAppSync(std::any&& payload)
{
    CHECK(this_thread::get_id() == s_globalAppMsgQueueImpl->mainThreadId);
    s_globalAppMsgQueueImpl->appReceiverFn(MOVE(payload));
}

bool tryDequeueAndMakeAppReceiveIt()
{
    if (auto msg = s_globalAppMsgQueueImpl->dequeue()) {
        // `dequeue` already checks that we're on the main thread.
        s_globalAppMsgQueueImpl->appReceiverFn(MOVE(*msg));
        return true;
    }
    return false;
}

uint32_t appQueueNotificationSdlEventType()
{
    return s_appQueueNotificationSdlEventType;
}
