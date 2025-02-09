#pragma once

#include "common/std.h"

class UI;
struct AppState;

class App
{
public:
    static unique_ptr<App> make(UI* ui, AppState& appState);
    virtual ~App() = default;

    virtual void receive(std::any&& msg) = 0;
    virtual void runAudioIODispatchLoop() = 0;
    virtual bool getAndClearIfUIRefreshNeeded() = 0;
};
