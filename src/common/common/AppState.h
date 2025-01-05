#pragma once

#include "AudioClip.h"
#include "Id.h"
#include "ReactiveStateEngine.h"
#include "audiodevicetypes.h"
#include "common.h"

struct TimeSignature {
    int upper, lower;
    bool operator==(const TimeSignature&) const = default;
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

struct Section;

enum class TimeUnit {
    wholeNotes,
    seconds
};

// AudioClip linked into an Arrangement section.
struct ClipLink {
    Id<Section> sectionId;
    Id<AudioClip> audioClipId;
    TimeUnit timeUnit; // For start
    Rational clipOriginToSectionStart;
};

struct Section {
    string name;
    optional<Rational> tempo;
    variant<Bars, Period, Duration> structure;
    vector<ClipLink> clipLinksAnchored;
    vector<ClipLink> clipLinksOverlapping;

    Rational duration(Rational defaultTempo) const;
};

struct Track {
    string name;
    bool operator==(const Track&) const = default;
};

struct AudioChannelPropertiesOnUI {
    string name;
    bool enabled;
    bool operator==(const AudioChannelPropertiesOnUI&) const = default;
};

struct AppState {
    AppState();

    ReactiveStateEngine rse;

    struct AudioSettingsUI {
        vector<string> outputDeviceNames, inputDeviceNames;
        size_t selectedOutputDeviceIx = 0, selectedInputDeviceIx = 0;

        const string& selectedOutputDeviceName() const;
        const string& selectedInputDeviceName() const;
        bool operator==(const AudioSettingsUI&) const = default;
    };
    rse::Computed<AudioSettingsUI> audioSettingsUI;
    rse::Value<bool> showAudioSettings{false};

    struct Metronome {
        bool on = false;
        Rational tempo{120, 4}; // Whole notes per minute.
        TimeSignature timeSignature = TimeSignature{4, 4};
        Rational bpm() const;
        bool operator==(const Metronome&) const = default;
    };
    rse::Value<Metronome> metronome;

    rse::Computed<bool> recordButtonEnabled, stopButtonEnabled, playButtonEnabled;
    rse::Computed<bool> recordButton, stopButton;

    rse::Computed<vector<AudioChannelPropertiesOnUI>> inputs, outputs;

    rse::Value<ActiveAudioDevices> activeAudioDevices;

    rse::Computed<monostate> anyVariableDisplayedOnUIChanged, metronomeChanged;

    rse::Value<optional<double>> clipBeingRecordedSeconds;
    rse::Value<optional<double>> playedTime;
    rse::Value<optional<AudioClip>> clipBeingRecorded;
    rse::Value<bool> clipBeingPlayed{false};

    rse::Value<unordered_map<Id<AudioClip>, AudioClip>> clips;
    rse::Value<unordered_map<Id<Section>, Section>> sections;
    rse::Value<vector<Id<Section>>> sectionOrder;
    rse::UndoableValue<int> nextNewTrackId{1};
    rse::UndoableValue<unordered_map<Id<Track>, Track>> tracks;
    rse::UndoableValue<vector<Id<Track>>> trackOrder;

    rse::Value<unordered_map<Id<ClipLink>, ClipLink>> clipLinks;
};
