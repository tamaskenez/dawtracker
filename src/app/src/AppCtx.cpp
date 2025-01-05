#include "AppCtx.h"

#include "common/AppState.h"

#include "audio/AudioIO.h"
#include "ui/UI.h"

AppCtx::AppCtx(UI* uiArg, AppState& appStateArg)
    : ui(uiArg)
    , appState(appStateArg)
    , rse(appState.rse)
    , audioEngine(AudioEngine::make())
    , audioIO(AudioIO::make())
{
}

AppCtx::~AppCtx() = default;
