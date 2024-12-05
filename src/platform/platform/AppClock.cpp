#include "AppClock.h"

#include "SDL3/SDL_timer.h"

AppClock::time_point AppClock::now()
{
    auto ticks = SDL_GetTicksNS();
    assert(std::in_range<chr::nanoseconds::rep>(ticks));
    return AppClock::time_point(chr::nanoseconds(ticks));
}
