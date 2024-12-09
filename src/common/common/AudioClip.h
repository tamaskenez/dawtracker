#pragma once

#include "std.h"

struct RecordingBuffer;

struct AudioClip {
    AudioClip(double sampleRate, size_t numChannels);
    double sampleRate;
    vector<deque<float>> channels;

    void append(const RecordingBuffer& from);
};
