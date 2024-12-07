namespace
{
void printChannels(string_view title, juce::StringArray ns, optional<juce::BigInteger> d)
{
    if (ns.isEmpty()) {
        return;
    }
    vector<string> ms;
    for (int i : vi::iota(0, ns.size())) {
        auto n = ns[i];
        if (d && (*d)[i]) {
            ms.push_back(fmt::format("[{}]", n.toStdString()));
        } else {
            ms.push_back(fmt::format("{}", n.toStdString()));
        }
    }
    fmt::println("{}({})", title, fmt::join(ms, " "));
}
} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInitialiser;

    juce::AudioDeviceManager adm;

    auto ads = adm.getAudioDeviceSetup();

    const auto& deviceTypes = adm.getAvailableDeviceTypes();

    auto r = adm.initialiseWithDefaultDevices(2, 2);
    fmt::println("r: {}", r.toStdString());

    for (auto& dt : deviceTypes) {
        fmt::println(
          "Device type `{}`, {}",
          dt->getTypeName().toStdString(),
          dt->hasSeparateInputsAndOutputs() ? "separate I/O" : "no separate I/O"
        );
        for (bool input : {false, true}) {
            auto deviceNames = dt->getDeviceNames(input);
            auto dix = dt->getDefaultDeviceIndex(input);

            for (int i : vi::iota(0, deviceNames.size())) {
                auto dn = deviceNames[i];
                fmt::print("- {} dev#{} ", input ? "IN " : "OUT", i);
                fmt::print("{}", dix == i ? "(def)" : "     ");
                fmt::println(": {}", dn.toStdString());
                UNUSED unique_ptr<juce::AudioIODevice> d;
                if (input) {
                    d.reset(dt->createDevice("", dn));
                } else {
                    d.reset(dt->createDevice(dn, ""));
                }
                printChannels("  outputs: ", d->getOutputChannelNames(), d->getDefaultOutputChannels());
                printChannels("   inputs: ", d->getInputChannelNames(), d->getDefaultInputChannels());
                fmt::println("Sample rates: ({})", fmt::join(d->getAvailableSampleRates(), " "));
                vector<string> bss;
                for (auto bs : d->getAvailableBufferSizes()) {
                    if (bs == d->getDefaultBufferSize()) {
                        bss.push_back(fmt::format("[{}]", bs));
                    } else {
                        bss.push_back(fmt::format("{}", bs));
                    }
                }
                fmt::println("Buffer sizes: ({})", fmt::join(bss, " "));
                fmt::println(
                  "Latency {} output, {} input", d->getOutputLatencyInSamples(), d->getInputLatencyInSamples()
                );
                fmt::println("Control panel: {}", d->hasControlPanel() ? "yes" : "no");
            }
        }
    }
    return EXIT_SUCCESS;
}
