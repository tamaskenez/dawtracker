#pragma once

#include "common/std.h"

struct AudioEngineState {
    struct Metronome {
        bool on = false;
        float bpm;
    } metronome;
};

class AudioEngine
{
public:
    using StateChangerFn = function<void(AudioEngineState&)>;

    static unique_ptr<AudioEngine> make();
    virtual ~AudioEngine() = default;

    // Called on main thread anytime.
    virtual void sendStateChangerFn(StateChangerFn fn) = 0;

    virtual void record() = 0;
    virtual void stopRecording() = 0;

    virtual void audioCallbacksAboutToStart(double sampleRate, size_t bufferSize, size_t numInputChannels) = 0;
    virtual void audioCallbacksStopped() = 0;

    // Called on the audio callback thread.
    virtual void process(span<const float*> inputChannels, span<float*> outputChannels, size_t numSamples) = 0;
};
