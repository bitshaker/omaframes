#pragma once
#include <hyprland/src/config/values/types/BoolValue.hpp>
#include <hyprland/src/config/values/types/ColorValue.hpp>
#include <hyprland/src/config/values/types/IntValue.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
namespace OmaFrames::Packs::Accents {
enum class Kind { Fireflies, Frost, Railway };
struct SConfig {
    SP<Config::Values::CBoolValue> enabled, animationEnabled, themeAware;
    SP<Config::Values::CIntValue> speed;
    SP<Config::Values::CColorValue> color;
};
SConfig &config(Kind kind);
void registerConfig(Kind kind);
void attach(PHLWINDOW window, Kind kind);
void stop();
inline constexpr auto PASS_NAME = "COmaFramesAccentPassElement";
} // namespace OmaFrames::Packs::Accents
