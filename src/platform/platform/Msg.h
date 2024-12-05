#pragma once

#include "AppClock.h"

struct Msg {
    AppTimestamp timestamp;
    std::any payload;
};
