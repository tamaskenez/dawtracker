#pragma once

#include "common/AudioClip.h"
#include "common/audiodevicetypes.h"

#include "audio/AudioEngine.h"

class UI;
struct AppState;
class ReactiveStateEngine;

// Core app state passed to dialogs.
class AudioIO;
struct AppCtx {
    UI* ui;

    AppState& appState;
    ReactiveStateEngine& rse;

    unique_ptr<AudioEngine> audioEngine;
    unique_ptr<AudioIO> audioIO;

    AppCtx(UI* uiArg, AppState& appState, ReactiveStateEngine& rse);
    ~AppCtx();
};
