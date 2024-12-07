#pragma once

#include "Dialog.h"

#include "common/std.h"

// The app-logic side of the things happening on the UI.
struct AppUI {
    vector<unique_ptr<Dialog>> dialogs;
    bool receive(std::any&& msg);
};
