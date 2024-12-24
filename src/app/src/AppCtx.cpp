#include "AppCtx.h"

#include "audio/AudioIO.h"
#include "ui/UI.h"

AppCtx::AppCtx(UI* uiArg, AppState& appStateArg, ReactiveStateEngine& rseArg)
    : ui(uiArg)
    , appState(appStateArg)
    , rse(rseArg)
    , audioEngine(AudioEngine::make())
    , audioIO(AudioIO::make())
{
}

AppCtx::~AppCtx() = default;
