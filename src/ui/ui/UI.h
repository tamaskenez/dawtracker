#pragma once

#include "common/std.h"

struct AppState;

class UI
{
public:
    static unique_ptr<UI> make(const AppState& appState);
    virtual ~UI() = default;
    virtual void render() = 0;
    virtual void addFrameTime(chr::high_resolution_clock::time_point t, chr::high_resolution_clock::duration dt) = 0;
};
