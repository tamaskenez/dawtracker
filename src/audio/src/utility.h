#pragma once

#include "common/std.h"

#include "juce_core/juce_core.h"

vector<string> toVectorString(const juce::StringArray& x);

template<class T>
vector<T> toVector(const juce::Array<T>& x)
{
    vector<T> y;
    y.reserve(size_t(x.size()));
    for (const T& i : x) {
        y.push_back(i);
    }
    return y;
}
