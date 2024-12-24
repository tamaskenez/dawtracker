#pragma once

#include "std.h"

enum InOrOut {
    in,
    out
};

struct AudioDeviceProperties {
    InOrOut ioo;
    string name;
    string type;
    vector<string> channelNames;
    vector<double> sampleRates;
    vector<int> bufferSizes;
    int defaultBufferSize;
};

struct ActiveAudioDevices {
    struct Device {
        string name;
        vector<string> channelNames;
        vector<size_t> activeChannels;
        bool operator==(const Device&) const = default;
    };
    optional<Device> outputDevice, inputDevice;
    int bufferSize = 0;
    double sampleRate = 0;

    bool canRecord() const
    {
        return inputDevice && !inputDevice->activeChannels.empty();
    }
    bool operator==(const ActiveAudioDevices&) const = default;
};
