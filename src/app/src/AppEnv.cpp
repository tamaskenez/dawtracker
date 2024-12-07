#include "AppEnv.h"

#include "audio/AudioEngine.h"

AppEnv::AppEnv(UI* uiArg)
    : ui(uiArg)
    , audioEngine(AudioEngine::make())
{
}

AppEnv::~AppEnv() = default;
