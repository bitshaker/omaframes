#include "CritterPack.hpp"

#include <ranges>
#include <string>
#include <string_view>

#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/render/Renderer.hpp>

#include "../../globals.hpp"
#include "CritterDecoration.hpp"
#include "CritterDirector.hpp"

namespace OmaFrames::Packs::Critter {
namespace {
SConfig packConfig;
SP<SHyprCtlCommand> controlCommand;

std::string_view trim(const std::string_view value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos)
        return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string runControlCommand(const eHyprCtlOutputFormat format, const std::string arguments) {
    constexpr std::string_view COMMAND = "omaframes";

    auto action = trim(arguments);
    if (action.starts_with(COMMAND) &&
        (action.size() == COMMAND.size() || std::string_view{" \t\r\n"}.contains(action[COMMAND.size()])))
        action = trim(action.substr(COMMAND.size()));
    if (action.empty() || action == "status")
        return director().status(format == FORMAT_JSON);

    if (action == "jump") {
        std::string error;
        const bool  started = director().forceJump(error);
        if (format == FORMAT_JSON)
            return started ? "{\"success\":true}\n" : "{\"success\":false,\"error\":\"" + error + "\"}\n";
        return started ? "ok\n" : "error: " + error + "\n";
    }

    return format == FORMAT_JSON ? "{\"success\":false,\"error\":\"usage: hyprctl omaframes [status|jump]\"}\n"
                                 : "usage: hyprctl omaframes [status|jump]\n";
}
}

SConfig& config() {
    return packConfig;
}

void registerConfig() {
    packConfig.enabled           = makeShared<Config::Values::CBoolValue>("plugin:omaframes:critter:enabled", "Draw OmaCritter", true);
    packConfig.motionEnabled     = makeShared<Config::Values::CBoolValue>("plugin:omaframes:critter:motion_enabled", "Allow OmaCritter to walk and jump", true);
    packConfig.themeAware        = makeShared<Config::Values::CBoolValue>("plugin:omaframes:critter:theme_aware", "Inherit the host window's border colors", true);
    packConfig.hideOnFullscreen  = makeShared<Config::Values::CBoolValue>("plugin:omaframes:critter:hide_on_fullscreen", "Hide OmaCritter in true fullscreen", true);
    packConfig.size              = makeShared<Config::Values::CIntValue>(
        "plugin:omaframes:critter:size", "OmaCritter size in logical pixels", 30, Config::Values::SIntValueOptions{.min = 16, .max = 64});
    packConfig.walkSpeed         = makeShared<Config::Values::CIntValue>(
        "plugin:omaframes:critter:walk_speed", "OmaCritter walking speed in logical pixels per second", 44, Config::Values::SIntValueOptions{.min = 8, .max = 240});
    packConfig.jumpIntervalMs    = makeShared<Config::Values::CIntValue>(
        "plugin:omaframes:critter:jump_interval_ms", "Average time between inter-window jumps", 12000,
        Config::Values::SIntValueOptions{.min = 1500, .max = 60000});
    packConfig.bodyColor         = makeShared<Config::Values::CColorValue>("plugin:omaframes:critter:col.body", "OmaCritter body color", 0xff68c66b);
    packConfig.accentColor       = makeShared<Config::Values::CColorValue>("plugin:omaframes:critter:col.accent", "OmaCritter accent color", 0xffd2d77d);

    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.enabled);
    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.motionEnabled);
    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.themeAware);
    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.hideOnFullscreen);
    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.size);
    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.walkSpeed);
    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.jumpIntervalMs);
    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.bodyColor);
    HyprlandAPI::addConfigValueV2(pluginHandle, packConfig.accentColor);
}

void start() {
    director().start();
    controlCommand = HyprlandAPI::registerHyprCtlCommand(pluginHandle, SHyprCtlCommand{
                                                                           .name  = "omaframes",
                                                                           .exact = false,
                                                                           .fn    = runControlCommand,
                                                                       });
}

void attach(PHLWINDOW window) {
    if (std::ranges::any_of(window->m_windowDecorations,
                            [](const auto& decoration) { return decoration->getDisplayName() == DISPLAY_NAME; }))
        return;

    HyprlandAPI::addWindowDecoration(pluginHandle, window, makeUnique<CCritterDecoration>(window));
    director().topologyChanged();
}

void stop() {
    if (controlCommand) {
        HyprlandAPI::unregisterHyprCtlCommand(pluginHandle, controlCommand);
        controlCommand.reset();
    }
    director().stop();
    g_pHyprRenderer->m_renderPass.removeAllOfType(PASS_NAME.data());
}
}
