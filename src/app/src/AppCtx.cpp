#include "AppCtx.h"

#include "audio/AudioEngine.h"

AppCtx::AppCtx(UI* uiArg)
    : ui(uiArg)
    , audioEngine(AudioEngine::make())
{
}

AppCtx::~AppCtx() = default;
