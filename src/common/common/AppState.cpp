#include "AppState.h"

#include "common/common.h"

namespace
{
static const string k_question_mark = "?";
}

const string& AppState::AudioSettingsUI::selectedOutputDeviceName() const
{
    CHECK_OR_RETURN_VAL(selectedOutputDeviceIx < outputDeviceNames.size(), k_question_mark);
    return outputDeviceNames[selectedOutputDeviceIx];
}
const string& AppState::AudioSettingsUI::selectedInputDeviceName() const
{
    CHECK_OR_RETURN_VAL(selectedInputDeviceIx < inputDeviceNames.size(), k_question_mark);
    return inputDeviceNames[selectedInputDeviceIx];
}
