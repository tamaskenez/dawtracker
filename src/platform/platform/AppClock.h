#pragma once

#include "common/std.h"

struct AppClock {
    using duration = chr::nanoseconds;
    using rep = duration::rep;
    using period = duration::period;
    using time_point = chr::time_point<AppClock>;

    static time_point now();
};

using AppTimestamp = AppClock::time_point;
