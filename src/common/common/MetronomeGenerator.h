#pragma once

#include "std.h"

struct MetronomeGenerator {
    double timeSinceLastStart = 0;

    void generate(double fs, float bpm, span<float> buf);
};
