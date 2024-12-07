#include "App.h"

#include "AppCtx.h"
#include "audio/AudioEngine.h"
#include "common/MetronomeGenerator.h"
#include "common/msg.h"
#include "platform/Msg.h"
#include "platform/platform.h"
#include "ui/UI.h"

namespace
{
uistate::AudioSettings refreshSettingsUIState(const vector<AudioDevice>& ads, const AudioSettings& as)
{
    vector<string> ods, ids;
    ods.push_back("None");
    ids.push_back("None");
    for (auto& x : ads) {
        switch (x.ioo) {
        case out:
            ods.push_back(x.name);
            break;
        case in:
            ids.push_back(x.name);
            break;
        }
    }
    size_t sod = 0, sid = 0;
    if (as.outputDevice) {
        for (size_t i : vi::iota(0u, ods.size())) {
            if (as.outputDevice->name == ods[i]) {
                sod = i;
                break;
            }
        }
    }
    if (as.inputDevice) {
        for (size_t i : vi::iota(0u, ids.size())) {
            if (as.inputDevice->name == ids[i]) {
                sid = i;
                break;
            }
        }
    }

    return uistate::AudioSettings{
      .outputDeviceNames = MOVE(ods),
      .inputDeviceNames = MOVE(ids),
      .selectedOutputDeviceIx = sod,
      .selectedInputDeviceIx = sid
    };
}
} // namespace

struct AppImpl
    : public App
    , public AppCtx {
    explicit AppImpl(UI* uiArg)
        : AppCtx(uiArg)
    {
        ui->setMetronome(&uiState.metronome);
        // audioEngine->setAudioCallback(...); //TODO
    }

    void receiveMainMenu(msg::MainMenu m)
    {
        switch (m) {
        case msg::MainMenu::quit:
            sendQuitEventToAppMain();
            break;
        case msg::MainMenu::settings:
            uiState.audioSettings = refreshSettingsUIState(audioEngine->getAudioDevices(), appState.audioSettings);
            ui->openSettings(&uiState.audioSettings);
            break;
        }
    }

    void receiveAudioSettings(const msg::AudioSettings::V& as)
    {
        switch_variant(
          as,
          [this](const msg::AudioSettings::OutputDeviceSelected& x) {
              uiState.audioSettings.selectedOutputDeviceIx = x.i;
              update_fromUiStateAudioSettings_toAudioEngine_toAppState_toUIState();
          },
          [this](const msg::AudioSettings::InputDeviceSelected& x) {
              uiState.audioSettings.selectedInputDeviceIx = x.i;
              update_fromUiStateAudioSettings_toAudioEngine_toAppState_toUIState();
          }
        );
    }

    void receiveMetronome(const msg::Metronome::V& m)
    {
        switch_variant(
          m,
          [this](const msg::Metronome::On& x) {
              uiState.metronome.on = x.b;
          },
          [this](const msg::Metronome::BPM& x) {
              uiState.metronome.bpm = x.bpm;
          }
        );
        sendRefreshUIEventToAppMain();
    }

    void runAudioEngineDispatchLoop() override
    {
        audioEngine->runDispatchLoopUntil(chr::milliseconds(1));
    }

    void receive(Msg&& msg) override
    {
        auto pl = MOVE(msg.payload);

        if (auto* a = std::any_cast<msg::MainMenu>(&pl)) {
            receiveMainMenu(*a);
        } else if (auto* b = std::any_cast<msg::AudioSettings::V>(&pl)) {
            receiveAudioSettings(*b);
        } else if (auto* c = std::any_cast<msg::AudioEngine::V>(&pl)) {
            receiveAudioEngine(*c);
        } else if (auto* d = std::any_cast<msg::Metronome::V>(&pl)) {
            receiveMetronome(*d);
        } else {
            LOG(DFATAL) << fmt::format("Invalid message: {}", pl.type().name());
        }
    }

    void receiveAudioEngine(const msg::AudioEngine::V& msg)
    {
        switch_variant(msg, [this](const msg::AudioEngine::Changed&) {
            update_fromAudioEngine_toAppState_toUIState();
        });
    }

    void update_fromAudioEngine_toAppState_toUIState()
    {
        appState.audioSettings = audioEngine->getAudioSettings();
        uiState.audioSettings = refreshSettingsUIState(audioEngine->getAudioDevices(), appState.audioSettings);
        sendRefreshUIEventToAppMain();
    }
    void update_fromUiStateAudioSettings_toAudioEngine_toAppState_toUIState()
    {
        auto& uas = uiState.audioSettings;
        bool validIx = isValidIndexOfContainer(uas.selectedOutputDeviceIx, uas.outputDeviceNames);
        LOG_IF(DFATAL, !validIx) << "Invalid selectedOutputDeviceIx";
        if (!validIx) {
            uas.selectedOutputDeviceIx = 0;
        }
        validIx = isValidIndexOfContainer(uas.selectedInputDeviceIx, uas.inputDeviceNames);
        LOG_IF(DFATAL, !validIx) << "Invalid selectedInputDeviceIx";
        if (!validIx) {
            uas.selectedInputDeviceIx = 0;
        }
        optional<string> selectedOutputDeviceName, selectedInputDeviceName;
        if (uas.selectedOutputDeviceIx > 0) {
            selectedOutputDeviceName = uas.outputDeviceNames[uas.selectedOutputDeviceIx];
        }
        if (uas.selectedInputDeviceIx > 0) {
            selectedInputDeviceName = uas.inputDeviceNames[uas.selectedInputDeviceIx];
        }
        if (auto as = audioEngine->initialize(selectedOutputDeviceName, selectedInputDeviceName)) {
            appState.audioSettings = *as;
        } else {
            appState.audioSettings = AudioSettings{};
            assert(false);
            // Platform::messageBoxError(as.error()); //TODO
        }
        uas = refreshSettingsUIState(audioEngine->getAudioDevices(), appState.audioSettings);
        sendRefreshUIEventToAppMain();
    }
};

unique_ptr<App> App::make(UI* ui)
{
    return make_unique<AppImpl>(ui);
}
