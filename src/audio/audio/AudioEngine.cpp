#include "AudioEngine.h"

#include "common/MetronomeGenerator.h"
#include "common/common.h"
#include "common/msg.h"
#include "platform/AppMsgQueue.h"

#include "readerwriterqueue/readerwriterqueue.h"

namespace
{
constexpr size_t k_numRecordingBuffers = 7;

void elementWiseOperatorPlusEquals(span<float> x, span<float> y)
{
    CHECK(x.size() == y.size());
    for (size_t i : vi::iota(0u, x.size())) {
        x[i] += y[i];
    }
}
} // namespace

struct AudioEngineImpl : public AudioEngine {
    moodycamel::ReaderWriterQueue<StateChangerFn> mainToCallbackThreadQueue;

    // Following variables will accessed on the audio callback thread.
    std::atomic_bool audioCallbacksRunning;
    double sampleRate = 0;
    size_t bufferSize = 0;
    AudioEngineState state;
    MetronomeGenerator metronome;
    vector<float> metronomeBuffer;
    array<RecordingBuffer, k_numRecordingBuffers> recordingBuffers;
    std::atomic_bool recording;
    void audioCallbacksAboutToStart(double sampleRateArg, size_t bufferSizeArg, size_t numInputChannels) override
    {
        LOG(INFO) << fmt::format(
          "audioCallbacksAboutToStart thread: {}, {}Hz/{}, ins: {}",
          this_thread::get_id(),
          sampleRateArg,
          bufferSizeArg,
          numInputChannels
        );
        sampleRate = sampleRateArg;
        bufferSize = bufferSizeArg;
        metronome.timeSinceLastStart = 0;
        metronomeBuffer.resize(bufferSize);
        audioCallbacksRunning = true;
        for (auto& rb : recordingBuffers) {
            rb.initialize(numInputChannels, bufferSize);
        }
    }

    void audioCallbacksStopped() override
    {
        audioCallbacksRunning = false;
        LOG(INFO) << fmt::format("audioCallbacksStopped thread: {}", this_thread::get_id());
        processMainToCallbackThreadQueue();
    }

    // Called on the audio callback thread.
    void process(span<const float*> inputChannels, span<float*> outputChannels, size_t numSamples) override
    {
        for (auto oc : outputChannels) {
            std::fill(oc, oc + numSamples, 0.0f);
        }
        if (!audioCallbacksRunning) {
            LOG(WARNING) << "Called while not running.";
            return;
        }
        processMainToCallbackThreadQueue();

        if (state.metronome.on) {
            assert(metronomeBuffer.size() == numSamples);
            metronome.generate(sampleRate, boost::rational_cast<double>(state.metronome.bpm), metronomeBuffer);
            for (auto oc : outputChannels) {
                elementWiseOperatorPlusEquals(span<float>(oc, numSamples), metronomeBuffer);
            }
        }
        if (state.clipToPlay) {
            auto& clip = *state.clipToPlay;
            if (state.nextSampleToPlay < clip.size()) {
                auto endSampleIx = std::min(numSamples, clip.size() - state.nextSampleToPlay);
                for (size_t chix : vi::iota(0u, std::min(outputChannels.size(), clip.channels.size()))) {
                    auto& sourceChannel = state.clipToPlay->channels[chix];
                    auto& outputChannel = outputChannels[chix];
                    for (size_t i = 0; i < endSampleIx; ++i) {
                        outputChannel[i] += sourceChannel[state.nextSampleToPlay + i];
                    }
                }
                state.nextSampleToPlay += endSampleIx;
            }
            if (clip.size() <= state.nextSampleToPlay) {
                state.clipToPlay.reset();
                state.nextSampleToPlay = 0;
                sendToApp(MAKE_VARIANT_V(msg::AudioEngine, PlayedTime{}));
            } else {
                sendToApp(MAKE_VARIANT_V(msg::AudioEngine, PlayedTime{state.nextSampleToPlay / sampleRate}));
            }
        }
        if (recording) {
            RecordingBuffer* recordingBuffer{};
            for (auto& rb : recordingBuffers) {
                if (!rb.sentToApp) {
                    recordingBuffer = &rb;
                    break;
                }
            }
            std::string c;
            for (auto& rb : recordingBuffers) {
                c += rb.sentToApp ? 'S' : '.';
            }
            if (!recordingBuffer) {
                sendToApp(MAKE_VARIANT_V(msg::AudioEngine, NoFreeRecordingBuffer{}));
            } else {
                CHECK(inputChannels.size() == recordingBuffer->channels.size());
                for (size_t i : vi::iota(0u, inputChannels.size())) {
                    CHECK(numSamples == recordingBuffer->channels[i].size());
                    std::copy_n(inputChannels[i], bufferSize, recordingBuffer->channels[i].begin());
                }
                recordingBuffer->sentToApp = true;
                LOG(INFO) << fmt::format("[{}]->sentToApp = true {}", fmt::ptr(recordingBuffer), c);
                sendToApp(msg::AudioEngine::V(
                  msg::AudioEngine::RecordingBufferRecorded{recordingBuffer, chr::high_resolution_clock::now()}
                ));
            }
        }
    }

    void processMainToCallbackThreadQueue()
    {
        StateChangerFn item;
        while (mainToCallbackThreadQueue.try_dequeue(item)) {
            item(state);
        }
    }

    void sendStateChangerFn(StateChangerFn fn) override
    {
        mainToCallbackThreadQueue.enqueue(MOVE(fn));
        if (!audioCallbacksRunning) {
            processMainToCallbackThreadQueue();
        }
    }

    void record() override
    {
        recording = true;
    }

    void stopRecording() override
    {
        recording = false;
    }

    void play(AudioClip&& clipArg) override
    {
        mainToCallbackThreadQueue.enqueue([clip = MOVE(clipArg)](AudioEngineState& s) mutable {
            s.clipToPlay = MOVE(clip);
            s.nextSampleToPlay = 0;
        });
    }
    void stopPlaying() override
    {
        mainToCallbackThreadQueue.enqueue([](AudioEngineState& s) mutable {
            s.clipToPlay.reset();
            s.nextSampleToPlay = 0;
        });
    }
};

unique_ptr<AudioEngine> AudioEngine::make()
{
    return make_unique<AudioEngineImpl>();
}
