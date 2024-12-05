#include "MsgQueue.h"

#include "common/common.h"

#include "concurrentqueue.h"

namespace
{
MsgQueue* s_globalAppQueue{};
}

struct MsgQueueImpl : public MsgQueue {
    std::thread::id constructorThreadId;
    std::deque<Msg> constructorThreadQueue;
    std::deque<Msg> dequedConcurrentQueueMsgs;
    moodycamel::ConcurrentQueue<Msg> concurrentQueue;

    MsgQueueImpl()
        : constructorThreadId(this_thread::get_id())
    {
    }
    ~MsgQueueImpl() override
    {
        CHECK(this_thread::get_id() == constructorThreadId);
        CHECK(s_globalAppQueue != this);
    }

    void makeThisGlobalAppQueue(bool b) override
    {
        CHECK(this_thread::get_id() == constructorThreadId);
        if (b) {
            CHECK(!s_globalAppQueue);
            s_globalAppQueue = this;
        } else {
            CHECK(s_globalAppQueue == this);
            s_globalAppQueue = nullptr;
        }
    }

    void enqueue(std::any&& payload) override
    {
        auto msg = Msg{.timestamp = AppClock::now(), .payload = MOVE(payload)};
        if (this_thread::get_id() == constructorThreadId) {
            constructorThreadQueue.push_back(MOVE(msg));
        } else {
            concurrentQueue.enqueue(MOVE(msg));
        }
    }

    optional<Msg> dequeue() override
    {
        CHECK(this_thread::get_id() == constructorThreadId);
        if (dequedConcurrentQueueMsgs.empty()) {
            Msg msg;
            if (concurrentQueue.try_dequeue(msg)) {
                dequedConcurrentQueueMsgs.push_back(MOVE(msg));
            }
        }
        if(!constructorThreadQueue.empty()&&(dequedConcurrentQueueMsgs.empty()||constructorThreadQueue.front().timestamp<= dequedConcurrentQueueMsgs.front().timestamp)){
            auto msg = MOVE(constructorThreadQueue.front());
            constructorThreadQueue.pop_front();
            return optional(MOVE(msg));
        }
        if (!dequedConcurrentQueueMsgs.empty()) {
            auto msg = MOVE(dequedConcurrentQueueMsgs.front());
            dequedConcurrentQueueMsgs.pop_front();
            return optional(MOVE(msg));
        }
        return nullopt;
    }
};

unique_ptr<MsgQueue> MsgQueue::make()
{
    return make_unique<MsgQueueImpl>();
}

MsgQueue* MsgQueue::globalAppQueue()
{
    CHECK(s_globalAppQueue);
    return s_globalAppQueue;
}
