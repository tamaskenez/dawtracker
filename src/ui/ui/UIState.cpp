#include "UIState.h"

#include "common/common.h"

namespace
{
static const string k_question_mark = "?";
}

const string& UIState::AudioSettings::selectedOutputDeviceName() const
{
    CHECK_OR_RETURN_VAL(selectedOutputDeviceIx < outputDeviceNames.size(), k_question_mark);
    return outputDeviceNames[selectedOutputDeviceIx];
}
const string& UIState::AudioSettings::selectedInputDeviceName() const
{
    CHECK_OR_RETURN_VAL(selectedInputDeviceIx < inputDeviceNames.size(), k_question_mark);
    return inputDeviceNames[selectedInputDeviceIx];
}
