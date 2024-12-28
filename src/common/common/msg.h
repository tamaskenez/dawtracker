#pragma once

#include "RecordingBuffer.h"
#include "common/Id.h"

struct AudioClip;

namespace msg
{
enum class MainMenu {
    quit,
    settings,
    hideSettings
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
    size_t numInputChannels;
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
struct OutputChanged {
    string name;
    bool enabled;
};
struct PlayClip {
    Id<AudioClip> id;
};
namespace AudioEngine
{
struct NoFreeRecordingBuffer {
};
struct RecordingBufferRecorded {
    RecordingBuffer* recordingBuffer;
    chr::high_resolution_clock::time_point timestamp;
};
struct PlayedTime {
    optional<double> t;
};
using V = variant<NoFreeRecordingBuffer, RecordingBufferRecorded, PlayedTime>;
} // namespace AudioEngine
} // namespace msg
