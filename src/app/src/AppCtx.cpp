#include "AppCtx.h"

#include "audio/AudioIO.h"

AppCtx::AppCtx(UI* uiArg)
    : ui(uiArg)
    , audioEngine(AudioEngine::make())
    , audioIO(AudioIO::make())
{
}

AppCtx::~AppCtx() = default;
