#pragma once

#include "common/std.h"

struct UIState;

class UI
{
public:
    static unique_ptr<UI> make();
    virtual ~UI() = default;
    virtual void render() = 0;
    virtual UIState* getUIState() = 0;
};
