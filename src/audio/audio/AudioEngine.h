#include "common/std.h"

#include "common/audiodevicetypes.h"

class AudioEngine
{
public:
    static unique_ptr<AudioEngine> make();

    virtual ~AudioEngine() = default;

    virtual vector<AudioDevice> getAudioDevices() = 0;
    virtual expected<AudioSettings, string>
    initialize(optional<string> outputDeviceName, optional<string> inputDeviceName) = 0;
    virtual void runDispatchLoopUntil(chr::milliseconds d) = 0;
};
