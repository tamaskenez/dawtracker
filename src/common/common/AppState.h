#pragma once

#include "std.h"

#include "AudioClip.h"
#include "audiodevicetypes.h"

#include "boost/rational.hpp"
using Rational = boost::rational<int64_t>;

struct TimeSignature {
    int upper, lower;
};
struct Bar {
    TimeSignature timeSignature;
    Rational duration(Rational tempo) const;
};

struct Bars {
    vector<Bar> bars;
    Rational duration(Rational tempo) const;
};
struct Beats {
    Rational beats;
    Rational duration(Rational tempo) const;
};
struct Duration {
    Rational duration;
};

struct ArrangementSection {
    string name;
    optional<Rational> tempo;
    variant<Bars, Beats, Duration> structure;

    Rational duration(Rational defaultTempo) const;
};

struct Arrangement {
    vector<ArrangementSection> sections;
};

struct AudioChannelPropertiesOnUI {
    string name;
    bool enabled;
    bool operator==(const AudioChannelPropertiesOnUI&) const = default;
};

struct AppState {
    AppState();

    struct AudioSettingsUI {
        vector<string> outputDeviceNames, inputDeviceNames;
        size_t selectedOutputDeviceIx = 0, selectedInputDeviceIx = 0;

        const string& selectedOutputDeviceName() const;
        const string& selectedInputDeviceName() const;
        bool operator==(const AudioSettingsUI&) const = default;
    } audioSettingsUI;
    bool showAudioSettings = false;

    struct Metronome {
        bool on = false;
        float bpm = 120.0;
        bool operator==(const Metronome&) const = default;
    } metronome;

    bool recordButtonEnabled = false, stopButtonEnabled = false, playButtonEnabled = false;
    bool recordButton = false, stopButton = false, playButton = false;

    vector<AudioChannelPropertiesOnUI> inputs;
    vector<AudioChannelPropertiesOnUI> outputs;

    ActiveAudioDevices activeAudioDevices;
    vector<AudioClip> clips;

    monostate anyVariableDisplayedOnUIChanged;
    monostate metronomeChanged;

    optional<double> clipBeingRecordedSeconds;
    optional<double> playedTime;
    optional<AudioClip> clipBeingRecorded;
    bool clipBeingPlayed = false;

    Arrangement arrangement;
};
