#pragma once

#include "std.h"

struct RecordingBuffer;

// Todo make it safer, guarantee invariants.
struct AudioClip {
    AudioClip(double sampleRate, size_t numChannels);
    double sampleRate;
    vector<deque<float>> channels;

    size_t size() const
    {
        return channels.empty() ? 0 : channels[0].size();
    }
    void append(const RecordingBuffer& from);
};
