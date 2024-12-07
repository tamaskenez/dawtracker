#pragma once

namespace msg
{
enum class MainMenu {
    quit,
    settings
};
namespace audiosettings
{
struct OutputDeviceSelected {
    size_t i;
};
struct InputDeviceSelected {
    size_t i;
};
} // namespace audiosettings
} // namespace msg
