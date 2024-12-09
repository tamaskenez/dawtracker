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
        LOG(INFO) << fmt::format("audioCallbacksAboutToStart thread: {}", this_thread::get_id());
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
    void process(UNUSED span<const float*> inputChannels, UNUSED span<float*> outputChannels, UNUSED size_t numSamples)
      override
    {
        for (auto oc : outputChannels) {
            std::fill(oc, oc + numSamples, 0);
        }
        if (!audioCallbacksRunning) {
            assert(false);
            return;
        }
        processMainToCallbackThreadQueue();

        if (state.metronome.on) {
            assert(metronomeBuffer.size() == numSamples);
            metronome.generate(sampleRate, state.metronome.bpm, metronomeBuffer);
            for (auto oc : outputChannels) {
                elementWiseOperatorPlusEquals(span<float>(oc, numSamples), metronomeBuffer);
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
            if (!recordingBuffer) {
                sendToApp(MAKE_VARIANT_V(msg::AudioEngine, NoFreeRecordingBuffer{}));
            } else {
                CHECK(inputChannels.size() == recordingBuffer->channels.size());
                for (size_t i : vi::iota(0u, inputChannels.size())) {
                    CHECK(numSamples == recordingBuffer->channels[i].size());
                    std::copy_n(inputChannels[i], bufferSize, recordingBuffer->channels[i].begin());
                }
                recordingBuffer->sentToApp = true;
                sendToApp(MAKE_VARIANT_V(msg::AudioEngine, RecordingBufferRecorded{recordingBuffer}));
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
};

unique_ptr<AudioEngine> AudioEngine::make()
{
    return make_unique<AudioEngineImpl>();
}
