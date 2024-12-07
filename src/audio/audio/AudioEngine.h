#include "common/std.h"

#include "common/audiodevicetypes.h"

using AudioCallbackFn =
  function<void(span<const float*> inputChannels, span<float*> outputChannels, size_t numSamples)>;

class AudioEngine
{
public:
    static unique_ptr<AudioEngine> make();

    virtual ~AudioEngine() = default;

    virtual vector<AudioDevice> getAudioDevices() = 0;
    virtual expected<AudioSettings, string>
    initialize(optional<string> outputDeviceName, optional<string> inputDeviceName) = 0;
    virtual AudioSettings getAudioSettings() = 0;
    virtual void setAudioCallback(AudioCallbackFn fn) = 0;

    virtual void runDispatchLoopUntil(chr::milliseconds d) = 0;
};
