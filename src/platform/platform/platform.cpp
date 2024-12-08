#include "platform.h"

#include "common/common.h"

#include "SDL3/SDL_events.h"

namespace
{
const uint32_t s_refreshUISdlEventType = SDL_RegisterEvents(1);
} // namespace

uint32_t refreshUISdlEventType()
{
    return s_refreshUISdlEventType;
}

void sendQuitEventToAppMain()
{
    SDL_Event e;
    SDL_zero(e); /* SDL will copy this entire struct! Initialize to keep memory checkers happy. */
    e.type = SDL_EVENT_QUIT;
    LOG_IF(FATAL, !SDL_PushEvent(&e)) << fmt::format("SDL_PushEvent failed: {}", SDL_GetError());
}

void sendRefreshUIEventToAppMain()
{
    SDL_Event e;
    SDL_zero(e); /* SDL will copy this entire struct! Initialize to keep memory checkers happy. */
    e.type = s_refreshUISdlEventType;
    LOG_IF(FATAL, !SDL_PushEvent(&e)) << fmt::format("SDL_PushEvent failed: {}", SDL_GetError());
}
