#include "AccentDecoration.hpp"
#include "AccentPassElement.hpp"
#include <algorithm>
#include <cairo/cairo.h>
#include <cmath>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/managers/eventLoop/EventLoopManager.hpp>
#include <hyprland/src/managers/eventLoop/EventLoopTimer.hpp>
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/render/Renderer.hpp>
using namespace Render::GL;
namespace OmaFrames::Packs::Accents {
namespace {
constexpr uint8_t ALL_EDGES = DECORATION_EDGE_BOTTOM | DECORATION_EDGE_LEFT | DECORATION_EDGE_RIGHT | DECORATION_EDGE_TOP;
constexpr const char *names[] = {"Fireflies", "Frost", "Railway"};
// Request damage just after the current render, then let the compositor's
// output scheduling pace the next frame. Waiting 33 ms *after* rendering adds
// another vblank wait and produces uneven, visibly low animation cadence.
constexpr auto NEXT_FRAME_REQUEST = std::chrono::milliseconds(1);
} // namespace
CAccentDecoration::CAccentDecoration(PHLWINDOW window, Kind kind)
    : IHyprWindowDecoration(window), m_kind(kind), m_window(window), m_growthStartedAt(std::chrono::steady_clock::now()) {
    m_lastWindowPosition = window->position(Desktop::View::IGeometric::GEOMETRIC_CURRENT);
    m_lastWindowSize = window->size(Desktop::View::IGeometric::GEOMETRIC_CURRENT);
    m_extents = {{18, 18}, {18, 18}};
    m_lastRelativeBox = CBox{0, 0, m_lastWindowSize.x, m_lastWindowSize.y}.addExtents(m_extents);
    if (g_pEventLoopManager) {
        m_animationTimer = makeShared<CEventLoopTimer>(
            std::nullopt, [](SP<CEventLoopTimer> timer, void *data) { static_cast<CAccentDecoration *>(data)->onAnimationTimer(timer); }, this);
        g_pEventLoopManager->addTimer(m_animationTimer);
    }
}
CAccentDecoration::~CAccentDecoration() {
    if (m_animationTimer && g_pEventLoopManager)
        g_pEventLoopManager->removeTimer(m_animationTimer);
    damageEntire();
}
void CAccentDecoration::onAnimationTimer(SP<CEventLoopTimer>) {
    if (!config(m_kind).enabled->value() || !validMapped(m_window))
        return;
    const auto window = m_window.lock();
    if (!window->m_workspace || (!window->m_pinned && !window->m_workspace->isVisible()))
        return;
    // Animate only the narrow perimeter, without repainting window contents.
    const auto position = window->position(Desktop::View::IGeometric::GEOMETRIC_CURRENT) + window->m_floatingOffset;
    const auto size = window->size(Desktop::View::IGeometric::GEOMETRIC_CURRENT);
    CBox box{position.x, position.y, size.x, size.y};
    if (!window->m_pinned)
        box.translate(window->m_workspace->m_renderOffset->value());
    if (box.width < 1 || box.height < 1)
        return;
    constexpr double margin = 18;
    g_pHyprRenderer->damageBox(CBox{box.x - margin, box.y - margin, box.width + 2 * margin, 2 * margin});
    g_pHyprRenderer->damageBox(CBox{box.x - margin, box.y + box.height - margin, box.width + 2 * margin, 2 * margin});
    g_pHyprRenderer->damageBox(CBox{box.x - margin, box.y, 2 * margin, box.height});
    g_pHyprRenderer->damageBox(CBox{box.x + box.width - margin, box.y, 2 * margin, box.height});
}
SDecorationPositioningInfo CAccentDecoration::getPositioningInfo() {
    const double extent = 18.0;

    SDecorationPositioningInfo info;
    info.policy = DECORATION_POSITION_ABSOLUTE;
    info.reserved = false;
    info.priority = 9980;
    info.edges = ALL_EDGES;
    info.desiredExtents = {{extent, extent}, {extent, extent}};
    return info;
}

void CAccentDecoration::onPositioningReply(const SDecorationPositioningReply &reply) {
    m_assignedGeometry = reply.assignedGeometry;
    damageEntire();
}

uint64_t CAccentDecoration::getDecorationFlags() { return DECORATION_PART_OF_MAIN_WINDOW; }

eDecorationLayer CAccentDecoration::getDecorationLayer() { return DECORATION_LAYER_OVER; }

std::string CAccentDecoration::getDisplayName() { return std::string("OmaFrames: ") + names[static_cast<int>(m_kind)]; }

void CAccentDecoration::draw(PHLMONITOR, const float &alpha) {
    if (!config(m_kind).enabled->value() || !validMapped(m_window)) {
        m_wasEnabled = false;
        return;
    }
    if (!m_wasEnabled) {
        m_growthStartedAt = std::chrono::steady_clock::now();
        m_wasEnabled = true;
    }

    const auto window = m_window.lock();
    if (!window->m_ruleApplicator->decorate().valueOrDefault())
        return;

    g_pHyprRenderer->m_renderPass.add(makeUnique<CAccentPassElement>(CAccentPassElement::SData{.decoration = this, .alpha = alpha}));
}

void CAccentDecoration::drawPass(PHLMONITOR monitor, const float &alpha) {
    const auto window = m_window.lock();
    auto &cfg = config(m_kind);
    if (!window || !monitor || !cfg.enabled->value())
        return;
    const auto workspace = window->m_workspace;
    const auto offset = workspace && !window->m_pinned ? workspace->m_renderOffset->value() : Vector2D();
    const auto position = window->position(Desktop::View::IGeometric::GEOMETRIC_CURRENT) + window->m_floatingOffset;
    const auto size = window->size(Desktop::View::IGeometric::GEOMETRIC_CURRENT);
    CBox box{position.x, position.y, size.x, size.y};
    box.translate(offset - monitor->m_position).scale(monitor->m_scale).round();
    if (box.width < 2 || box.height < 2)
        return;
    const double scale = monitor->m_scale;
    const bool motion = cfg.animationEnabled->value();
    const double time = motion ? std::chrono::duration<double>(std::chrono::steady_clock::now() - m_growthStartedAt).count() : 0;
    CHyprColor color{static_cast<uint64_t>(cfg.color->value())};
    if (cfg.themeAware->value() && !window->m_realBorderColor.m_colors.empty())
        color = window->m_realBorderColor.m_colors.front();
    const CHyprColor white{0.9F, 0.97F, 1.F, 1.F};
    const auto rect = [&](double x, double y, double w, double h, const CHyprColor &c, double opacity = 1.0, bool rounded = false) {
        g_pHyprOpenGL->renderRect(CBox{x, y, w, h}, c.modifyA(c.a * alpha * opacity), {.round = rounded ? static_cast<int>(std::min(w, h) / 2) : 0});
    };
    const double w = box.width, h = box.height, perimeter = 2 * (w + h);
    const auto point = [&](double distance) -> Vector2D {
        double d = std::fmod(distance, perimeter);
        if (d < 0)
            d += perimeter;
        if (d < w)
            return {box.x + d, box.y};
        d -= w;
        if (d < h)
            return {box.x + w, box.y + d};
        d -= h;
        if (d < w)
            return {box.x + w - d, box.y + h};
        d -= w;
        return {box.x, box.y + h - d};
    };
    if (m_kind == Kind::Fireflies) {
        const double seed = static_cast<double>(reinterpret_cast<uintptr_t>(window.get()) % 997) / 997;
        for (int i = 0; i < 12; ++i) {
            // Stable per-insect traits, sampled independently of render cadence.
            // Smooth mixed frequencies give wandering and hesitations without
            // frame-to-frame random jumps or a shared marching rhythm.
            const auto random = [&](double salt) {
                const double value = std::sin((i + 1) * 127.1 + seed * 311.7 + salt * 74.7) * 43758.5453;
                return value - std::floor(value);
            };
            const double phase = random(1) * 6.28318530718;
            const double phase2 = random(2) * 6.28318530718;
            const double direction = random(3) < 0.5 ? -1.0 : 1.0;
            const double pace = 0.55 + random(4) * 0.9;
            const double wander = 18 * std::sin(time * 0.35 + phase) + 8 * std::sin(time * 0.83 + phase2);
            const double distance = perimeter * (i + 0.15 + random(5) * 0.7) / 12 + direction * (time * cfg.speed->value() * pace + wander) * scale;
            auto pos = point(distance);
            // Drift plus glow stays within the advertised 18-pixel extent.
            pos.x += (6 * std::sin(time * (0.5 + random(6)) + phase) + 3 * std::sin(time * 1.37 + phase2)) * scale;
            pos.y += (6 * std::sin(time * (0.45 + random(7)) + phase2) + 3 * std::cos(time * 1.13 + phase)) * scale;
            const double pulse = 0.5 + 0.5 * std::sin(time * (0.9 + random(8) * 1.5) + phase + 0.6 * std::sin(time * 0.37 + phase2));
            const double glow = motion ? 0.12 + 0.88 * pulse * pulse : 0.8;
            for (int radius = 7; radius >= 2; --radius)
                rect(pos.x - radius * scale, pos.y - radius * scale, 2 * radius * scale, 2 * radius * scale, color, glow * 0.035, true);
            rect(pos.x - 1.3 * scale, pos.y - 1.3 * scale, 2.6 * scale, 2.6 * scale, white, glow, true);
        }
    } else if (m_kind == Kind::Frost) {
        // Cache small corner sprites; crystal growth scales their geometry.
        // Quantized colors avoid texture churn during border transitions.
        uint64_t key = 0;
        for (float channel : {color.r, color.g, color.b, color.a})
            key = (key << 4) | static_cast<uint64_t>(std::lround(std::clamp(channel, 0.F, 1.F) * 15));
        if (key != m_crystalKey) {
            for (int corner = 0; corner < 4; ++corner) {
                auto *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 256, 256);
                auto *cr = cairo_create(surface);
                cairo_scale(cr, 2, 2);
                cairo_translate(cr, corner % 2 ? 112 : 16, corner / 2 ? 112 : 16);
                cairo_scale(cr, corner % 2 ? -1 : 1, corner / 2 ? -1 : 1);
                cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
                for (int axis = 0; axis < 2; ++axis) {
                    cairo_save(cr);
                    if (axis)
                        cairo_rotate(cr, 1.5707963267948966);
                    cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a * 0.85);
                    cairo_set_line_width(cr, 1.3);
                    cairo_move_to(cr, 0, 0);
                    cairo_line_to(cr, 96, 0);
                    cairo_stroke(cr);
                    for (int j = 1; j <= 7; ++j) {
                        const double x = j * 11, branch = (1 - j / 8.0) * 13;
                        for (int side : {-1, 1}) {
                            cairo_set_source_rgba(cr, 0.9, 0.97, 1, 0.75);
                            cairo_set_line_width(cr, 0.8);
                            cairo_move_to(cr, x, 0);
                            cairo_line_to(cr, x + branch * 0.65, side * branch);
                            cairo_stroke(cr);
                            cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a * 0.7);
                            cairo_move_to(cr, x + branch * 0.35, side * branch * 0.55);
                            cairo_line_to(cr, x + branch * 0.8, side * branch * 0.55);
                            cairo_stroke(cr);
                        }
                    }
                    cairo_restore(cr);
                }
                cairo_surface_flush(surface);
                m_crystals[corner] = g_pHyprRenderer->createTexture(surface);
                cairo_destroy(cr);
                cairo_surface_destroy(surface);
            }
            m_crystalKey = key;
        }
        const double growth = motion ? std::clamp(time / 1.6, 0.0, 1.0) : 1;
        const double size = std::min({128 * scale, w * 0.45, h * 0.45}) * growth;
        for (int corner = 0; corner < 4; ++corner) {
            if (size < 0.1 || !m_crystals[corner])
                continue;
            const double x = box.x + (corner % 2 ? w : 0) - (corner % 2 ? 112.0 : 16.0) / 128 * size;
            const double y = box.y + (corner / 2 ? h : 0) - (corner / 2 ? 112.0 : 16.0) / 128 * size;
            CHyprOpenGLImpl::STextureRenderData data;
            data.a = alpha;
            data.allowDim = false;
            g_pHyprOpenGL->renderTexture(m_crystals[corner], CBox{x, y, size, size}, data);
        }
    } else {
        // Twin rails and sleepers follow the window's four edges.
        for (int edge = 0; edge < 4; ++edge) {
            const bool horizontal = edge % 2 == 0;
            const double length = horizontal ? w : h;
            const auto start = point(edge == 0 ? 0 : edge == 1 ? w : edge == 2 ? w + h : 2 * w + h);
            const double sign = edge >= 2 ? -1 : 1;
            for (double d = 5 * scale; d < length; d += 13 * scale) {
                const auto pos = start + (horizontal ? Vector2D{sign * d, 0.0} : Vector2D{0.0, sign * d});
                rect(pos.x - (horizontal ? 1 : 5) * scale, pos.y - (horizontal ? 5 : 1) * scale, (horizontal ? 2 : 10) * scale,
                     (horizontal ? 10 : 2) * scale, color, 0.45);
            }
            for (int side : {-1, 1}) {
                if (horizontal)
                    rect(box.x, start.y + side * 3 * scale, w, scale, white, 0.65);
                else
                    rect(start.x + side * 3 * scale, box.y, scale, h, white, 0.65);
            }
        }
        const double distance = 40 * scale + time * cfg.speed->value() * scale;
        for (int car = 2; car >= 0; --car) {
            const double d = distance - car * 19 * scale;
            const auto pos = point(d);
            const auto next = point(d + scale);
            const bool horizontal = std::abs(next.x - pos.x) > std::abs(next.y - pos.y);
            const double dx = horizontal ? (next.x > pos.x ? 1.0 : -1.0) : 0.0;
            const double dy = horizontal ? 0.0 : (next.y > pos.y ? 1.0 : -1.0);
            const auto part = [&](double along, double across, double length, double width, const CHyprColor &paint) {
                const auto center = pos + Vector2D{dx * along - dy * across, dy * along + dx * across} * scale;
                const double rw = (horizontal ? length : width) * scale, rh = (horizontal ? width : length) * scale;
                rect(center.x - rw / 2, center.y - rh / 2, rw, rh, paint, 1, true);
            };
            const CHyprColor dark{0.08F, 0.12F, 0.16F, 1.F};
            for (int axle : {-4, 4})
                for (int side : {-1, 1})
                    part(axle, side * 4.5, 3, 3, dark);
            part(0, 0, 15, 8, color);
            if (car == 0) {
                part(-4, 0, 6, 10, color); // Driver's cab.
                part(-4, 0, 3, 5, white);
                part(3, 0, 3, 5, dark);  // Chimney seen from above.
                part(7, 0, 2, 4, white); // Headlamp.
            } else {
                part(-3, 0, 3, 4, white);
                part(3, 0, 3, 4, white);
            }
        }
    }
    // A one-shot timer is rearmed only by a visible rendered window. Hidden
    // workspaces and reduced-motion mode therefore have no continuous timer.
    if (motion && (m_kind != Kind::Frost || time < 1.6) && m_animationTimer)
        m_animationTimer->updateTimeout(NEXT_FRAME_REQUEST);
}
eDecorationType CAccentDecoration::getDecorationType() { return DECORATION_CUSTOM; }

void CAccentDecoration::updateWindow(PHLWINDOW window) {
    // Clear the previous bounds first: during a new tiled window's opening
    // animation Hyprland can move and resize its neighbors several times.
    const CBox previousDamage = m_lastRelativeBox.copy().translate(m_lastWindowPosition).expand(3);
    g_pHyprRenderer->damageBox(previousDamage);

    m_lastWindowPosition = window->position(Desktop::View::IGeometric::GEOMETRIC_CURRENT);
    m_lastWindowSize = window->size(Desktop::View::IGeometric::GEOMETRIC_CURRENT);

    const double extent = 18.0;
    m_extents = {{extent, extent}, {extent, extent}};
    m_lastRelativeBox = CBox{0, 0, m_lastWindowSize.x, m_lastWindowSize.y}.addExtents(m_extents);
    damageEntire();
}

void CAccentDecoration::damageEntire() {
    // Hyprland's live window bounds include current layout/open-animation
    // geometry, workspace offsets, floating offsets, and all decorations.
    // Prefer them over our cache, which can briefly lag a newly mapped window.
    if (const auto window = m_window.lock(); window && window->m_isMapped) {
        g_pHyprRenderer->damageWindow(window);
        return;
    }

    CBox damage = m_lastRelativeBox.copy().translate(m_lastWindowPosition).expand(3);
    g_pHyprRenderer->damageBox(damage);
}
} // namespace OmaFrames::Packs::Accents
