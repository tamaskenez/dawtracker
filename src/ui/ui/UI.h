#pragma once

#include "common/std.h"

class UI
{
public:
    static unique_ptr<UI> make();
    virtual ~UI() = default;
};
