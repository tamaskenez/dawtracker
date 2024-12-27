#pragma once

#include "std.h"

struct MetronomeGenerator {
    double timeSinceLastStart = 0;

    void generate(double fs, double bpm, span<float> buf);
};
