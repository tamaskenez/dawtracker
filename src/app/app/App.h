#pragma once

#include "common/std.h"

struct Msg;

class UI;

class App
{
public:
    static unique_ptr<App> make(UI* ui);
    virtual ~App() = default;

    virtual void receive(Msg&& msg) = 0;
};
