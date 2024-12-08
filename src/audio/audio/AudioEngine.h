#pragma once

#include "common/std.h"

struct AudioEngineState {
    struct Metronome {
        bool on = false;
        double bpm;
    } metronome;
};

class AudioEngine
{
public:
    using StateChangerFn = function<void(AudioEngineState&)>;

    static unique_ptr<AudioEngine> make();
    virtual ~AudioEngine() = default;

    // Called on main thread anytime.
    virtual void sendStateChangerFn(StateChangerFn fn, bool audioCallbackRunning) = 0;

    // Called probably on the audio callback thread but I'm not sure.
    virtual void audioCallbacksAboutToStart(double sampleRate, size_t bufferSize) = 0;
    virtual void audioCallbacksStopped() = 0;

    // Called on the audio callback thread.
    virtual void process(span<const float*> inputChannels, span<float*> outputChannels, size_t numSamples) = 0;
};
