#pragma once

#include "common/std.h"
#include "common/uistate.h"

class UI
{
public:
    static unique_ptr<UI> make();
    virtual ~UI() = default;
    virtual void render() = 0;
    virtual void openSettings(const uistate::AudioSettings* audioSettingsState) = 0;
    virtual void setMetronome(const uistate::Metronome* metronome) = 0;
    virtual void closeDialogs() = 0;
};
