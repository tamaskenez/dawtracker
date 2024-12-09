#include "AudioClip.h"

#include "RecordingBuffer.h"
#include "common.h"

AudioClip::AudioClip(double sampleRateArg, size_t numChannels)
    : sampleRate(sampleRateArg)
    , channels(numChannels)
{
}

void AudioClip::append(const RecordingBuffer& from)
{
    CHECK(channels.size() == from.channels.size());
    for (size_t i : vi::iota(0u, channels.size())) {
        auto& toChannel = channels[i];
        auto& fromChannel = from.channels[i];
        toChannel.insert(toChannel.end(), fromChannel.begin(), fromChannel.end());
    }
}
