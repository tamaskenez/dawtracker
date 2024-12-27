#pragma once

#include "common/common.h"

#include "common/AudioClip.h"

struct AudioEngineState {
    struct Metronome {
        bool on = false;
        Rational bpm;
    } metronome;

    // todo these should be one data.
    optional<AudioClip> clipToPlay;
    size_t nextSampleToPlay = 0;
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

    virtual void play(AudioClip&& clip) = 0;
    virtual void stopPlaying() = 0;

    virtual void audioCallbacksAboutToStart(double sampleRate, size_t bufferSize, size_t numInputChannels) = 0;
    virtual void audioCallbacksStopped() = 0;

    // Called on the audio callback thread.
    virtual void process(span<const float*> inputChannels, span<float*> outputChannels, size_t numSamples) = 0;
};
