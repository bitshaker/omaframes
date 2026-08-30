#include "VineDecoration.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <numbers>

#include <cairo/cairo.h>
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

enum class ELeafDirection : uint8_t {
    UP,
    RIGHT,
    DOWN,
    LEFT,
};

struct SLeafPlacement {
    double stop;
    double delay;
    int    tilt;
};

struct SLeafLayout {
    std::array<SLeafPlacement, 5> placements;
    size_t                        count = 0;
};

uint64_t nextRandom(uint64_t& state) {
    state += 0x9e3779b97f4a7c15ULL;
    uint64_t value = state;
    value          = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9ULL;
    value          = (value ^ (value >> 27U)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31U);
}

double randomUnit(uint64_t& state) {
    return static_cast<double>(nextRandom(state) >> 11U) * (1.0 / 9007199254740992.0);
}

SLeafLayout leafPlacements(const double logicalLength, uint64_t seed) {
    const int nominalCount = static_cast<int>(std::lround(logicalLength / 230.0));
    const int countJitter  = static_cast<int>(nextRandom(seed) % 3U) - 1;
    const int count        = std::clamp(nominalCount + countJitter, 1, 5);
    const double cell      = 0.78 / static_cast<double>(count);

    SLeafLayout layout{};
    layout.count = static_cast<size_t>(count);
    for (int index = 0; index < count; ++index) {
        const double positionJitter = (randomUnit(seed) - 0.5) * cell * 0.56;
        const double stop           = std::clamp(0.11 + cell * (static_cast<double>(index) + 0.5) + positionJitter, 0.08, 0.92);
        const double delay          = randomUnit(seed) * 0.018;
        const int    tilt           = static_cast<int>(nextRandom(seed) % 3U) - 1;
        layout.placements[static_cast<size_t>(index)] = {.stop = stop, .delay = delay, .tilt = tilt};
    }

    return layout;
}

double leafAngle(const ELeafDirection direction, const int tilt) {
    constexpr double TILT_RADIANS = 24.0 * std::numbers::pi / 180.0;

    double angle = 0;
    switch (direction) {
        case ELeafDirection::UP: angle = -std::numbers::pi / 2.0; break;
        case ELeafDirection::RIGHT: angle = 0; break;
        case ELeafDirection::DOWN: angle = std::numbers::pi / 2.0; break;
        case ELeafDirection::LEFT: angle = std::numbers::pi; break;
    }

    return angle + static_cast<double>(tilt) * TILT_RADIANS;
}

double configuredExtent() {
    return std::max<Config::INTEGER>(config().extent->value(), config().stemThickness->value());
}

double smoothstep(const double value) {
    const double clamped = std::clamp(value, 0.0, 1.0);
    return clamped * clamped * (3.0 - 2.0 * clamped);
}

double sproutProgress(const double growth, const double threshold) {
    const double start  = std::clamp(threshold, 0.0, 0.99);
    const double window = std::min(SPROUT_WINDOW, 1.0 - start);
    return smoothstep((growth - start) / window);
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

uint64_t leafTextureKey(const SPalette& palette) {
    uint64_t key = 1469598103934665603ULL;

    // Four bits per channel keep live border transitions smooth without
    // rebuilding twelve small textures for every imperceptible color step.
    const auto appendColor = [&key](const CHyprColor& color) {
        for (const float channel : std::array{color.r, color.g, color.b, color.a}) {
            const auto quantized = static_cast<uint8_t>(std::lround(std::clamp(channel, 0.F, 1.F) * 15.F));
            key ^= quantized;
            key *= 1099511628211ULL;
        }
    };

    appendColor(palette.leaf);
    appendColor(palette.bud);
    return key;
}

void setCairoColor(cairo_t* cairo, const CHyprColor& color) {
    cairo_set_source_rgba(cairo, color.r, color.g, color.b, color.a);
}

SP<Render::ITexture> createLeafTexture(const SPalette& palette, const ELeafDirection direction, const int tilt) {
    constexpr int TEXTURE_SIZE = 96;

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, TEXTURE_SIZE, TEXTURE_SIZE);
    cairo_t*         cairo   = cairo_create(surface);

    cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cairo, 0, 0, 0, 0);
    cairo_paint(cairo);
    cairo_set_operator(cairo, CAIRO_OPERATOR_OVER);
    cairo_set_antialias(cairo, CAIRO_ANTIALIAS_BEST);

    const double angle = leafAngle(direction, tilt);

    cairo_translate(cairo, TEXTURE_SIZE / 2.0, TEXTURE_SIZE / 2.0);
    cairo_rotate(cairo, angle);
    cairo_translate(cairo, -TEXTURE_SIZE / 2.0, -TEXTURE_SIZE / 2.0);

    const auto petioleColor = mixColors(palette.leaf, CHyprColor{0.F, 0.F, 0.F, static_cast<float>(palette.leaf.a)}, 0.26);
    setCairoColor(cairo, petioleColor);
    cairo_set_line_width(cairo, 4.0);
    cairo_set_line_cap(cairo, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cairo, 1, 48);
    cairo_line_to(cairo, 13, 48);
    cairo_stroke(cairo);

    // A heart-shaped climbing-vine silhouette, authored in a square so it can
    // be rotated cleanly for each window edge without raster assets. Broad
    // curves and one dominant tip remain recognizable at decoration scale.
    cairo_move_to(cairo, 6, 48);
    cairo_curve_to(cairo, 18, 44, 15, 29, 29, 21);
    cairo_curve_to(cairo, 45, 12, 67, 24, 92, 46);
    cairo_curve_to(cairo, 74, 65, 53, 81, 33, 77);
    cairo_curve_to(cairo, 17, 73, 19, 56, 6, 48);
    cairo_close_path(cairo);

    setCairoColor(cairo, palette.leaf);
    cairo_fill_preserve(cairo);
    setCairoColor(cairo, mixColors(palette.leaf, CHyprColor{0.F, 0.F, 0.F, static_cast<float>(palette.leaf.a)}, 0.34));
    cairo_set_line_width(cairo, 3.0);
    cairo_set_line_join(cairo, CAIRO_LINE_JOIN_ROUND);
    cairo_stroke(cairo);

    const auto veinColor = mixColors(palette.leaf, palette.bud, 0.30);
    setCairoColor(cairo, veinColor.modifyA(veinColor.a * 0.68));
    cairo_set_line_width(cairo, 2.6);
    cairo_set_line_cap(cairo, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cairo, CAIRO_LINE_JOIN_ROUND);

    cairo_move_to(cairo, 8, 48);
    cairo_curve_to(cairo, 35, 47, 62, 47, 87, 46);
    cairo_move_to(cairo, 34, 47);
    cairo_line_to(cairo, 24, 29);
    cairo_move_to(cairo, 54, 47);
    cairo_line_to(cairo, 45, 22);
    cairo_move_to(cairo, 35, 49);
    cairo_line_to(cairo, 25, 68);
    cairo_move_to(cairo, 55, 48);
    cairo_line_to(cairo, 45, 73);
    cairo_stroke(cairo);

    cairo_surface_flush(surface);
    auto texture = g_pHyprRenderer->createTexture(surface);
    cairo_destroy(cairo);
    cairo_surface_destroy(surface);
    return texture;
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
    m_layoutSeed         = reinterpret_cast<uintptr_t>(window.get()) ^ static_cast<uint64_t>(m_growthStartedAt.time_since_epoch().count());

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

    const double leafLength = config().leafSize->value() * monitor->m_scale;

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

    const auto drawLeaf = [&](const Vector2D& anchor, const ELeafDirection direction, const double threshold, const int tilt) {
        const double sprout = sproutProgress(growth, threshold);
        if (sprout <= 0)
            return;

        const double size         = leafLength * 1.08 * sprout;
        const double angle        = leafAngle(direction, tilt);
        const double anchorLocalX = 0.5 - 0.5 * std::cos(angle);
        const double anchorLocalY = 0.5 - 0.5 * std::sin(angle);
        const CBox   textureBox{anchor.x - anchorLocalX * size, anchor.y - anchorLocalY * size, size, size};
        const size_t textureIndex = static_cast<size_t>(direction) * 3U + static_cast<size_t>(tilt + 1);
        const auto&  texture      = m_leafTextures[textureIndex];
        if (texture) {
            CHyprOpenGLImpl::STextureRenderData textureData;
            textureData.a        = static_cast<float>(alpha * sprout);
            textureData.allowDim = false;
            g_pHyprOpenGL->renderTexture(texture, textureBox, textureData);
        } else {
            const auto fallbackColor = palette.leaf.modifyA(palette.leaf.a * alpha * sprout);
            g_pHyprOpenGL->renderRect(textureBox, fallbackColor, {.round = static_cast<int>(size / 2.0)});
        }
    };

    const auto textureKey = leafTextureKey(palette);
    if (textureKey != m_leafTextureKey) {
        for (size_t index = 0; index < m_leafTextures.size(); ++index) {
            const auto direction = static_cast<ELeafDirection>(index / 3U);
            const auto tilt      = static_cast<int>(index % 3U) - 1;
            m_leafTextures[index] = createLeafTexture(palette, direction, tilt);
        }
        m_leafTextureKey = textureKey;
    }

    const auto topLeaves    = leafPlacements(borderBox.width / monitor->m_scale, m_layoutSeed ^ 0x4a2f17c32e91d56bULL);
    const auto rightLeaves  = leafPlacements(borderBox.height / monitor->m_scale, m_layoutSeed ^ 0xc1b508da7634ef29ULL);
    const auto bottomLeaves = leafPlacements(borderBox.width / monitor->m_scale, m_layoutSeed ^ 0x8de713b94065ac2fULL);
    const auto leftLeaves   = leafPlacements(borderBox.height / monitor->m_scale, m_layoutSeed ^ 0x36a9cf0257e184bdULL);

    for (size_t index = 0; index < topLeaves.count; ++index) {
        const auto&  leaf      = topLeaves.placements[index];
        const double x         = borderBox.x + borderBox.width * leaf.stop;
        const double threshold = borderBox.width * leaf.stop / perimeter + leaf.delay;
        drawLeaf({x, borderBox.y}, ELeafDirection::UP, threshold, leaf.tilt);
    }

    for (size_t index = 0; index < rightLeaves.count; ++index) {
        const auto&  leaf      = rightLeaves.placements[index];
        const double y         = borderBox.y + borderBox.height * leaf.stop;
        const double threshold = (borderBox.width + borderBox.height * leaf.stop) / perimeter + leaf.delay;
        drawLeaf({borderBox.x + borderBox.width, y}, ELeafDirection::RIGHT, threshold, leaf.tilt);
    }

    for (size_t index = 0; index < bottomLeaves.count; ++index) {
        const auto&  leaf      = bottomLeaves.placements[index];
        const double x         = borderBox.x + borderBox.width * leaf.stop;
        const double threshold = (borderBox.width + borderBox.height + borderBox.width * (1.0 - leaf.stop)) / perimeter + leaf.delay;
        drawLeaf({x, borderBox.y + borderBox.height}, ELeafDirection::DOWN, threshold, leaf.tilt);
    }

    for (size_t index = 0; index < leftLeaves.count; ++index) {
        const auto&  leaf      = leftLeaves.placements[index];
        const double y         = borderBox.y + borderBox.height * leaf.stop;
        const double threshold = (2.0 * borderBox.width + borderBox.height + borderBox.height * (1.0 - leaf.stop)) / perimeter + leaf.delay;
        drawLeaf({borderBox.x, y}, ELeafDirection::LEFT, threshold, leaf.tilt);
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
