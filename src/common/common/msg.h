#pragma once

namespace msg
{
enum class MainMenu {
    quit,
    settings
};

namespace AudioSettings
{
struct OutputDeviceSelected {
    size_t i;
};
struct InputDeviceSelected {
    size_t i;
};
using V = variant<OutputDeviceSelected, InputDeviceSelected>;
} // namespace AudioSettings

namespace AudioEngine
{
struct Changed {
};
using V = variant<Changed>;
} // namespace AudioEngine

namespace Metronome
{
struct On {
    bool b;
};
struct BPM {
    float bpm;
};
using V = variant<On, BPM>;
} // namespace Metronome
} // namespace msg
