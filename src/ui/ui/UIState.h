#pragma once

#include "common/std.h"

struct UIState {
    struct AudioSettings {
        vector<string> outputDeviceNames, inputDeviceNames;
        size_t selectedOutputDeviceIx = 0, selectedInputDeviceIx = 0;

        const string& selectedOutputDeviceName() const;
        const string& selectedInputDeviceName() const;
    };

    struct Metronome {
        bool on = false;
        float bpm = 120.0;
    };

    AudioSettings audioSettings;
    bool showAudioSettings = false;
    Metronome metronome;

    bool recordButtonEnabled = false, stopButtonEnabled = false, playButtonEnabled = false;
    bool recordButton = false, stopButton = false, playButton = false;

    struct Input {
        string name;
        bool enabled;
    };
    struct Output {
        string name;
        bool enabled;
    };
    vector<Input> inputs;
    vector<Output> outputs;

    optional<double> clipBeingRecordedSeconds;
};
