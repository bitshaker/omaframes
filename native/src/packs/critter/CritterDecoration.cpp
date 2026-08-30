#include "CritterDecoration.hpp"

#include <algorithm>
#include <cmath>

#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/render/Renderer.hpp>

#include "CritterDirector.hpp"
#include "CritterPack.hpp"

namespace OmaFrames::Packs::Critter {
namespace {
constexpr uint8_t ALL_EDGES = DECORATION_EDGE_BOTTOM | DECORATION_EDGE_LEFT | DECORATION_EDGE_RIGHT | DECORATION_EDGE_TOP;
}

CCritterDecoration::CCritterDecoration(PHLWINDOW window) : IHyprWindowDecoration(window), m_window(window) {
    m_lastExtent = director().decorationExtent();
}

CCritterDecoration::~CCritterDecoration() {
    damageEntire();
}

SDecorationPositioningInfo CCritterDecoration::getPositioningInfo() {
    const double extent = director().decorationExtent();

    SDecorationPositioningInfo info;
    info.policy         = DECORATION_POSITION_ABSOLUTE;
    info.reserved       = false;
    info.priority       = 9970;
    info.edges          = ALL_EDGES;
    info.desiredExtents = {{extent, extent}, {extent, extent}};
    return info;
}

void CCritterDecoration::onPositioningReply(const SDecorationPositioningReply&) {
    damageEntire();
}

void CCritterDecoration::draw(PHLMONITOR monitor, const float& alpha) {
    if (!config().enabled->value() || !validMapped(m_window))
        return;

    const auto window = m_window.lock();
    if (!window->m_ruleApplicator->decorate().valueOrDefault())
        return;

    const double extent = director().decorationExtent();
    if (std::abs(extent - m_lastExtent) > 0.01) {
        m_lastExtent = extent;
        g_pDecorationPositioner->repositionDeco(this);
    }

    director().queueWindowPass(window, monitor, alpha);
}

eDecorationType CCritterDecoration::getDecorationType() {
    return DECORATION_CUSTOM;
}

void CCritterDecoration::updateWindow(PHLWINDOW window) {
    director().windowUpdated(window);
}

void CCritterDecoration::damageEntire() {
    if (const auto window = m_window.lock(); window && window->m_isMapped)
        g_pHyprRenderer->damageWindow(window);
}

uint64_t CCritterDecoration::getDecorationFlags() {
    return DECORATION_NON_SOLID;
}

eDecorationLayer CCritterDecoration::getDecorationLayer() {
    return DECORATION_LAYER_OVER;
}

std::string CCritterDecoration::getDisplayName() {
    return std::string(DISPLAY_NAME);
}
}
