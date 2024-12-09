#pragma once

#include "common/std.h"

struct RecordingBuffer {
    vector<vector<float>> channels;
    std::atomic_bool sentToApp;

    void initialize(size_t numChannels, size_t bufferSize);
};
