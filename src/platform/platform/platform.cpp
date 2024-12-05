#include "platform.h"

#include "MsgQueue.h"

#include "common/common.h"

#include "SDL3/SDL_events.h"

namespace
{
const uint32_t s_appQueueNotificationSdlEventType = SDL_RegisterEvents(1);
}

void sendToApp(std::any&& payload)
{
    MsgQueue::globalAppQueue()->enqueue(MOVE(payload));

    SDL_Event e;
    SDL_zero(e); /* SDL will copy this entire struct! Initialize to keep memory checkers happy. */
    e.type = s_appQueueNotificationSdlEventType;
    LOG_IF(FATAL, !SDL_PushEvent(&e)) << fmt::format("SDL_PushEvent failed: {}", SDL_GetError());
}

uint32_t appQueueNotificationSdlEventType()
{
    return s_appQueueNotificationSdlEventType;
}

void sendQuitEventToAppMain()
{
    SDL_Event e;
    SDL_zero(e); /* SDL will copy this entire struct! Initialize to keep memory checkers happy. */
    e.type = SDL_EVENT_QUIT;
    LOG_IF(FATAL, !SDL_PushEvent(&e)) << fmt::format("SDL_PushEvent failed: {}", SDL_GetError());
}
