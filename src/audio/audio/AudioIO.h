#include "common/std.h"

#include "common/audiodevicetypes.h"

using AudioCallbacksAboutToStartFn = function<void(double sampleRate, size_t bufferSize)>;
using AudioCallbacksStoppedFn = function<void()>;
using AudioCallbackFn =
  function<void(span<const float*> inputChannels, span<float*> outputChannels, size_t numSamples)>;

class AudioIO
{
public:
    static unique_ptr<AudioIO> make();

    virtual ~AudioIO() = default;

    virtual vector<AudioDevice> getAudioDevices() = 0;
    virtual expected<AudioSettings, string>
    initialize(optional<string> outputDeviceName, optional<string> inputDeviceName) = 0;
    virtual AudioSettings getAudioSettings() = 0;
    virtual void setCallbacks(
      AudioCallbacksAboutToStartFn startFn, AudioCallbackFn callbackFn, AudioCallbacksStoppedFn stoppedFn
    ) = 0;

    virtual void runDispatchLoopUntil(chr::milliseconds d) = 0;
};
