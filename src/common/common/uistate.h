#pragma once

#include "std.h"

namespace uistate
{
struct AudioSettings {
    vector<string> outputDeviceNames, inputDeviceNames;
    size_t selectedOutputDeviceIx, selectedInputDeviceIx;

    const string& selectedOutputDeviceName() const;
    const string& selectedInputDeviceName() const;
};
} // namespace uistate
