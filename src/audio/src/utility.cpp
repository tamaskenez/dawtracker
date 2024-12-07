#include "utility.h"

vector<string> toVectorString(const juce::StringArray& x)
{
    vector<string> y;
    y.reserve(size_t(x.size()));
    for (auto& i : x) {
        y.push_back(i.toStdString());
    }
    return y;
}
