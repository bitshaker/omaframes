#define WLR_USE_UNSTABLE

#include <stdexcept>
#include <string>

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/desktop/state/WindowState.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/event/EventBus.hpp>
#include <hyprland/src/render/Renderer.hpp>

#include "globals.hpp"
#include "packs/BuiltInPacks.hpp"

namespace {
CHyprSignalListener newWindowListener;

void attachBuiltInPacks(PHLWINDOW window) {
    for (const auto& pack : OmaFrames::Packs::builtInPacks())
        pack.attach(window);
}
}

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    pluginHandle = handle;

    const std::string runningHash = __hyprland_api_get_hash();
    const std::string headerHash  = __hyprland_api_get_client_hash();
    if (runningHash != headerHash) {
        HyprlandAPI::addNotification(pluginHandle, "[omaframes] Hyprland/header version mismatch", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[omaframes] Hyprland/header version mismatch");
    }

    for (const auto& pack : OmaFrames::Packs::builtInPacks())
        pack.registerConfig();

    HyprlandAPI::reloadConfig();

    newWindowListener = Event::bus()->m_events.window.open.listen([](PHLWINDOW window) { attachBuiltInPacks(window); });

    for (auto& window : Desktop::windowState()->windows()) {
        if (window->isHidden() || !window->m_isMapped)
            continue;

        attachBuiltInPacks(window);
    }

    for (const auto& pack : OmaFrames::Packs::builtInPacks())
        pack.start();

    HyprlandAPI::addNotification(pluginHandle, "[omaframes] Vines and Chameleon loaded", CHyprColor{0.35, 0.85, 0.42, 1.0}, 3500);
    return {"omaframes", "Extensible living window effects; includes Vines and Chameleon", "Joe Homs", "0.5.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    newWindowListener.reset();

    const auto packs = OmaFrames::Packs::builtInPacks();
    for (auto iterator = packs.rbegin(); iterator != packs.rend(); ++iterator)
        iterator->stop();
}
