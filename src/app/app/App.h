#pragma once

#include "common/std.h"

class UI;

class App
{
public:
    static unique_ptr<App> make(UI* ui);
    virtual ~App() = default;
};
