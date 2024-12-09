#pragma once

#include "std.h"

enum InputOrOutput {
    in,
    out
};

struct AudioDevice {
    InputOrOutput ioo;
    string name;
    string type;
    vector<string> channelNames;
    vector<double> sampleRates;
    vector<int> bufferSizes;
    int defaultBufferSize;
};

struct AudioSettings {
    struct Device {
        string name;
        vector<string> channelNames;
        vector<size_t> activeChannels;
    };
    optional<Device> outputDevice, inputDevice;
    int bufferSize = 0;
    double sampleRate = 0;

    bool canRecord() const
    {
        return inputDevice && !inputDevice->activeChannels.empty();
    }
};
