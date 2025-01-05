#pragma once

#include <any>

enum class Platform {
    mac,
    win,
    linux
};

inline Platform getPlatform()
{
#if defined _WIN32
    return Platform::win;
#elifdef __APPLE__
    return Platform::mac;
#elifdef __LINUX__
    return Platform::linux;
#else
  #error
#endif
}

void sendQuitEventToAppMain();
