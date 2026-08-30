#pragma once

#include <string_view>

#include <hyprland/src/config/values/types/BoolValue.hpp>
#include <hyprland/src/config/values/types/ColorValue.hpp>
#include <hyprland/src/config/values/types/IntValue.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>

namespace OmaFrames::Packs::Vines {
inline constexpr std::string_view ID           = "vines";
inline constexpr std::string_view DISPLAY_NAME = "OmaFrames: Vines";
inline constexpr std::string_view PASS_NAME    = "COmaFramesVinesPassElement";

struct SConfig {
    SP<Config::Values::CBoolValue>  enabled;
    SP<Config::Values::CBoolValue>  animationEnabled;
    SP<Config::Values::CBoolValue>  themeAware;
    SP<Config::Values::CIntValue>   growthDurationMs;
    SP<Config::Values::CIntValue>   stemThickness;
    SP<Config::Values::CIntValue>   extent;
    SP<Config::Values::CIntValue>   leafSize;
    SP<Config::Values::CColorValue> stemColor;
    SP<Config::Values::CColorValue> leafColor;
    SP<Config::Values::CColorValue> budColor;
};

SConfig& config();
void     registerConfig();
void     start();
void     attach(PHLWINDOW window);
void     stop();
}
