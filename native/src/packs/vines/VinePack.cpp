#include "VinePack.hpp"

#include <ranges>

#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/render/Renderer.hpp>

#include "VineDecoration.hpp"
#include "../../globals.hpp"

namespace OmaFrames::Packs::Vines {
namespace {
SConfig packConfig;
}

SConfig& config() {
    return packConfig;
}

void registerConfig() {
    packConfig.enabled = makeShared<Config::Values::CBoolValue>("plugin:omaframes:vines:enabled", "Draw the Vines decoration pack", true);
    packConfig.animationEnabled = makeShared<Config::Values::CBoolValue>("plugin:omaframes:vines:animation_enabled", "Animate initial vine growth", true);
    packConfig.themeAware = makeShared<Config::Values::CBoolValue>("plugin:omaframes:vines:theme_aware", "Inherit each window's resolved border colors", true);
    packConfig.growthDurationMs = makeShared<Config::Values::CIntValue>(
        "plugin:omaframes:vines:growth_duration_ms", "Initial growth duration in milliseconds", 1800, Config::Values::SIntValueOptions{.min = 100, .max = 15000});
    packConfig.stemThickness = makeShared<Config::Values::CIntValue>(
        "plugin:omaframes:vines:stem_thickness", "Stem thickness in logical pixels", 3, Config::Values::SIntValueOptions{.min = 1, .max = 12});
    packConfig.extent = makeShared<Config::Values::CIntValue>("plugin:omaframes:vines:extent", "Space around the window used by the vines", 16,
                                                              Config::Values::SIntValueOptions{.min = 4, .max = 40});
    packConfig.leafSize = makeShared<Config::Values::CIntValue>("plugin:omaframes:vines:leaf_size", "Leaf length", 13,
                                                                Config::Values::SIntValueOptions{.min = 4, .max = 30});
    packConfig.stemColor = makeShared<Config::Values::CColorValue>("plugin:omaframes:vines:col.stem", "Stem color", 0xff43945a);
    packConfig.leafColor = makeShared<Config::Values::CColorValue>("plugin:omaframes:vines:col.leaf", "Leaf color", 0xff68c66b);
    packConfig.budColor  = makeShared<Config::Values::CColorValue>("plugin:omaframes:vines:col.bud", "Bud color", 0xffd2d77d);

    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.enabled);
    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.animationEnabled);
    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.themeAware);
    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.growthDurationMs);
    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.stemThickness);
    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.extent);
    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.leafSize);
    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.stemColor);
    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.leafColor);
    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.budColor);
}

void attach(PHLWINDOW window) {
    if (std::ranges::any_of(window->m_windowDecorations,
                            [](const auto& decoration) { return decoration->getDisplayName() == DISPLAY_NAME; }))
        return;

    HyprlandAPI::addWindowDecoration(pluginHandle, window, makeUnique<CVineDecoration>(window));
}

void unload() {
    g_pHyprRenderer->m_renderPass.removeAllOfType(PASS_NAME.data());
}
}
