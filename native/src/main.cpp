#define WLR_USE_UNSTABLE

#include <algorithm>
#include <ranges>
#include <stdexcept>
#include <string>

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/desktop/state/WindowState.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/event/EventBus.hpp>
#include <hyprland/src/render/Renderer.hpp>

#include "VineDecoration.hpp"
#include "globals.hpp"

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

namespace {
void attachToWindow(PHLWINDOW window) {
    if (std::ranges::any_of(window->m_windowDecorations, [](const auto& decoration) { return decoration->getDisplayName() == "OmaVines prototype"; }))
        return;

    HyprlandAPI::addWindowDecoration(pluginHandle, window, makeUnique<CVineDecoration>(window));
}
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    pluginHandle = handle;

    const std::string runningHash = __hyprland_api_get_hash();
    const std::string headerHash  = __hyprland_api_get_client_hash();
    if (runningHash != headerHash) {
        HyprlandAPI::addNotification(pluginHandle, "[omavines] Hyprland/header version mismatch", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[omavines] Hyprland/header version mismatch");
    }

    config.enabled       = makeShared<Config::Values::CBoolValue>("plugin:omavines:enabled", "Draw the prototype vine decoration", true);
    config.stemThickness = makeShared<Config::Values::CIntValue>("plugin:omavines:stem_thickness", "Stem thickness in logical pixels", 3,
                                                                  Config::Values::SIntValueOptions{.min = 1, .max = 12});
    config.extent = makeShared<Config::Values::CIntValue>("plugin:omavines:extent", "Space around the window used by the vines", 16,
                                                           Config::Values::SIntValueOptions{.min = 4, .max = 40});
    config.leafSize = makeShared<Config::Values::CIntValue>("plugin:omavines:leaf_size", "Prototype leaf length", 13,
                                                             Config::Values::SIntValueOptions{.min = 4, .max = 30});
    config.stemColor = makeShared<Config::Values::CColorValue>("plugin:omavines:col.stem", "Stem color", 0xff43945a);
    config.leafColor = makeShared<Config::Values::CColorValue>("plugin:omavines:col.leaf", "Leaf color", 0xff68c66b);
    config.budColor  = makeShared<Config::Values::CColorValue>("plugin:omavines:col.bud", "Bud color", 0xffd2d77d);

    HyprlandAPI::addConfigValueV2(pluginHandle, config.enabled);
    HyprlandAPI::addConfigValueV2(pluginHandle, config.stemThickness);
    HyprlandAPI::addConfigValueV2(pluginHandle, config.extent);
    HyprlandAPI::addConfigValueV2(pluginHandle, config.leafSize);
    HyprlandAPI::addConfigValueV2(pluginHandle, config.stemColor);
    HyprlandAPI::addConfigValueV2(pluginHandle, config.leafColor);
    HyprlandAPI::addConfigValueV2(pluginHandle, config.budColor);
    HyprlandAPI::reloadConfig();

    static auto newWindowListener = Event::bus()->m_events.window.open.listen([](PHLWINDOW window) { attachToWindow(window); });

    for (auto& window : Desktop::windowState()->windows()) {
        if (window->isHidden() || !window->m_isMapped)
            continue;

        attachToWindow(window);
    }

    HyprlandAPI::addNotification(pluginHandle, "[omavines] Native decoration prototype loaded", CHyprColor{0.35, 0.85, 0.42, 1.0}, 3500);
    return {"omavines", "Experimental living window decorations", "OmaVines contributors", "0.1.0-prototype"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    g_pHyprRenderer->m_renderPass.removeAllOfType("CVinePassElement");
}
