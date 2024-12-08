#include "MetronomeGenerator.h"

namespace
{
constexpr double tau = .015;
constexpr double lambda = 1.0 / tau;
constexpr double volume = 0.2;
constexpr double f1 = 954.42;
constexpr double f2 = 1092.07;
constexpr double f3 = 1222.59;
constexpr double f4 = 630.62;
constexpr double f5 = 1907.20;
constexpr double f6 = 1484.75;
constexpr double f7 = 573.69;
} // namespace

void MetronomeGenerator::generate(double fs, float bpm, span<float> buf)
{
    double beatInSec = 60.0 / bpm;
    double secPerSample = 1.0 / fs;
    for (size_t i : vi::iota(0u, buf.size())) {
        double envelope = exp(-timeSinceLastStart * lambda);
        double _2pit = 2.0 * std::numbers::pi * timeSinceLastStart;
        buf[i] = float(tanh(
          volume * envelope
          * (cos(_2pit * f1) + cos(_2pit * f2) + cos(_2pit * f3) + cos(_2pit * f4) + cos(_2pit * f5) + cos(_2pit * f6) + cos(_2pit * f7))
        ));
        timeSinceLastStart = fmod(timeSinceLastStart + secPerSample, beatInSec);
    }
}
