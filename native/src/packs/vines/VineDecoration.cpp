#include "VineDecoration.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/event/EventBus.hpp>
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/render/Renderer.hpp>

#include "VinePack.hpp"
#include "VinePassElement.hpp"

using namespace Render::GL;

namespace OmaFrames::Packs::Vines {
namespace {
constexpr uint8_t ALL_EDGES = DECORATION_EDGE_BOTTOM | DECORATION_EDGE_LEFT | DECORATION_EDGE_RIGHT | DECORATION_EDGE_TOP;
constexpr double  SPROUT_WINDOW = 0.075;

struct SPalette {
    Config::CGradientValueData stem;
    CHyprColor                 leaf;
    CHyprColor                 bud;
};

double configuredExtent() {
    return std::max<Config::INTEGER>(config().extent->value(), config().stemThickness->value());
}

double smoothstep(const double value) {
    const double clamped = std::clamp(value, 0.0, 1.0);
    return clamped * clamped * (3.0 - 2.0 * clamped);
}

double sproutProgress(const double growth, const double threshold) {
    return smoothstep((growth - threshold) / SPROUT_WINDOW);
}

CHyprColor mixColors(const CHyprColor& from, const CHyprColor& to, const double amount) {
    return CHyprColor{static_cast<float>(from.r + (to.r - from.r) * amount), static_cast<float>(from.g + (to.g - from.g) * amount),
                      static_cast<float>(from.b + (to.b - from.b) * amount), static_cast<float>(from.a + (to.a - from.a) * amount)};
}

CHyprColor mixWithWhite(const CHyprColor& color, const double amount) {
    return mixColors(color, Colors::WHITE, amount);
}

CHyprColor gradientColorAt(const Config::CGradientValueData& gradient, const double position) {
    if (gradient.m_colors.size() == 1)
        return gradient.m_colors.front();

    const double scaled = std::clamp(position, 0.0, 1.0) * static_cast<double>(gradient.m_colors.size() - 1);
    const auto   index  = static_cast<size_t>(std::floor(scaled));
    if (index >= gradient.m_colors.size() - 1)
        return gradient.m_colors.back();

    return mixColors(gradient.m_colors[index], gradient.m_colors[index + 1], scaled - static_cast<double>(index));
}

SPalette resolvedPalette(PHLWINDOW window) {
    if (config().themeAware->value() && !window->m_realBorderColor.m_colors.empty()) {
        const auto& colors = window->m_realBorderColor.m_colors;
        return {
            .stem = window->m_realBorderColor,
            .leaf = mixWithWhite(colors.front(), 0.18),
            .bud  = mixWithWhite(colors.back(), 0.42),
        };
    }

    const auto stem = CHyprColor{static_cast<uint64_t>(config().stemColor->value())};
    return {
        .stem = Config::CGradientValueData{stem},
        .leaf = CHyprColor{static_cast<uint64_t>(config().leafColor->value())},
        .bud  = CHyprColor{static_cast<uint64_t>(config().budColor->value())},
    };
}
}

CVineDecoration::CVineDecoration(PHLWINDOW window) :
    IHyprWindowDecoration(window), m_window(window), m_growthStartedAt(std::chrono::steady_clock::now()),
    m_tickListener(Event::bus()->m_events.tick.listen([this] {
        if (animationRunning())
            damageEntire();
    })) {
    m_lastWindowPosition = window->position(Desktop::View::IGeometric::GEOMETRIC_CURRENT);
    m_lastWindowSize     = window->size(Desktop::View::IGeometric::GEOMETRIC_CURRENT);

    const double extent = configuredExtent();
    m_extents           = {{extent, extent}, {extent, extent}};
    m_lastRelativeBox   = CBox{0, 0, m_lastWindowSize.x, m_lastWindowSize.y}.addExtents(m_extents);
}

CVineDecoration::~CVineDecoration() {
    damageEntire();
}

double CVineDecoration::growthProgress() const {
    if (!config().animationEnabled->value())
        return 1.0;

    const auto durationMs = std::max<Config::INTEGER>(100, config().growthDurationMs->value());
    const auto elapsed    = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - m_growthStartedAt).count();
    return smoothstep(elapsed / static_cast<double>(durationMs));
}

bool CVineDecoration::animationRunning() const {
    return config().enabled->value() && config().animationEnabled->value() && growthProgress() < 1.0;
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

    const double growth    = growthProgress();
    const double thickness = std::max(1.0, static_cast<double>(config().stemThickness->value()) * monitor->m_scale);
    const auto   palette   = resolvedPalette(window);
    const double perimeter = 2.0 * (borderBox.width + borderBox.height);
    double       remaining = perimeter * growth;

    const auto reveal = [&remaining](const double length) {
        const double visible = std::clamp(remaining, 0.0, length);
        remaining -= visible;
        return visible;
    };

    const auto drawStemSegment = [&](const CBox& box, const double palettePosition) {
        if (box.width <= 0 || box.height <= 0)
            return;

        const auto color = gradientColorAt(palette.stem, palettePosition);
        g_pHyprOpenGL->renderRect(box, color.modifyA(color.a * alpha), {.round = static_cast<int>(std::ceil(thickness / 2.0))});
    };

    const double top = reveal(borderBox.width);
    drawStemSegment(CBox{borderBox.x, borderBox.y - thickness / 2.0, top, thickness}, 0.125);

    const double right = reveal(borderBox.height);
    drawStemSegment(CBox{borderBox.x + borderBox.width - thickness / 2.0, borderBox.y, thickness, right}, 0.375);

    const double bottom = reveal(borderBox.width);
    drawStemSegment(CBox{borderBox.x + borderBox.width - bottom, borderBox.y + borderBox.height - thickness / 2.0, bottom, thickness}, 0.625);

    const double left = reveal(borderBox.height);
    drawStemSegment(CBox{borderBox.x - thickness / 2.0, borderBox.y + borderBox.height - left, thickness, left}, 0.875);

    const double longSide  = config().leafSize->value() * monitor->m_scale;
    const double shortSide = std::max(4.0, longSide * 0.55);

    const auto drawGrowingRect = [&](CBox box, const CHyprColor& color, const double threshold) {
        const double sprout = sproutProgress(growth, threshold);
        if (sprout <= 0)
            return;

        box.x += box.width * (1.0 - sprout) / 2.0;
        box.y += box.height * (1.0 - sprout) / 2.0;
        box.width *= sprout;
        box.height *= sprout;

        const auto grownColor = color.modifyA(color.a * alpha * sprout);
        g_pHyprOpenGL->renderRect(box, grownColor, {.round = static_cast<int>(std::min(box.width, box.height) / 2.0)});
    };

    const std::array<double, 3> horizontalStops = {0.17, 0.50, 0.82};
    const std::array<double, 2> verticalStops   = {0.31, 0.68};

    for (const double stop : horizontalStops) {
        const double x = borderBox.x + borderBox.width * stop - shortSide / 2.0;

        const CBox topLeaf{x, borderBox.y - longSide * 0.58, shortSide, longSide};
        const CBox bottomLeaf{x, borderBox.y + borderBox.height - longSide * 0.42, shortSide, longSide};
        drawGrowingRect(topLeaf, palette.leaf, borderBox.width * stop / perimeter);
        drawGrowingRect(bottomLeaf, palette.leaf, (borderBox.width + borderBox.height + borderBox.width * (1.0 - stop)) / perimeter);
    }

    for (const double stop : verticalStops) {
        const double y = borderBox.y + borderBox.height * stop - shortSide / 2.0;

        const CBox leftLeaf{borderBox.x - longSide * 0.58, y, longSide, shortSide};
        const CBox rightLeaf{borderBox.x + borderBox.width - longSide * 0.42, y, longSide, shortSide};
        drawGrowingRect(rightLeaf, palette.leaf, (borderBox.width + borderBox.height * stop) / perimeter);
        drawGrowingRect(leftLeaf, palette.leaf, (2.0 * borderBox.width + borderBox.height + borderBox.height * (1.0 - stop)) / perimeter);
    }

    const double budSize = std::max(3.0, thickness * 1.35);
    for (const double stop : std::array<double, 2>{0.34, 0.70}) {
        const CBox topBud{borderBox.x + borderBox.width * stop - budSize / 2.0, borderBox.y - budSize / 2.0, budSize, budSize};
        drawGrowingRect(topBud, palette.bud, borderBox.width * stop / perimeter);
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
