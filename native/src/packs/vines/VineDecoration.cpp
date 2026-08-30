#include "VineDecoration.hpp"

#include <algorithm>
#include <array>

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/render/Renderer.hpp>

#include "VinePassElement.hpp"
#include "VinePack.hpp"

using namespace Render::GL;

namespace OmaFrames::Packs::Vines {
namespace {
constexpr uint8_t ALL_EDGES = DECORATION_EDGE_BOTTOM | DECORATION_EDGE_LEFT | DECORATION_EDGE_RIGHT | DECORATION_EDGE_TOP;

double configuredExtent() {
    return std::max<Config::INTEGER>(config().extent->value(), config().stemThickness->value());
}

CHyprColor withWindowAlpha(const CHyprColor& color, const float alpha) {
    return color.modifyA(color.a * alpha);
}
}

CVineDecoration::CVineDecoration(PHLWINDOW window) : IHyprWindowDecoration(window), m_window(window) {
    m_lastWindowPosition = window->position(Desktop::View::IGeometric::GEOMETRIC_CURRENT);
    m_lastWindowSize     = window->size(Desktop::View::IGeometric::GEOMETRIC_CURRENT);
}

CVineDecoration::~CVineDecoration() {
    damageEntire();
}

SDecorationPositioningInfo CVineDecoration::getPositioningInfo() {
    const double extent = configuredExtent();

    SDecorationPositioningInfo info;
    info.policy         = DECORATION_POSITION_STICKY;
    info.reserved       = false;
    info.priority       = 9980;
    info.edges          = ALL_EDGES;
    info.desiredExtents = {{extent, extent}, {extent, extent}};
    return info;
}

void CVineDecoration::onPositioningReply(const SDecorationPositioningReply& reply) {
    m_assignedGeometry = reply.assignedGeometry;
}

uint64_t CVineDecoration::getDecorationFlags() {
    return DECORATION_PART_OF_MAIN_WINDOW;
}

eDecorationLayer CVineDecoration::getDecorationLayer() {
    return DECORATION_LAYER_OVER;
}

std::string CVineDecoration::getDisplayName() {
    return std::string(DISPLAY_NAME);
}

void CVineDecoration::draw(PHLMONITOR, const float& alpha) {
    if (!config().enabled->value() || !validMapped(m_window))
        return;

    const auto window = m_window.lock();
    if (!window->m_ruleApplicator->decorate().valueOrDefault())
        return;

    g_pHyprRenderer->m_renderPass.add(makeUnique<CVinePassElement>(CVinePassElement::SData{.decoration = this, .alpha = alpha}));
}

void CVineDecoration::drawPass(PHLMONITOR monitor, const float& alpha) {
    const auto window = m_window.lock();
    if (!window || !monitor || !config().enabled->value())
        return;

    const double extent = configuredExtent();
    if (m_assignedGeometry.width < extent + 1 || m_assignedGeometry.height < extent + 1)
        return;

    const auto workspace       = window->m_workspace;
    const auto workspaceOffset = workspace && !window->m_pinned ? workspace->m_renderOffset->value() : Vector2D();

    CBox borderBox = m_assignedGeometry;
    borderBox.translate(g_pDecorationPositioner->getEdgeDefinedPoint(ALL_EDGES, window));
    borderBox.translate(window->m_floatingOffset - monitor->m_position + workspaceOffset);
    borderBox.expand(-extent).scale(monitor->m_scale).round();

    if (borderBox.width < 1 || borderBox.height < 1)
        return;

    static auto borderSize = CConfigValue<Config::INTEGER>("general:border_size");

    const int rounding = window->rounding() == 0 ? 0 : static_cast<int>((window->rounding() + *borderSize) * monitor->m_scale);
    const int thickness = static_cast<int>(config().stemThickness->value());
    const auto stem      = CHyprColor{static_cast<uint64_t>(config().stemColor->value())};
    const auto leaf      = withWindowAlpha(CHyprColor{static_cast<uint64_t>(config().leafColor->value())}, alpha);
    const auto bud       = withWindowAlpha(CHyprColor{static_cast<uint64_t>(config().budColor->value())}, alpha);

    g_pHyprOpenGL->scissor(nullptr);
    g_pHyprOpenGL->renderBorder(borderBox, stem,
                                {.round = rounding, .roundingPower = window->roundingPower(), .borderSize = thickness, .a = alpha, .outerRound = -1});

    const double longSide  = config().leafSize->value() * monitor->m_scale;
    const double shortSide = std::max(4.0, longSide * 0.55);

    const std::array<double, 3> horizontalStops = {0.17, 0.50, 0.82};
    const std::array<double, 2> verticalStops   = {0.31, 0.68};

    for (const double stop : horizontalStops) {
        const double x = borderBox.x + borderBox.width * stop - shortSide / 2.0;

        CBox topLeaf{x, borderBox.y - longSide * 0.58, shortSide, longSide};
        CBox bottomLeaf{x, borderBox.y + borderBox.height - longSide * 0.42, shortSide, longSide};
        g_pHyprOpenGL->renderRect(topLeaf, leaf, {.round = static_cast<int>(shortSide / 2.0)});
        g_pHyprOpenGL->renderRect(bottomLeaf, leaf, {.round = static_cast<int>(shortSide / 2.0)});
    }

    for (const double stop : verticalStops) {
        const double y = borderBox.y + borderBox.height * stop - shortSide / 2.0;

        CBox leftLeaf{borderBox.x - longSide * 0.58, y, longSide, shortSide};
        CBox rightLeaf{borderBox.x + borderBox.width - longSide * 0.42, y, longSide, shortSide};
        g_pHyprOpenGL->renderRect(leftLeaf, leaf, {.round = static_cast<int>(shortSide / 2.0)});
        g_pHyprOpenGL->renderRect(rightLeaf, leaf, {.round = static_cast<int>(shortSide / 2.0)});
    }

    const double budSize = std::max(3.0, thickness * monitor->m_scale * 1.35);
    for (const double stop : std::array<double, 2>{0.34, 0.70}) {
        CBox topBud{borderBox.x + borderBox.width * stop - budSize / 2.0, borderBox.y - budSize / 2.0, budSize, budSize};
        g_pHyprOpenGL->renderRect(topBud, bud, {.round = static_cast<int>(budSize / 2.0)});
    }

    m_extents         = {{extent, extent}, {extent, extent}};
    m_lastRelativeBox = CBox{0, 0, m_lastWindowSize.x, m_lastWindowSize.y}.addExtents(m_extents);

    if (extent != m_lastExtent) {
        m_lastExtent = extent;
        g_pDecorationPositioner->repositionDeco(this);
    }
}

eDecorationType CVineDecoration::getDecorationType() {
    return DECORATION_CUSTOM;
}

void CVineDecoration::updateWindow(PHLWINDOW window) {
    m_lastWindowPosition = window->position(Desktop::View::IGeometric::GEOMETRIC_CURRENT);
    m_lastWindowSize     = window->size(Desktop::View::IGeometric::GEOMETRIC_CURRENT);
    damageEntire();
}

void CVineDecoration::damageEntire() {
    CBox damage = m_lastRelativeBox.copy().translate(m_lastWindowPosition).expand(3);
    g_pHyprRenderer->damageBox(damage);
}
}
