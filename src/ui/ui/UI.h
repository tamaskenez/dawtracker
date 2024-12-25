#pragma once

#include "common/std.h"

struct AppState;
class ReactiveStateEngine;

class UI
{
public:
    static unique_ptr<UI> make(const AppState& appState, ReactiveStateEngine& rse);
    virtual ~UI() = default;
    virtual void render() = 0;
    virtual void addFrameTime(chr::high_resolution_clock::time_point t, chr::high_resolution_clock::duration dt) = 0;
};
