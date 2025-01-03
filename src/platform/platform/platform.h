#pragma once

#include <any>

enum class Platform {
    mac,
    win
};

inline Platform getPlatform()
{
#if defined _WIN32
    return Platform::win;
#elif __APPLE__
    return Platform::mac;
#else
  #error
#endif
}

void sendQuitEventToAppMain();
