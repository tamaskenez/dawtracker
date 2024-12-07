#pragma once

#include "std.h"

namespace uistate
{
struct AudioSettings {
    vector<string> outputDeviceNames, inputDeviceNames;
    size_t selectedOutputDeviceIx = 0, selectedInputDeviceIx = 0;

    const string& selectedOutputDeviceName() const;
    const string& selectedInputDeviceName() const;
};
struct Metronome {
    bool on = false;
    float bpm = 120.0;
};
} // namespace uistate
