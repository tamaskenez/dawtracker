#pragma once

#include "AppState.h"
#include "UIState.h"
#include "common/audiodevicetypes.h"

class UI;

// Core app state passed to dialogs.
class AudioIO;
struct AppCtx {
    UI* ui; // To control the UI.

    AppState appState; // Essential state of the app.
    UIState uiState;   // To provide data to the UI

    // Volatile state of the app that can be reconstructed from the essential.
    unique_ptr<AudioIO> audioIO;

    explicit AppCtx(UI* uiArg);
    ~AppCtx();
};
