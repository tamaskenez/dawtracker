#pragma once

#include "common/std.h"

class UI;

class App
{
public:
    static unique_ptr<App> make(UI* ui);
    virtual ~App() = default;

    virtual void receive(std::any&& msg) = 0;
    virtual void runAudioIODispatchLoop() = 0;
};
