#pragma once

#include <span>
#include <string_view>

#include <hyprland/src/desktop/DesktopTypes.hpp>

namespace OmaFrames::Packs {
struct SBuiltInPack {
    std::string_view id;
    void (*registerConfig)();
    void (*start)();
    void (*attach)(PHLWINDOW window);
    void (*stop)();
};

std::span<const SBuiltInPack> builtInPacks();
}
