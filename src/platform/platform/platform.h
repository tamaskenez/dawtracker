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

uint32_t appQueueNotificationSdlEventType();
uint32_t refreshUISdlEventType();
void sendToApp(std::any&& payload);
void sendQuitEventToAppMain();
void sendRefreshUIEventToAppMain();
