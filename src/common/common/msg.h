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

namespace AudioIO
{
struct Changed {
};
struct AudioCallbacksAboutToStart {
    double sampleRate;
    size_t bufferSize;
};
struct AudioCallbacksStopped {
};
using V = variant<Changed, AudioCallbacksAboutToStart, AudioCallbacksStopped>;
} // namespace AudioIO

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

enum class Transport {
    record,
    stop,
    play
};

struct InputChanged {
    string name;
    bool enabled;
};
} // namespace msg
