#include "AudioEngine.h"

#include "utility.h"

#include "common/common.h"

#include "juce_audio_devices/juce_audio_devices.h"

namespace
{
std::atomic_bool s_singleInstanceCreated;

vector<int> toVectorInt(const juce::BigInteger& bi)
{
    vector<int> y;
    y.reserve(size_t(bi.countNumberOfSetBits()));
    for (int i = bi.findNextSetBit(0); i >= 0; i = bi.findNextSetBit(i + 1)) {
        y.push_back(i);
    }
    return y;
}

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
      .activeChannels = toVectorInt(activeChannels)
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

} // namespace

struct AudioEngineImpl
    : public AudioEngine
    , public juce::ChangeListener {
    juce::ScopedJuceInitialiser_GUI juceInitialiser;
    juce::AudioDeviceManager deviceManager;

    AudioEngineImpl()
    {
        deviceManager.addChangeListener(this);
    }

    void changeListenerCallback(juce::ChangeBroadcaster* source) override
    {
        if (source == &deviceManager) {
            fmt::println("Change from deviceManager");
        } else {
            fmt::println("Change from somebody else: {}", fmt::ptr(source));
        }
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
        auto* cad = deviceManager.getCurrentAudioDevice();
        if (!cad) {
            return AudioSettings{};
        }
        ads = deviceManager.getAudioDeviceSetup();
        bool hasActiveChannels = (ads.outputDeviceName.isNotEmpty() && !ads.outputChannels.isZero())
                              || (ads.inputDeviceName.isNotEmpty() && !ads.inputChannels.isZero());
        fmt::println(
          "Recived new AudioDeviceSetup, fs: {} Hz, buffer: {}",
          cad->getCurrentSampleRate(),
          cad->getCurrentBufferSizeSamples()
        );
        fmt::println(
          "out: {} [{}]",
          ads.outputDeviceName.toStdString(),
          toString(cad->getOutputChannelNames(), cad->getActiveOutputChannels())
        );
        fmt::println(
          "out: {} [{}]",
          ads.inputDeviceName.toStdString(),
          toString(cad->getInputChannelNames(), cad->getActiveInputChannels())
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
};

unique_ptr<AudioEngine> AudioEngine::make()
{
    LOG_IF(FATAL, s_singleInstanceCreated.exchange(true)) << "There can only be a single instance of AudioEngine.";
    return make_unique<AudioEngineImpl>();
}
