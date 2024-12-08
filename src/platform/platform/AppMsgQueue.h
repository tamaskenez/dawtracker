#pragma once

#include "common/std.h"

using AppReceiverFn = function<void(std::any&&)>;

// Class providing synchronous (from main thread) and asynchronous (from any thread) message sending to the app, through
// a lambda.
//
// An object of the class can be made global so the instance set up to talk to the App can be accessed without
// passing it to every distant component.
//
// Whenever an asynchronous message is sent to the app, an SDL user event of type `appQueueNotificationSdlEventType` is
// sent to the SDL event loop to make it deque the message and call the app's receive function.
class AppMsgQueue
{
public:
    // The thread on which this is called will be recognized as the main thread.
    static unique_ptr<AppMsgQueue> make(AppReceiverFn appReceiverFn);

    // Must not be global when destructed. Must be called on the main thread.
    virtual ~AppMsgQueue() = default;

    // Must be on the thread it was constructed.
    // Call with `false` to make this non-global again.
    virtual void makeThisGlobalAppQueue(bool b) = 0;

    // Can be called from any thread.
    virtual void enqueue(std::any&& payload) = 0;

    // Must be called from main thread.
    virtual optional<std::any> dequeue() = 0;
};

uint32_t appQueueNotificationSdlEventType();

// The following functions use the global AppMsgQueue.
bool isThisTheMainThread();
void sendToApp(std::any&& payload);
void sendToAppSync(std::any&& payload);
bool tryDequeueAndMakeAppReceiveIt();
