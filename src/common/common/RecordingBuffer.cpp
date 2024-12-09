#include "RecordingBuffer.h"

void RecordingBuffer::initialize(size_t numChannels, size_t bufferSize)
{
    channels.assign(numChannels, vector<float>(bufferSize));
}
