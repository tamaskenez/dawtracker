#include "AppCtx.h"

#include "audio/AudioIO.h"
#include "ui/UI.h"

AppCtx::AppCtx(UI* uiArg)
    : ui(uiArg)
    , uiState(ui->getUIState())
    , audioEngine(AudioEngine::make())
    , audioIO(AudioIO::make())
{
}

AppCtx::~AppCtx() = default;
