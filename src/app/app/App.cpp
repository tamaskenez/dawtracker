#include "App.h"

#include "AppCtx.h"
#include "audio/AudioIO.h"
#include "common/MetronomeGenerator.h"
#include "common/msg.h"
#include "platform/platform.h"
#include "ui/UI.h"

namespace
{
UIState::AudioSettings refreshSettingsUIState(const vector<AudioDevice>& ads, const AudioSettings& as)
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

    return UIState::AudioSettings{
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
        println("main thread: {}", this_thread::get_id());
        audioIO->setAudioCallback([audioEngine_ = audioEngine.get()](
                                    span<const float*> inputChannels, span<float*> outputChannels, size_t numSamples
                                  ) {
            audioEngine_->process(inputChannels, outputChannels, numSamples);
        });
        audioEngineUpdateMetronome();
    }

    void updateUIStateDerivedFields()
    {
        uiState->recordButton = appState.audioSettings.inputDevice.has_value();
    }

    void audioEngineUpdateMetronome()
    {
        audioEngine->sendStateChangerFn([this](AudioEngineState& s) {
            s.metronome.on = uiState->metronome.on;
            s.metronome.bpm = uiState->metronome.bpm;
        });
    }

    void receiveMainMenu(msg::MainMenu m)
    {
        switch (m) {
        case msg::MainMenu::quit:
            sendQuitEventToAppMain();
            break;
        case msg::MainMenu::settings:
            uiState->audioSettings = refreshSettingsUIState(audioIO->getAudioDevices(), appState.audioSettings);
            uiState->showAudioSettings = true;
            sendRefreshUIEventToAppMain();
            break;
        }
    }

    void receiveAudioSettings(const msg::AudioSettings::V& as)
    {
        switch_variant(
          as,
          [this](const msg::AudioSettings::OutputDeviceSelected& x) {
              uiState->audioSettings.selectedOutputDeviceIx = x.i;
              update_fromUiStateAudioSettings_toAudioIO_toAppState_toUIState();
          },
          [this](const msg::AudioSettings::InputDeviceSelected& x) {
              uiState->audioSettings.selectedInputDeviceIx = x.i;
              update_fromUiStateAudioSettings_toAudioIO_toAppState_toUIState();
          }
        );
        updateUIStateDerivedFields();
    }

    void receiveMetronome(const msg::Metronome::V& m)
    {
        switch_variant(
          m,
          [this](const msg::Metronome::On& x) {
              uiState->metronome.on = x.b;
          },
          [this](const msg::Metronome::BPM& x) {
              uiState->metronome.bpm = x.bpm;
          }
        );
        audioEngineUpdateMetronome();
        sendRefreshUIEventToAppMain();
    }

    void runAudioIODispatchLoop() override
    {
        audioIO->runDispatchLoopUntil(chr::milliseconds(1));
    }

    void receiveTransport(msg::Transport t)
    {
        switch (t) {
        case msg::Transport::record:
            break;
        case msg::Transport::stop:
            break;
        case msg::Transport::play:
            break;
        }
    }

    void receive(std::any&& msg) override
    {
        if (auto* a = std::any_cast<msg::MainMenu>(&msg)) {
            receiveMainMenu(*a);
        } else if (auto* b = std::any_cast<msg::AudioSettings::V>(&msg)) {
            receiveAudioSettings(*b);
        } else if (auto* c = std::any_cast<msg::AudioIO::V>(&msg)) {
            receiveAudioIO(*c);
        } else if (auto* d = std::any_cast<msg::Metronome::V>(&msg)) {
            receiveMetronome(*d);
        } else if (auto* e = std::any_cast<msg::Transport>(&msg)) {
            receiveTransport(*e);
        } else if (auto* f = std::any_cast<msg::InputChanged>(&msg)) {
            if (auto r = audioIO->enableInput(f->name, f->enabled); !r) {
                // TODO: messageBox(r)
                LOG(ERROR) << fmt::format("Failed changing input {} to {}: {}", f->name, f->enabled, r.error());
            }
            update_fromAudioIO_toAppState_toUIState();
        } else {
            LOG(DFATAL) << fmt::format("Invalid message: {}", msg.type().name());
        }
    }

    void receiveAudioIO(const msg::AudioIO::V& msg)
    {
        switch_variant(
          msg,
          [this](const msg::AudioIO::Changed&) {
              update_fromAudioIO_toAppState_toUIState();
          },
          [this](const msg::AudioIO::AudioCallbacksAboutToStart& x) {
              audioEngine->audioCallbacksAboutToStart(x.sampleRate, x.bufferSize);
          },
          [this](const msg::AudioIO::AudioCallbacksStopped&) {
              audioEngine->audioCallbacksStopped();
          }
        );
    }

    void update_uiStateInputs_from_appStateAudioSettings()
    {
        uiState->inputs.clear();
        if (auto id = appState.audioSettings.inputDevice) {
            uiState->inputs.reserve(id->channelNames.size());
            for (auto& n : id->channelNames) {
                uiState->inputs.push_back(UIState::Input{.name = n, .enabled = false});
            }
            for (auto i : id->activeChannels) {
                uiState->inputs.at(i).enabled = true;
            }
        }
    }
    void update_fromAudioIO_toAppState_toUIState()
    {
        appState.audioSettings = audioIO->getAudioSettings();
        uiState->audioSettings = refreshSettingsUIState(audioIO->getAudioDevices(), appState.audioSettings);
        update_uiStateInputs_from_appStateAudioSettings();
        sendRefreshUIEventToAppMain();
    }
    void update_fromUiStateAudioSettings_toAudioIO_toAppState_toUIState()
    {
        auto& uas = uiState->audioSettings;
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
        if (auto as = audioIO->initialize(selectedOutputDeviceName, selectedInputDeviceName)) {
            appState.audioSettings = *as;
        } else {
            appState.audioSettings = AudioSettings{};
            assert(false);
            // Platform::messageBoxError(as.error()); //TODO
            LOG(ERROR) << fmt::format(
              "Failed to initialize audio devices to output: \"{}\", input: \"{}\": {}",
              selectedOutputDeviceName.value_or("None"),
              selectedInputDeviceName.value_or("None"),
              as.error()
            );
        }
        uas = refreshSettingsUIState(audioIO->getAudioDevices(), appState.audioSettings);
        update_uiStateInputs_from_appStateAudioSettings();
        sendRefreshUIEventToAppMain();
    }
};

unique_ptr<App> App::make(UI* ui)
{
    return make_unique<AppImpl>(ui);
}
