#pragma once

#include "AppUI.h"

#include "common/audiodevicetypes.h"
#include "common/uistate.h"

class UI;

struct UIState {
    uistate::AudioSettings audioSettings;
};

// Things we should keep across application suspends, launches but not part of a document.
struct AppState {
    AudioSettings audioSettings;
};

// Core app state passed to dialogs.
class AudioEngine;
struct AppEnv {
    UI* ui;
    UIState uiState;
    AppUI appUI;
    AppState appState;
    unique_ptr<AudioEngine> audioEngine;
    explicit AppEnv(UI* uiArg);
    ~AppEnv();
};
