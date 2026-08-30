#pragma once

#include <string_view>

#include <hyprland/src/config/values/types/BoolValue.hpp>
#include <hyprland/src/config/values/types/ColorValue.hpp>
#include <hyprland/src/config/values/types/IntValue.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>

namespace OmaFrames::Packs::Critter {
inline constexpr std::string_view ID           = "critter";
inline constexpr std::string_view DISPLAY_NAME = "OmaFrames: Chameleon";
inline constexpr std::string_view PASS_NAME    = "COmaFramesCritterPassElement";

struct SConfig {
    SP<Config::Values::CBoolValue>  enabled;
    SP<Config::Values::CBoolValue>  motionEnabled;
    SP<Config::Values::CBoolValue>  themeAware;
    SP<Config::Values::CBoolValue>  hideOnFullscreen;
    SP<Config::Values::CIntValue>   size;
    SP<Config::Values::CIntValue>   walkSpeed;
    SP<Config::Values::CIntValue>   jumpIntervalMs;
    SP<Config::Values::CColorValue> bodyColor;
    SP<Config::Values::CColorValue> accentColor;
};

SConfig& config();
void     registerConfig();
void     start();
void     attach(PHLWINDOW window);
void     stop();
}
