#include "App.h"

#include "AppCtx.h"

#include "audio/AudioIO.h"
#include "common/AppState.h"
#include "common/MetronomeGenerator.h"
#include "common/ReactiveStateEngine.h"
#include "common/msg.h"
#include "platform/platform.h"
#include "ui/UI.h"

namespace
{
AppState::AudioSettingsUI makeAudioSettingsUI(const vector<AudioDeviceProperties>& ads, const ActiveAudioDevices& as)
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

    return AppState::AudioSettingsUI{
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
    AppImpl(UI* uiArg, AppState& appStateArg, ReactiveStateEngine& rseArg)
        : AppCtx(uiArg, appStateArg, rseArg)
    {
        println("main thread: {}", this_thread::get_id());
        audioIO->setAudioCallback([audioEngine_ = audioEngine.get()](
                                    span<const float*> inputChannels, span<float*> outputChannels, size_t numSamples
                                  ) {
            audioEngine_->process(inputChannels, outputChannels, numSamples);
        });

        rse.registerUpdater(appState.audioSettingsUI, [this]() {
            return makeAudioSettingsUI(audioIO->getAudioDevices(), rse.get(appState.activeAudioDevices));
        });
        rse.registerUpdater(appState.playButtonEnabled, [this]() {
            return rse.get(appState.activeAudioDevices.outputDevice).has_value() && !rse.get(appState.clips).empty()
                && !rse.get(appState.clipBeingRecorded);
        });
        rse.registerUpdater(appState.playButton, [this]() {
            return rse.get(appState.clipBeingPlayed);
        });

        rse.registerUpdater(appState.recordButtonEnabled, [this]() {
            return rse.get(appState.activeAudioDevices).canRecord() && !rse.get(appState.clipBeingRecorded)
                && !rse.get(appState.clipBeingPlayed);
        });
        rse.registerUpdater(appState.recordButton, [this]() {
            return rse.get(appState.clipBeingRecorded).has_value();
        });
        rse.registerUpdater(appState.stopButtonEnabled, [this]() {
            return rse.get(appState.clipBeingPlayed) || rse.get(appState.clipBeingRecorded).has_value();
        });
        rse.registerUpdater(appState.stopButton, [this]() {
            return rse.get(appState.clipBeingPlayed) || rse.get(appState.clipBeingRecorded).has_value();
        });
        rse.registerUpdater(
          appState.anyVariableDisplayedOnUIChanged,
          []() {
              return monostate{};
          },
          appState.audioSettingsUI,
          appState.showAudioSettings,
          appState.metronome,
          appState.recordButtonEnabled,
          appState.stopButtonEnabled,
          appState.playButtonEnabled,
          appState.recordButton,
          appState.stopButton,
          appState.playButton,
          appState.inputs,
          appState.outputs,
          appState.clips
        );
        fmt::println("appState.metronome: {}", reinterpret_cast<intptr_t>(&appState.metronome));
        rse.registerUpdater(
          appState.metronomeChanged,
          []() {
              return monostate{};
          },
          appState.metronome
        );
        rse.registerUpdater(appState.inputs, [this]() {
            vector<AudioChannelPropertiesOnUI> inputs;
            if (auto id = rse.get(appState.activeAudioDevices).inputDevice) {
                inputs.reserve(id->channelNames.size());
                for (auto& n : id->channelNames) {
                    inputs.push_back(AudioChannelPropertiesOnUI{.name = n, .enabled = false});
                }
                for (auto i : id->activeChannels) {
                    inputs.at(i).enabled = true;
                }
            }
            return inputs;
        });
        rse.registerUpdater(appState.outputs, [this]() {
            vector<AudioChannelPropertiesOnUI> outputs;
            if (auto id = rse.get(appState.activeAudioDevices).outputDevice) {
                outputs.reserve(id->channelNames.size());
                for (auto& n : id->channelNames) {
                    outputs.push_back(AudioChannelPropertiesOnUI{.name = n, .enabled = false});
                }
                for (auto i : id->activeChannels) {
                    outputs.at(i).enabled = true;
                }
            }
            return outputs;
        });
    }

    void receiveMainMenu(msg::MainMenu m)
    {
        switch (m) {
        case msg::MainMenu::quit:
            sendQuitEventToAppMain();
            break;
        case msg::MainMenu::settings:
            rse.set(appState.showAudioSettings, true);
            break;
        case msg::MainMenu::hideSettings:
            rse.set(appState.showAudioSettings, false);
            break;
        }
    }

    void receiveAudioSettings(const msg::AudioSettings::V& as)
    {
        auto& asui = rse.get(appState.audioSettingsUI);
        size_t selectedOutputDeviceIx = asui.selectedOutputDeviceIx, selectedInputDeviceIx = asui.selectedInputDeviceIx;
        switch_variant(
          as,
          [&](const msg::AudioSettings::OutputDeviceSelected& x) {
              selectedOutputDeviceIx = x.i;
          },
          [&](const msg::AudioSettings::InputDeviceSelected& x) {
              selectedInputDeviceIx = x.i;
          }
        );

        bool validIx = isValidIndexOfContainer(selectedOutputDeviceIx, asui.outputDeviceNames);
        LOG_IF(DFATAL, !validIx) << "Invalid selectedOutputDeviceIx";
        if (!validIx) {
            selectedOutputDeviceIx = 0;
        }
        validIx = isValidIndexOfContainer(selectedInputDeviceIx, asui.inputDeviceNames);
        LOG_IF(DFATAL, !validIx) << "Invalid selectedInputDeviceIx";
        if (!validIx) {
            selectedInputDeviceIx = 0;
        }
        optional<string> selectedOutputDeviceName, selectedInputDeviceName;
        if (selectedOutputDeviceIx > 0) {
            selectedOutputDeviceName = asui.outputDeviceNames[selectedOutputDeviceIx];
        }
        if (selectedInputDeviceIx > 0) {
            selectedInputDeviceName = asui.inputDeviceNames[selectedInputDeviceIx];
        }
        if (auto aad = audioIO->initialize(selectedOutputDeviceName, selectedInputDeviceName)) {
            rse.set(appState.activeAudioDevices, MOVE(*aad));
        } else {
            rse.set(appState.activeAudioDevices, ActiveAudioDevices{});
            assert(false);
            // Platform::messageBoxError(as.error()); //TODO
            LOG(ERROR) << fmt::format(
              "Failed to initialize audio devices to output: \"{}\", input: \"{}\": {}",
              selectedOutputDeviceName.value_or("None"),
              selectedInputDeviceName.value_or("None"),
              aad.error()
            );
        }
    }

    void receiveMetronome(const msg::Metronome::V& m)
    {
        auto metronome = rse.get(appState.metronome);
        switch_variant(
          m,
          [&](const msg::Metronome::On& x) {
              metronome.on = x.b;
          },
          [&](const msg::Metronome::BPM& x) {
              metronome.bpm = x.bpm;
          }
        );
        fmt::println("appState.metronome: {}", reinterpret_cast<intptr_t>(&appState.metronome));
        rse.set(appState.metronome, MOVE(metronome));
    }

    void runAudioIODispatchLoop() override
    {
        audioIO->runDispatchLoopUntil(chr::milliseconds(1));
    }

    void receiveTransport(msg::Transport t)
    {
        switch (t) {
        case msg::Transport::record: {
            LOG(INFO) << "Record";
            CHECK(!rse.get(appState.clipBeingRecorded));
            auto& aad = rse.get(appState.activeAudioDevices);
            CHECK(aad.canRecord());
            rse.setAsDifferent(
              appState.clipBeingRecorded, AudioClip(aad.sampleRate, aad.inputDevice->activeChannels.size())
            );
            rse.set(appState.clipBeingRecordedSeconds, 0);
            audioEngine->record();
        } break;
        case msg::Transport::stop: {
            LOG(INFO) << "Stop";
            CHECK(rse.get(appState.clipBeingPlayed) || rse.get(appState.clipBeingRecorded).has_value());
            rse.set(appState.clipBeingPlayed, false);
            if (rse.get(appState.clipBeingRecorded)) {
                stopRecording();
            }
        } break;
        case msg::Transport::play:
            LOG(INFO) << "Play";
            break;
        }
    }

    void stopRecording()
    {
        auto clipBeingRecordedBorrow = rse.borrowThenSet(appState.clipBeingRecorded);
        CHECK(clipBeingRecordedBorrow->has_value());
        audioEngine->stopRecording();
        rse.borrowThenSet(appState.clips)->push_back(MOVE(**clipBeingRecordedBorrow));
        rse.set(appState.clipBeingRecorded, nullopt);
        rse.set(appState.clipBeingRecordedSeconds, nullopt);
    }

    void receiveAudioEngine(const msg::AudioEngine::V& msg)
    {
        switch_variant(
          msg,
          [this](const msg::AudioEngine::NoFreeRecordingBuffer&) {
              // todo messagebox
              LOG(ERROR) << "Main thread couldn't keep up with the recording. Recording stopped.";
              stopRecording();
          },
          [this](const msg::AudioEngine::RecordingBufferRecorded& x) {
              // Add buffer to current clip.
              auto clipBeingRecordedBorrow = rse.borrowThenSet(appState.clipBeingRecorded);
              if (!clipBeingRecordedBorrow->has_value()) {
                  LOG(INFO) << "RecordingBufferRecorded when not recording";
                  return;
              }
              CHECK(clipBeingRecordedBorrow->has_value() && !(*clipBeingRecordedBorrow)->channels.empty());
              (*clipBeingRecordedBorrow)->append(*x.recordingBuffer);
              x.recordingBuffer->sentToApp = false;
              LOG(INFO) << fmt::format(
                "[{}]->sentToApp = false, {} ms",
                fmt::ptr(x.recordingBuffer),
                1000.0 * chr::duration<double>(chr::high_resolution_clock::now() - x.timestamp)
              );
              rse.set(
                appState.clipBeingRecordedSeconds,
                (*clipBeingRecordedBorrow)->channels[0].size() / rse.get(appState.activeAudioDevices).sampleRate
              );
          },
          [this](const msg::AudioEngine::PlayedTime& x) {
              rse.set(appState.playedTime, x.t);
          }
        );
        NOP;
    }
    void playClip(size_t i)
    {
        auto& clips = rse.get(appState.clips);
        CHECK(i < clips.size());
        rse.set(appState.clipBeingPlayed, true);
        audioEngine->play(AudioClip(clips[i]));
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
            if (auto r = audioIO->enableInOrOut(InOrOut::in, f->name, f->enabled); !r) {
                // TODO: messageBox(r)
                LOG(ERROR) << fmt::format("Failed changing input {} to {}: {}", f->name, f->enabled, r.error());
            }
            rse.set(appState.activeAudioDevices, audioIO->getActiveAudioDevices());
        } else if (auto* h = std::any_cast<msg::OutputChanged>(&msg)) {
            if (auto r = audioIO->enableInOrOut(InOrOut::out, h->name, h->enabled); !r) {
                // TODO: messageBox(r)
                LOG(ERROR) << fmt::format("Failed changing output {} to {}: {}", h->name, h->enabled, r.error());
            }
            rse.set(appState.activeAudioDevices, audioIO->getActiveAudioDevices());
        } else if (auto* g = std::any_cast<msg::AudioEngine::V>(&msg)) {
            receiveAudioEngine(*g);
        } else if (auto* i = std::any_cast<msg::PlayClip>(&msg)) {
            playClip(i->i);
        } else {
            LOG(DFATAL) << fmt::format("Invalid message: {}", msg.type().name());
        }

        if (!rse.isUpToDate(appState.metronomeChanged)) {
            rse.updateIfNeeded(appState.metronomeChanged);
            audioEngine->sendStateChangerFn([metronome = appState.metronome](AudioEngineState& s) {
                s.metronome.on = metronome.on;
                s.metronome.bpm = metronome.bpm;
            });
        }
    }

    void receiveAudioIO(const msg::AudioIO::V& msg)
    {
        switch_variant(
          msg,
          [this](const msg::AudioIO::Changed&) {
              rse.set(appState.activeAudioDevices, audioIO->getActiveAudioDevices());
          },
          [this](const msg::AudioIO::AudioCallbacksAboutToStart& x) {
              audioEngine->audioCallbacksAboutToStart(x.sampleRate, x.bufferSize, x.numInputChannels);
          },
          [this](const msg::AudioIO::AudioCallbacksStopped&) {
              audioEngine->audioCallbacksStopped();
          }
        );
    }
    bool getAndClearIfUIRefreshNeeded() override
    {
        if (rse.isUpToDate(appState.anyVariableDisplayedOnUIChanged)) {
            return false;
        }
        rse.updateIfNeeded(appState.anyVariableDisplayedOnUIChanged);
        return true;
    }
};

unique_ptr<App> App::make(UI* ui, AppState& appState, ReactiveStateEngine& rse)
{
    return make_unique<AppImpl>(ui, appState, rse);
}
