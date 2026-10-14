#include "AccentPack.hpp"
#include "../../globals.hpp"
#include "AccentDecoration.hpp"
#include <array>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/render/Renderer.hpp>
#include <ranges>
namespace OmaFrames::Packs::Accents {
namespace {
std::array<SConfig, 3> configs;
// Hyprland stores config names as const char pointers, so keep ownership.
std::array<std::array<std::string, 5>, 3> configNames;
constexpr const char *ids[] = {"fireflies", "frost", "railway"};
constexpr const char *names[] = {"Fireflies", "Frost", "Railway"};
} // namespace
SConfig &config(Kind kind) { return configs[static_cast<int>(kind)]; }
void registerConfig(Kind kind) {
    auto &c = config(kind);
    const auto prefix = std::string("plugin:omaframes:") + ids[static_cast<int>(kind)] + ":";
    auto &keys = configNames[static_cast<int>(kind)];
    keys = {prefix + "enabled", prefix + "animation_enabled", prefix + "theme_aware", prefix + "speed", prefix + "col.accent"};
    c.enabled = makeShared<Config::Values::CBoolValue>(keys[0].c_str(), "Enable this effect pack", false);
    c.animationEnabled = makeShared<Config::Values::CBoolValue>(keys[1].c_str(), "Animate this effect; disable for a static frame", true);
    c.themeAware = makeShared<Config::Values::CBoolValue>(keys[2].c_str(), "Inherit the resolved window border color", false);
    c.speed = makeShared<Config::Values::CIntValue>(keys[3].c_str(), "Movement speed in logical pixels per second", 44,
                                                    Config::Values::SIntValueOptions{.min = 5, .max = 150});
    const uint64_t colors[] = {0xffd9f576, 0xff9ddfff, 0xffe8a45b};
    c.color = makeShared<Config::Values::CColorValue>(keys[4].c_str(), "Effect accent color", colors[static_cast<int>(kind)]);
    HyprlandAPI::addConfigValueV2(pluginHandle, c.enabled);
    HyprlandAPI::addConfigValueV2(pluginHandle, c.animationEnabled);
    HyprlandAPI::addConfigValueV2(pluginHandle, c.themeAware);
    HyprlandAPI::addConfigValueV2(pluginHandle, c.speed);
    HyprlandAPI::addConfigValueV2(pluginHandle, c.color);
}
void attach(PHLWINDOW window, Kind kind) {
    const auto name = std::string("OmaFrames: ") + names[static_cast<int>(kind)];
    if (std::ranges::any_of(window->m_windowDecorations, [&](const auto &d) { return d->getDisplayName() == name; }))
        return;
    HyprlandAPI::addWindowDecoration(pluginHandle, window, makeUnique<CAccentDecoration>(window, kind));
}
void stop() { g_pHyprRenderer->m_renderPass.removeAllOfType(PASS_NAME); }
} // namespace OmaFrames::Packs::Accents
