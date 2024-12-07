#pragma once

#include "common/std.h"

// A dialog in the app-logic side of the UI
struct Dialog {
    virtual bool receive(std::any&& msg) = 0;
    virtual ~Dialog() = default;
};
