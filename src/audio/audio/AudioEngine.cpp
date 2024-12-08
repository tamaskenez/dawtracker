#include "AudioEngine.h"

#include "common/MetronomeGenerator.h"
#include "common/common.h"

#include "readerwriterqueue/readerwriterqueue.h"

struct ScopedRaceConditionDetector {
    std::atomic_bool& thatObjectIsInUse;
    explicit ScopedRaceConditionDetector(std::atomic_bool& thatObjectIsInUseArg)
        : thatObjectIsInUse(thatObjectIsInUseArg)
    {
        CHECK(!thatObjectIsInUse.exchange(true));
    }
    ~ScopedRaceConditionDetector()
    {
        thatObjectIsInUse.store(false);
    }
};

struct AudioEngineImpl : public AudioEngine {
    std::atomic_bool thisObjectIsInUse;
    AudioEngineState state;
    moodycamel::ReaderWriterQueue<StateChangerFn> mainToCallbackThreadQueue;

    MetronomeGenerator metronome;
    double sampleRate = 0;
    size_t bufferSize = 0;

    void audioCallbacksAboutToStart(double sampleRateArg, size_t bufferSizeArg) override
    {
        fmt::println("audioCallbacksAboutToStart thread: {}", this_thread::get_id());
        sampleRate = sampleRateArg;
        bufferSize = bufferSizeArg;
    }

    void audioCallbacksStopped() override
    {
        fmt::println("audioCallbacksStopped thread: {}", this_thread::get_id());
        processMainToCallbackThreadQueue();
    }

    // Called on the audio callback thread.
    void process(UNUSED span<const float*> inputChannels, UNUSED span<float*> outputChannels, UNUSED size_t numSamples)
      override
    {
        static bool printed = false;
        if (!printed) {
            fmt::println("process thread: {}", this_thread::get_id());
            printed = true;
        }
        ScopedRaceConditionDetector rcd(thisObjectIsInUse);
    }

    void processMainToCallbackThreadQueue()
    {
        StateChangerFn item;
        while (mainToCallbackThreadQueue.try_dequeue(item)) {
            item(state);
        }
    }

    void sendStateChangerFn(StateChangerFn fn, bool audioCallbackRunning) override
    {
        mainToCallbackThreadQueue.enqueue(MOVE(fn));
        if (!audioCallbackRunning) {
            processMainToCallbackThreadQueue();
        }
    }
};

unique_ptr<AudioEngine> AudioEngine::make()
{
    return make_unique<AudioEngineImpl>();
}
