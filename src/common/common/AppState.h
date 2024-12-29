#pragma once

#include "AudioClip.h"
#include "Id.h"
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
        Rational tempo{120, 4}; // Whole notes per minute.
        TimeSignature timeSignature = TimeSignature{4, 4};
        Rational bpm() const;
        bool operator==(const Metronome&) const = default;
    } metronome;

    bool recordButtonEnabled = false, stopButtonEnabled = false, playButtonEnabled = false;
    bool recordButton = false, stopButton = false, playButton = false;

    vector<AudioChannelPropertiesOnUI> inputs;
    vector<AudioChannelPropertiesOnUI> outputs;

    ActiveAudioDevices activeAudioDevices;

    monostate anyVariableDisplayedOnUIChanged;
    monostate metronomeChanged;

    optional<double> clipBeingRecordedSeconds;
    optional<double> playedTime;
    optional<AudioClip> clipBeingRecorded;
    bool clipBeingPlayed = false;

    unordered_map<Id<AudioClip>, AudioClip> clips;
    unordered_map<Id<Section>, Section> sections;
    vector<Id<Section>> sectionOrder;
    int nextNewTrackId = 1;
    unordered_map<Id<Track>, Track> tracks;
    vector<Id<Track>> trackOrder;

    unordered_map<Id<ClipLink>, ClipLink> clipLinks;
};
