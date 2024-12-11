#pragma once

#include "AppState.h"

#include "common/AudioClip.h"
#include "common/audiodevicetypes.h"
#include "ui/UIState.h"

#include "audio/AudioEngine.h"

class UI;

// Core app state passed to dialogs.
class AudioIO;
struct AppCtx {
    UI* ui;           // To control the UI.
    UIState* uiState; // To provide data to the UI

    AppState appState; // Essential state of the app.

    unique_ptr<AudioEngine> audioEngine;
    // Volatile state of the app that can be reconstructed from essential or doesn't need to be reconstructed.
    unique_ptr<AudioIO> audioIO;
    optional<AudioClip> clipBeingRecorded;
    bool clipBeingPlayed = false;
    vector<AudioClip> clips;

    explicit AppCtx(UI* uiArg);
    ~AppCtx();
};
