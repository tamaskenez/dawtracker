#include "AppCtx.h"

#include "audio/AudioIO.h"

AppCtx::AppCtx(UI* uiArg)
    : ui(uiArg)
    , audioIO(AudioIO::make())
{
}

AppCtx::~AppCtx() = default;
