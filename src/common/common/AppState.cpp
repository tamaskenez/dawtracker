#include "AppState.h"

#include "common/common.h"

namespace
{
static const string k_question_mark = "?";
}

const string& AppState::AudioSettingsUI::selectedOutputDeviceName() const
{
    CHECK_OR_RETURN_VAL(selectedOutputDeviceIx < outputDeviceNames.size(), k_question_mark);
    return outputDeviceNames[selectedOutputDeviceIx];
}
const string& AppState::AudioSettingsUI::selectedInputDeviceName() const
{
    CHECK_OR_RETURN_VAL(selectedInputDeviceIx < inputDeviceNames.size(), k_question_mark);
    return inputDeviceNames[selectedInputDeviceIx];
}

AppState::AppState()
{
    auto ts88 = TimeSignature{8, 8};
    auto ts44 = TimeSignature{4, 4};
    auto ts34 = TimeSignature{3, 4};
    auto v1 = ArrangementSection{
      .name = "Verse 1", .structure = Bars{.bars = {Bar{.timeSignature = ts44}, Bar{.timeSignature = ts88}}}
    };
    auto v2 = ArrangementSection{
      .name = "Verse 2", .structure = Bars{.bars = {Bar{.timeSignature = ts44}, Bar{.timeSignature = ts34}}}
    };
    auto ch = ArrangementSection{.name = "Chorus", .structure = Bars{.bars = {Bar{.timeSignature = ts44}}}};
    arrangement = Arrangement{
      .sections = {v1, v2, ch}
    };
}

Rational ArrangementSection::duration(Rational defaultTempo) const
{
    auto actualTempo = tempo.value_or(defaultTempo);
    return switch_variant(
      structure,
      [&](const Bars& x) {
          return x.duration(actualTempo);
      },
      [&](const Period& x) {
          return x.duration(actualTempo);
      },
      [](const Duration& x) {
          return x.seconds;
      }
    );
}

Rational Bar::duration(Rational tempo) const
{
    return Rational(60 * timeSignature.upper,timeSignature.lower) / tempo;
}
Rational Bars::duration(Rational tempo) const
{
    Rational r(0);
    for (auto& b : bars) {
        r += b.duration(tempo);
    }
    return r;
}
Rational Period::duration(Rational tempo) const
{
    return wholeNotes / tempo * 60;
}

Rational AppState::Metronome::bpm()const{
    return tempo * timeSignature.lower;
}
