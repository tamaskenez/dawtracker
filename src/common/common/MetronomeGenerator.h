#pragma once

#include "std.h"

struct MetronomeGenerator {
    optional<float> bpm;
    double timeSinceLastStart = 0;

    void generateAdd(double fs, span<float> buf);
};
