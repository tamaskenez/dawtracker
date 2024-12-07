#pragma once

#include "common/audiodevicetypes.h"

// Things we should keep across application suspends, launches but not part of a document.
struct AppState {
    AudioSettings audioSettings;
};
