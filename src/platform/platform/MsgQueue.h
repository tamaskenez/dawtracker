#pragma once

#include "Msg.h"

#include "common/std.h"

// Support multiple writers and single reader who must be on the thread where the queue was created.
class MsgQueue
{
public:
    static unique_ptr<MsgQueue> make();
    // Return non-null pointer.
    // There must be a global app queue at this point.
    static MsgQueue* globalAppQueue();

    // Must not be global when destructed. Must be on the thread it was constructed.
    virtual ~MsgQueue() = default;

    // Must be on the thread it was constructed.
    virtual void makeThisGlobalAppQueue(bool b) = 0;

    virtual void enqueue(std::any&& payload) = 0;
    virtual optional<Msg> dequeue() = 0;
};
