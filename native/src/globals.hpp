#pragma once

#include <hyprland/src/config/values/types/BoolValue.hpp>
#include <hyprland/src/config/values/types/ColorValue.hpp>
#include <hyprland/src/config/values/types/IntValue.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>

inline HANDLE pluginHandle = nullptr;

struct SPluginConfig {
    SP<Config::Values::CBoolValue> enabled;
    SP<Config::Values::CIntValue>  stemThickness;
    SP<Config::Values::CIntValue>  extent;
    SP<Config::Values::CIntValue>  leafSize;
    SP<Config::Values::CColorValue> stemColor;
    SP<Config::Values::CColorValue> leafColor;
    SP<Config::Values::CColorValue> budColor;
};

inline SPluginConfig config = {};
