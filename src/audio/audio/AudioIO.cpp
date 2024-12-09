#include "AudioIO.h"

#include "utility.h"

#include "common/common.h"
#include "common/msg.h"
#include "platform/AppMsgQueue.h"

#include "juce_audio_devices/juce_audio_devices.h"

namespace
{
std::atomic_bool s_singleInstanceCreated;

optional<AudioSettings::Device> toAudioSettingsDevice(
  const juce::String& deviceName, const juce::StringArray& channelNames, const juce::BigInteger& activeChannels
)
{
    if (deviceName.isEmpty()) {
        return nullopt;
    }
    return AudioSettings::Device{
      .name = deviceName.toStdString(),
      .channelNames = toVectorString(channelNames),
      .activeChannels = toVectorSizeT(activeChannels)
    };
}

string toString(const juce::StringArray& channelNames, const juce::BigInteger& activeChannels)
{
    vector<string> s;
    for (int i : vi::iota(0, channelNames.size())) {
        if (activeChannels[i]) {
            s.push_back(fmt::format("*{}*", channelNames[i].toStdString()));
        } else {
            s.push_back(channelNames[i].toStdString());
        }
    }
    return fmt::format("{}", fmt::join(s, " "));
}

struct AudioIODeviceCallback : public juce::AudioIODeviceCallback {
    vector<const float*> inputChannels;
    vector<float*> outputChannels;
    AudioCallbackFn audioCallback;

    void audioDeviceIOCallbackWithContext(
      const float* const* inputChannelData,
      int numInputChannels,
      float* const* outputChannelData,
      int numOutputChannels,
      int numSamples,
      UNUSED const juce::AudioIODeviceCallbackContext& context
    ) override
    {
        if (!audioCallback) {
            return;
        }
        // Memory allocation but only after device changes when the channel count increases.
        // We intentionally don't reserve a reasonable (16) channels in the constructor to experience the possible
        // effects of the memory allocation more often.
        inputChannels.clear();
        inputChannels.reserve(size_t(numInputChannels));
        for (size_t i : vi::iota(0u, size_t(numInputChannels))) {
            inputChannels.push_back(inputChannelData[i]);
        }
        outputChannels.clear();
        outputChannels.reserve(size_t(numOutputChannels));
        for (size_t i : vi::iota(0u, size_t(numOutputChannels))) {
            outputChannels.push_back(outputChannelData[i]);
        }
        audioCallback(inputChannels, outputChannels, size_t(numSamples));
    }
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override
    {
        sendToAppSync(msg::AudioIO::V(msg::AudioIO::AudioCallbacksAboutToStart{
          .sampleRate = device->getCurrentSampleRate(), .bufferSize = size_t(device->getCurrentBufferSizeSamples())
        }));
    }

    void audioDeviceStopped() override
    {
        sendToAppSync(MAKE_VARIANT_V(msg::AudioIO, AudioCallbacksStopped{}));
    }

    void audioDeviceError(const juce::String& errorMessage) override
    {
        fmt::println("ERROR(audioDevice): {}", errorMessage.toStdString());
    }
};

} // namespace

struct AudioIOImpl
    : public AudioIO
    , public juce::ChangeListener {
    juce::ScopedJuceInitialiser_GUI juceInitialiser;
    AudioIODeviceCallback deviceCallback;
    juce::AudioDeviceManager deviceManager;

    AudioIOImpl()
    {
        deviceManager.addChangeListener(this);
        deviceManager.addAudioCallback(&deviceCallback);
    }
    ~AudioIOImpl() override
    {
        deviceManager.removeAudioCallback(&deviceCallback);
        deviceManager.removeChangeListener(this);
    }

    void changeListenerCallback(juce::ChangeBroadcaster* source) override
    {
        CHECK_OR_RETURN(source == &deviceManager);
        sendToApp(MAKE_VARIANT_V(msg::AudioIO, Changed{}));
    }

    void runDispatchLoopUntil(chr::milliseconds d) override
    {
        juce::MessageManager::getInstance()->runDispatchLoopUntil(int(d.count()));
    }

    vector<AudioDevice> getAudioDevices() override
    {
        vector<AudioDevice> devices;
        for (auto& dt : deviceManager.getAvailableDeviceTypes()) {
            for (bool input : {false, true}) {
                auto deviceNames = dt->getDeviceNames(input);
                for (int i : vi::iota(0, deviceNames.size())) {
                    auto dn = deviceNames[i];
                    unique_ptr<juce::AudioIODevice> d(input ? dt->createDevice("", dn) : dt->createDevice(dn, ""));
                    devices.push_back(AudioDevice{
                      .ioo = input ? InputOrOutput::in : InputOrOutput::out,
                      .name = d->getName().toStdString(),
                      .type = d->getTypeName().toStdString(),
                      .channelNames = toVectorString(input ? d->getInputChannelNames() : d->getOutputChannelNames()),
                      .sampleRates = toVector<double>(d->getAvailableSampleRates()),
                      .bufferSizes = toVector<int>(d->getAvailableBufferSizes()),
                      .defaultBufferSize = d->getDefaultBufferSize()
                    });
                }
            }
        }
        return devices;
    }
    expected<AudioSettings, string>
    initialize(optional<string> outputDeviceName, optional<string> inputDeviceName) override
    {
        auto ads = deviceManager.getAudioDeviceSetup();

        // If there's a change, apply it.
        if (outputDeviceName.value_or(string()) != ads.outputDeviceName.toStdString()
            || inputDeviceName.value_or(string()) != ads.inputDeviceName.toStdString()) {
            auto preferredSetupOptions = juce::AudioDeviceManager::AudioDeviceSetup{
              .outputDeviceName = outputDeviceName.value_or(string()),
              .inputDeviceName = inputDeviceName.value_or(string())
            };
            auto result = deviceManager.setAudioDeviceSetup(preferredSetupOptions, true);
            if (result.isNotEmpty()) {
                return unexpected(result.toStdString());
            }
        }
        return getAudioSettings();
    }
    AudioSettings getAudioSettings() override
    {
        auto* cad = deviceManager.getCurrentAudioDevice();
        if (!cad) {
            return AudioSettings{};
        }
        auto ads = deviceManager.getAudioDeviceSetup();
        bool hasActiveChannels = (ads.outputDeviceName.isNotEmpty() && !ads.outputChannels.isZero())
                              || (ads.inputDeviceName.isNotEmpty() && !ads.inputChannels.isZero());
        fmt::println(
          "Received new AudioDeviceSetup, fs: {} Hz, buffer: {}",
          cad->getCurrentSampleRate(),
          cad->getCurrentBufferSizeSamples()
        );
        fmt::println(
          "out: {} [{}], lat: {}",
          ads.outputDeviceName.toStdString(),
          toString(cad->getOutputChannelNames(), cad->getActiveOutputChannels()),
          cad->getOutputLatencyInSamples()
        );
        fmt::println(
          "in: {} [{}], lat: {}",
          ads.inputDeviceName.toStdString(),
          toString(cad->getInputChannelNames(), cad->getActiveInputChannels()),
          cad->getInputLatencyInSamples()
        );

        assert(ads.outputChannels == cad->getActiveOutputChannels());
        assert(ads.inputChannels == cad->getActiveInputChannels());
        if (hasActiveChannels) {
            assert(ads.sampleRate == cad->getCurrentSampleRate());
            assert(ads.bufferSize == cad->getCurrentBufferSizeSamples());
        }
        return AudioSettings{
          .outputDevice =
            toAudioSettingsDevice(ads.outputDeviceName, cad->getOutputChannelNames(), cad->getActiveOutputChannels()),
          .inputDevice =
            toAudioSettingsDevice(ads.inputDeviceName, cad->getInputChannelNames(), cad->getActiveInputChannels()),
          .bufferSize = cad->getCurrentBufferSizeSamples(),
          .sampleRate = cad->getCurrentSampleRate()
        };
    }
    void setAudioCallback(AudioCallbackFn callbackFn) override
    {
        juce::ScopedLock scopedLock(deviceManager.getAudioCallbackLock());
        deviceCallback.audioCallback = MOVE(callbackFn);
    }
    expected<void, string> enableInput(string_view name, bool enabled) override
    {
        auto* cad = deviceManager.getCurrentAudioDevice();
        CHECK_OR_RETURN_VAL(cad, unexpected("[internal error] There's no current audio device."));
        auto icn = cad->getInputChannelNames();
        optional<int> requestedChix;
        for (int chix : vi::iota(0, icn.size())) {
            if (icn[chix].toStdString() == name) {
                requestedChix = chix;
                break;
            }
        }
        auto ads = deviceManager.getAudioDeviceSetup();
        CHECK_OR_RETURN_VAL(
          requestedChix,
          unexpected(fmt::format(
            "[internal error] Input channel name {} not found in the current input device ({})",
            name,
            ads.inputDeviceName.toStdString()
          ))
        );

        assert(cad->getActiveInputChannels() == ads.inputChannels);
        ads.inputChannels.setBit(*requestedChix, enabled);
        ads.useDefaultInputChannels = false;
        ads.useDefaultOutputChannels = false;
        auto result = deviceManager.setAudioDeviceSetup(ads, true);
        if (result.isNotEmpty()) {
            return unexpected(result.toStdString());
        }
        return {};
    }
};

unique_ptr<AudioIO> AudioIO::make()
{
    LOG_IF(FATAL, s_singleInstanceCreated.exchange(true)) << "There can only be a single instance of AudioIO.";
    return make_unique<AudioIOImpl>();
}
