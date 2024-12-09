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

vector<int> toVectorInt(const juce::BigInteger& bi)
{
    vector<int> y;
    y.reserve(size_t(bi.countNumberOfSetBits()));
    for (int i = bi.findNextSetBit(0); i >= 0; i = bi.findNextSetBit(i + 1)) {
        y.push_back(i);
    }
    return y;
}

vector<size_t> toVectorSizeT(const juce::BigInteger& bi)
{
    vector<size_t> y;
    y.reserve(size_t(bi.countNumberOfSetBits()));
    for (int i = bi.findNextSetBit(0); i >= 0; i = bi.findNextSetBit(i + 1)) {
        y.push_back(size_t(i));
    }
    return y;
}
