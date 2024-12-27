#pragma once

#include "common.h"

#include "AudioClip.h"
#include "audiodevicetypes.h"

struct TimeSignature {
    int upper, lower;
    bool    operator==(const TimeSignature&)const=default;
};
struct Bar {
    TimeSignature timeSignature;
    Rational duration(Rational tempo) const;
};

struct Bars {
    vector<Bar> bars;
    Rational duration(Rational tempo) const;
};

struct Period {
    Rational wholeNotes;
    Rational duration(Rational tempo) const;
};
struct Duration {
    Rational seconds;
};

struct ArrangementSection {
    string name;
    optional<Rational> tempo;
    variant<Bars, Period, Duration> structure;

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
        Rational tempo {120,4};
        TimeSignature timeSignature = TimeSignature{4,4};
        Rational bpm()const;
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
