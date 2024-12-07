#include "App.h"

#include "AppEnv.h"
#include "Dialog.h"
#include "audio/AudioEngine.h"
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

struct Settings : public Dialog {
    AppEnv* appEnv;
    explicit Settings(AppEnv* e)
        : appEnv(e)
    {
    }
    void applyUIStateToAppStateAndAudioEngine()
    {
        auto& uas = appEnv->uiState.audioSettings;
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
        if (auto as = appEnv->audioEngine->initialize(selectedOutputDeviceName, selectedInputDeviceName)) {
            appEnv->appState.audioSettings = *as;
        } else {
            appEnv->appState.audioSettings = AudioSettings{};
            assert(false);
            // Platform::messageBoxError(as.error()); //TODO
        }
        uas = refreshSettingsUIState(appEnv->audioEngine->getAudioDevices(), appEnv->appState.audioSettings);
    }

    bool receive(UNUSED std::any&& msg) override
    {
        if (auto* a = std::any_cast<msg::audiosettings::OutputDeviceSelected>(&msg)) {
            appEnv->uiState.audioSettings.selectedOutputDeviceIx = a->i;
            applyUIStateToAppStateAndAudioEngine();
            return true;
        } else if (auto* b = std::any_cast<msg::audiosettings::InputDeviceSelected>(&msg)) {
            appEnv->uiState.audioSettings.selectedInputDeviceIx = b->i;
            applyUIStateToAppStateAndAudioEngine();
            return true;
        }
        return false;
    }
};

struct MainWindow : public Dialog {
    AppEnv* appEnv;
    explicit MainWindow(AppEnv* e)
        : appEnv(e)
    {
    }
    bool receive(std::any&& msg) override
    {
        if (auto* a = std::any_cast<msg::MainMenu>(&msg)) {
            switch (*a) {
            case msg::MainMenu::quit:
                sendQuitEventToAppMain();
                return true;
            case msg::MainMenu::settings:
                appEnv->uiState.audioSettings =
                  refreshSettingsUIState(appEnv->audioEngine->getAudioDevices(), appEnv->appState.audioSettings);
                appEnv->ui->openSettings(&appEnv->uiState.audioSettings);
                appEnv->appUI.dialogs.push_back(make_unique<Settings>(appEnv));
                return true;
            }
        }
        return false;
    }
};

struct AppImpl
    : public App
    , public AppEnv {
    explicit AppImpl(UI* uiArg)
        : AppEnv(uiArg)
    {
        appUI.dialogs.push_back(make_unique<MainWindow>(this));
    }
    void receive(Msg&& msg) override
    {
        auto pl = MOVE(msg.payload);
        if (!appUI.receive(MOVE(pl))) {
            LOG(DFATAL) << fmt::format("Invalid message: {}", pl.type().name());
        }
    }
    void runAudioEngineDispatchLoop() override
    {
        audioEngine->runDispatchLoopUntil(chr::milliseconds(5));
    }
};

unique_ptr<App> App::make(UI* ui)
{
    return make_unique<AppImpl>(ui);
}
