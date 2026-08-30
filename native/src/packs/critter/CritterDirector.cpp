#include "CritterDirector.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numbers>
#include <ranges>
#include <sstream>

#include <cairo/cairo.h>
#include <hyprland/src/desktop/state/FocusState.hpp>
#include <hyprland/src/desktop/state/WindowState.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/event/EventBus.hpp>
#include <hyprland/src/managers/eventLoop/EventLoopManager.hpp>
#include <hyprland/src/managers/eventLoop/EventLoopTimer.hpp>
#include <hyprland/src/managers/fullscreen/FullscreenController.hpp>
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/render/Renderer.hpp>
#include <hyprland/src/render/decorations/DecorationPositioner.hpp>

#include "CritterPack.hpp"
#include "CritterPassElement.hpp"

using namespace Render::GL;

namespace OmaFrames::Packs::Critter {
namespace {
constexpr auto   ACTIVE_FRAME_INTERVAL    = std::chrono::milliseconds(16);
constexpr auto   RAIL_TRANSITION_DURATION = std::chrono::milliseconds(180);
constexpr double PI                       = std::numbers::pi;

struct SPalette {
    CHyprColor body;
    CHyprColor accent;
    CHyprColor outline;
};

double clampUnit(const double value) {
    return std::clamp(value, 0.0, 1.0);
}

double smoothstep(const double value) {
    const double clamped = clampUnit(value);
    return clamped * clamped * (3.0 - 2.0 * clamped);
}

CHyprColor mixColors(const CHyprColor& from, const CHyprColor& to, const double amount) {
    return CHyprColor{static_cast<float>(from.r + (to.r - from.r) * amount), static_cast<float>(from.g + (to.g - from.g) * amount),
                      static_cast<float>(from.b + (to.b - from.b) * amount), static_cast<float>(from.a + (to.a - from.a) * amount)};
}

SPalette resolvedPalette(PHLWINDOW window) {
    if (window && config().themeAware->value() && !window->m_realBorderColor.m_colors.empty()) {
        const auto& colors = window->m_realBorderColor.m_colors;
        const auto  body   = mixColors(colors.front(), Colors::WHITE, 0.24);
        return {
            .body    = body,
            .accent  = mixColors(colors.back(), Colors::WHITE, 0.54),
            .outline = mixColors(body, Colors::BLACK, 0.58),
        };
    }

    const auto body = CHyprColor{static_cast<uint64_t>(config().bodyColor->value())};
    return {
        .body    = body,
        .accent  = CHyprColor{static_cast<uint64_t>(config().accentColor->value())},
        .outline = mixColors(body, Colors::BLACK, 0.58),
    };
}

uint64_t paletteKey(const SPalette& palette) {
    uint64_t key = 1469598103934665603ULL;
    const auto append = [&key](const CHyprColor& color) {
        for (const float channel : std::array{color.r, color.g, color.b, color.a}) {
            const auto quantized = static_cast<uint8_t>(std::lround(std::clamp(channel, 0.F, 1.F) * 15.F));
            key ^= quantized;
            key *= 1099511628211ULL;
        }
    };

    append(palette.body);
    append(palette.accent);
    append(palette.outline);
    return key;
}

void setCairoColor(cairo_t* cairo, const CHyprColor& color) {
    cairo_set_source_rgba(cairo, color.r, color.g, color.b, color.a);
}

void ellipse(cairo_t* cairo, const double x, const double y, const double radiusX, const double radiusY) {
    cairo_save(cairo);
    cairo_translate(cairo, x, y);
    cairo_scale(cairo, radiusX, radiusY);
    cairo_arc(cairo, 0, 0, 1, 0, 2.0 * PI);
    cairo_restore(cairo);
}

double edgeAngle(const CCritterDirector::EEdge edge) {
    switch (edge) {
        case CCritterDirector::EEdge::TOP: return 0;
        case CCritterDirector::EEdge::RIGHT: return PI / 2.0;
        case CCritterDirector::EEdge::BOTTOM: return PI;
        case CCritterDirector::EEdge::LEFT: return -PI / 2.0;
    }
    return 0;
}

SP<Render::ITexture> createCritterTexture(const SPalette& palette, const CCritterDirector::EEdge edge, const bool forward,
                                          const CCritterDirector::EPose pose) {
    constexpr int TEXTURE_SIZE = 160;

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, TEXTURE_SIZE, TEXTURE_SIZE);
    cairo_t*         cairo   = cairo_create(surface);

    cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cairo, 0, 0, 0, 0);
    cairo_paint(cairo);
    cairo_set_operator(cairo, CAIRO_OPERATOR_OVER);
    cairo_set_antialias(cairo, CAIRO_ANTIALIAS_BEST);

    cairo_translate(cairo, TEXTURE_SIZE / 2.0, TEXTURE_SIZE / 2.0);
    cairo_rotate(cairo, edgeAngle(edge));
    if (!forward)
        cairo_scale(cairo, -1.0, 1.0);
    cairo_translate(cairo, -TEXTURE_SIZE / 2.0, -TEXTURE_SIZE / 2.0);

    const bool   walkingA = pose == CCritterDirector::EPose::WALK_A;
    const bool   walkingB = pose == CCritterDirector::EPose::WALK_B;
    const bool   crouched = pose == CCritterDirector::EPose::CROUCH || pose == CCritterDirector::EPose::LAND;
    const bool   airborne = pose == CCritterDirector::EPose::FLIGHT;
    const double bodyY    = crouched ? 92.0 : (airborne ? 73.0 : 84.0);
    const double bodyRY   = crouched ? 16.0 : 22.0;
    const double legLift  = airborne ? -11.0 : 0.0;
    const double stride   = walkingA ? 8.0 : (walkingB ? -8.0 : 0.0);

    setCairoColor(cairo, CHyprColor{0.F, 0.F, 0.F, 0.20F});
    ellipse(cairo, 82, 117, crouched ? 50 : 44, crouched ? 8 : 6);
    cairo_fill(cairo);

    cairo_set_line_cap(cairo, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cairo, CAIRO_LINE_JOIN_ROUND);

    // Curved tail, outlined first so it stays readable over bright windows.
    cairo_move_to(cairo, 68, bodyY + 1);
    cairo_curve_to(cairo, 48, bodyY - 4, 43, bodyY + 17, 26, bodyY + 12);
    cairo_curve_to(cairo, 16, bodyY + 9, 17, bodyY - 1, 25, bodyY - 5);
    setCairoColor(cairo, palette.outline);
    cairo_set_line_width(cairo, 15);
    cairo_stroke_preserve(cairo);
    setCairoColor(cairo, palette.body);
    cairo_set_line_width(cairo, 9);
    cairo_stroke(cairo);

    const auto drawLeg = [&](const double hipX, const bool front, const bool upper) {
        const double swing = (front ? stride : -stride) * (upper ? 1.0 : -0.72);
        const double hipY  = bodyY + (upper ? 8.0 : 11.0) + legLift;
        const double kneeX = hipX + swing * 0.55;
        const double footX = hipX + swing;
        const double footY = airborne ? hipY + 4.0 : 113.0;

        cairo_move_to(cairo, hipX, hipY);
        cairo_line_to(cairo, kneeX, hipY + (airborne ? 4.0 : 9.0));
        cairo_line_to(cairo, footX, footY);
        setCairoColor(cairo, palette.outline);
        cairo_set_line_width(cairo, 8);
        cairo_stroke_preserve(cairo);
        setCairoColor(cairo, palette.body);
        cairo_set_line_width(cairo, 4);
        cairo_stroke(cairo);

        if (!airborne) {
            cairo_move_to(cairo, footX - 4, footY);
            cairo_line_to(cairo, footX + 5, footY);
            setCairoColor(cairo, palette.outline);
            cairo_set_line_width(cairo, 4);
            cairo_stroke(cairo);
        }
    };

    drawLeg(68, false, true);
    drawLeg(96, true, true);
    drawLeg(73, false, false);
    drawLeg(103, true, false);

    ellipse(cairo, 82, bodyY, crouched ? 39 : 42, bodyRY);
    setCairoColor(cairo, palette.body);
    cairo_fill_preserve(cairo);
    setCairoColor(cairo, palette.outline);
    cairo_set_line_width(cairo, 6);
    cairo_stroke(cairo);

    ellipse(cairo, 116, bodyY - 5, crouched ? 23 : 25, crouched ? 19 : 23);
    setCairoColor(cairo, palette.body);
    cairo_fill_preserve(cairo);
    setCairoColor(cairo, palette.outline);
    cairo_set_line_width(cairo, 6);
    cairo_stroke(cairo);

    ellipse(cairo, 77, bodyY - 7, 8, 6);
    ellipse(cairo, 94, bodyY + 5, 6, 5);
    setCairoColor(cairo, palette.accent.modifyA(palette.accent.a * 0.82));
    cairo_fill(cairo);

    const double eyeY = bodyY - (crouched ? 10 : 13);
    ellipse(cairo, 124, eyeY, 7.5, 7.5);
    setCairoColor(cairo, CHyprColor{0.97F, 0.98F, 0.86F, 1.F});
    cairo_fill_preserve(cairo);
    setCairoColor(cairo, palette.outline);
    cairo_set_line_width(cairo, 3);
    cairo_stroke(cairo);

    ellipse(cairo, 126.5, eyeY + (airborne ? 1.0 : 0.0), 3.2, 3.2);
    setCairoColor(cairo, palette.outline);
    cairo_fill(cairo);

    cairo_move_to(cairo, 136, bodyY - 1);
    cairo_curve_to(cairo, 141, bodyY + 1, 142, bodyY + 4, 137, bodyY + 5);
    setCairoColor(cairo, palette.outline);
    cairo_set_line_width(cairo, 3);
    cairo_stroke(cairo);

    cairo_surface_flush(surface);
    auto texture = g_pHyprRenderer->createTexture(surface);
    cairo_destroy(cairo);
    cairo_surface_destroy(surface);
    return texture;
}

double distanceSquared(const Vector2D& first, const Vector2D& second) {
    const double x = first.x - second.x;
    const double y = first.y - second.y;
    return x * x + y * y;
}

Vector2D clampActorCenter(Vector2D center, const CBox& bounds, const double size) {
    constexpr double MARGIN = 1.0;
    if (bounds.width < size + 2.0 * MARGIN || bounds.height < size + 2.0 * MARGIN)
        return center;

    center.x = std::clamp(center.x, bounds.x + size / 2.0 + MARGIN, bounds.x + bounds.width - size / 2.0 - MARGIN);
    center.y = std::clamp(center.y, bounds.y + size / 2.0 + MARGIN, bounds.y + bounds.height - size / 2.0 - MARGIN);
    return center;
}

std::string_view phaseName(const CCritterDirector::EPhase phase) {
    switch (phase) {
        case CCritterDirector::EPhase::HIDDEN: return "hidden";
        case CCritterDirector::EPhase::IDLE: return "idle";
        case CCritterDirector::EPhase::WALKING: return "walking";
        case CCritterDirector::EPhase::CROUCHING: return "crouching";
        case CCritterDirector::EPhase::AIRBORNE: return "airborne";
        case CCritterDirector::EPhase::LANDING: return "landing";
    }
    return "unknown";
}

std::string_view edgeName(const CCritterDirector::EEdge edge) {
    switch (edge) {
        case CCritterDirector::EEdge::TOP: return "top";
        case CCritterDirector::EEdge::RIGHT: return "right";
        case CCritterDirector::EEdge::BOTTOM: return "bottom";
        case CCritterDirector::EEdge::LEFT: return "left";
    }
    return "unknown";
}

std::string_view placementName(const CCritterDirector::EPlacement placement) {
    switch (placement) {
        case CCritterDirector::EPlacement::OUTWARD: return "outward";
        case CCritterDirector::EPlacement::MONITOR_BOUNDARY_INWARD: return "monitor-boundary-inward";
        case CCritterDirector::EPlacement::TILED_OCCLUSION_INWARD: return "tiled-occlusion-inward";
        case CCritterDirector::EPlacement::AIRBORNE: return "airborne";
    }
    return "unknown";
}

double placementFactor(const CCritterDirector::EPlacement placement) {
    return placement == CCritterDirector::EPlacement::OUTWARD ? 1.0 : -1.0;
}

CCritterDirector::EEdge oppositeEdge(const CCritterDirector::EEdge edge) {
    switch (edge) {
        case CCritterDirector::EEdge::TOP: return CCritterDirector::EEdge::BOTTOM;
        case CCritterDirector::EEdge::RIGHT: return CCritterDirector::EEdge::LEFT;
        case CCritterDirector::EEdge::BOTTOM: return CCritterDirector::EEdge::TOP;
        case CCritterDirector::EEdge::LEFT: return CCritterDirector::EEdge::RIGHT;
    }
    return edge;
}

std::string windowAddress(PHLWINDOW window) {
    if (!window)
        return "null";

    std::ostringstream stream;
    stream << "0x" << std::hex << reinterpret_cast<uintptr_t>(window.get());
    return stream.str();
}
}

CCritterDirector& director() {
    static CCritterDirector instance;
    return instance;
}

void CCritterDirector::start() {
    if (m_started)
        return;

    m_started = true;
    m_renderListener = Event::bus()->m_events.render.stage.listen([this](const eRenderStage stage) {
        if (stage == RENDER_POST_WINDOWS)
            queueFlightPass();
    });
    m_activeWindowListener = Event::bus()->m_events.window.active.listen([this](PHLWINDOW active, Desktop::eFocusReason) { activeWindowChanged(active); });
    m_closeWindowListener  = Event::bus()->m_events.window.close.listen([this](PHLWINDOW) { topologyChanged(); });
    m_moveWindowListener = Event::bus()->m_events.window.moveToWorkspace.listen([this](PHLWINDOW, PHLWORKSPACE) { topologyChanged(); });
    m_fullscreenListener = Event::bus()->m_events.window.fullscreen.listen([this](PHLWINDOW) { topologyChanged(); });
    m_floatingListener = Event::bus()->m_events.window.floating.listen([this](PHLWINDOW window) {
        if (window == Desktop::focusState()->window())
            activeWindowChanged(window);
        else
            topologyChanged();
    });
    m_workspaceListener  = Event::bus()->m_events.workspace.active.listen([this](PHLWORKSPACE) { topologyChanged(); });
    m_monitorLayoutListener = Event::bus()->m_events.monitor.layoutChanged.listen([this]() { topologyChanged(); });
    m_configListener = Event::bus()->m_events.config.props_refreshed.listen([this](const bool) {
        topologyChanged();
        refreshDecorations();
    });

    if (g_pEventLoopManager) {
        m_timer = makeShared<CEventLoopTimer>(
            std::nullopt, [](SP<CEventLoopTimer> timer, void* data) { static_cast<CCritterDirector*>(data)->onTimer(timer); }, this);
        g_pEventLoopManager->addTimer(m_timer);
    }

    topologyChanged();
}

void CCritterDirector::stop() {
    if (!m_started)
        return;

    const auto previous = m_lastActorBox;
    m_started           = false;

    if (m_timer && g_pEventLoopManager) {
        g_pEventLoopManager->removeTimer(m_timer);
        m_timer.reset();
    }

    m_renderListener.reset();
    m_activeWindowListener.reset();
    m_closeWindowListener.reset();
    m_moveWindowListener.reset();
    m_fullscreenListener.reset();
    m_floatingListener.reset();
    m_workspaceListener.reset();
    m_monitorLayoutListener.reset();
    m_configListener.reset();

    g_pHyprRenderer->m_renderPass.removeAllOfType(PASS_NAME.data());
    m_phase = EPhase::HIDDEN;
    m_host.reset();
    m_target.reset();
    m_flightMonitor.reset();
    m_lastActorBox.reset();
    m_lastPlacement.reset();
    m_railTransition.reset();
    m_activeWindowOverride.reset();
    m_textures.fill(nullptr);
    m_textureKey = UINT64_MAX;
    damageActorTransition(previous, std::nullopt);
}

double CCritterDirector::decorationExtent() const {
    if (!config().size)
        return 22;
    return std::ceil(static_cast<double>(config().size->value()) * 0.62);
}

std::string CCritterDirector::status(const bool json) const {
    const auto now    = Clock::now();
    const auto host   = m_host.lock();
    const auto target = m_target.lock();
    const auto actor  = actorState(now);
    const auto targets = eligibleTargets(host).size();

    std::ostringstream stream;
    if (json) {
        stream << "{\"started\":" << (m_started ? "true" : "false") << ",\"enabled\":"
               << (config().enabled->value() ? "true" : "false") << ",\"motionEnabled\":"
               << (config().motionEnabled->value() ? "true" : "false") << ",\"phase\":\"" << phaseName(m_phase)
               << "\",\"host\":\"" << windowAddress(host) << "\",\"target\":\"" << windowAddress(target)
               << "\",\"eligibleTargets\":" << targets << ",\"actorVisible\":" << (actor ? "true" : "false");
        if (actor) {
            stream << ",\"actor\":{\"x\":" << actor->box.x << ",\"y\":" << actor->box.y << ",\"size\":" << actor->box.width
                   << ",\"edge\":\"" << edgeName(actor->edge) << "\",\"placement\":\"" << placementName(actor->placement)
                   << "\",\"inward\":" << (actor->inward ? "true" : "false")
                   << ",\"transitioning\":" << (actor->transitioning ? "true" : "false") << "}";
            if (m_phase == EPhase::AIRBORNE) {
                const auto destination = railPoint(target, m_flightLanding, now, false);
                stream << ",\"destinationPlacement\":\"" << placementName(destination.placement) << "\"";
            }
        }
        stream << "}\n";
        return stream.str();
    }

    stream << "phase=" << phaseName(m_phase) << " host=" << windowAddress(host) << " target=" << windowAddress(target)
           << " eligible_targets=" << targets << " actor_visible=" << (actor ? "true" : "false");
    if (actor) {
        stream << " actor_x=" << actor->box.x << " actor_y=" << actor->box.y << " actor_size=" << actor->box.width
               << " actor_edge=" << edgeName(actor->edge) << " actor_placement=" << placementName(actor->placement)
               << " actor_inward=" << (actor->inward ? "true" : "false")
               << " actor_transitioning=" << (actor->transitioning ? "true" : "false");
        if (m_phase == EPhase::AIRBORNE) {
            const auto destination = railPoint(target, m_flightLanding, now, false);
            stream << " destination_placement=" << placementName(destination.placement);
        }
    }
    stream << '\n';
    return stream.str();
}

bool CCritterDirector::forceJump(std::string& error) {
    if (!m_started || !config().enabled->value()) {
        error = "OmaCritter is not enabled";
        return false;
    }
    if (!config().motionEnabled->value()) {
        error = "OmaCritter motion is disabled";
        return false;
    }

    const auto active = Desktop::focusState()->window();
    if (!eligible(active)) {
        error = "the active window is not an eligible host";
        return false;
    }
    if (eligibleTargets(active).empty()) {
        error = "the active window has no eligible target on its workspace and monitor";
        return false;
    }

    const auto now      = Clock::now();
    const auto previous = m_lastActorBox;
    m_host              = active;
    m_target.reset();
    m_flightMonitor.reset();
    m_phase             = EPhase::IDLE;
    m_phaseStarted      = now;
    m_phaseDeadline     = now;
    m_nextJump          = now;
    m_lastTick          = now;
    m_direction         = randomValue() % 2U == 0 ? -1 : 1;
    m_perimeterPosition = windowRailBox(active).width * 0.68;
    beginCrouch(now);

    if (m_phase != EPhase::CROUCHING) {
        error = "OmaCritter could not select a target";
        return false;
    }

    m_railTransition.reset();
    const auto current = actorState(now);
    damageActorTransition(previous, current ? std::optional{current->box} : std::nullopt);
    m_lastActorBox  = current ? std::optional{current->box} : std::nullopt;
    m_lastPlacement = current ? std::optional{current->placement} : std::nullopt;
    armTimer(now);
    return true;
}

bool CCritterDirector::eligible(PHLWINDOW window) const {
    if (!window || !window->m_isMapped || window->isHidden() || !window->m_workspace || !window->m_workspace->isVisible() || window->m_pinned)
        return false;
    if (!window->m_ruleApplicator->decorate().valueOrDefault())
        return false;

    if (config().hideOnFullscreen->value() &&
        Fullscreen::controller()->getFullscreenModes(window->m_workspace).internal == Fullscreen::FSMODE_FULLSCREEN)
        return false;

    const auto size        = window->size(Desktop::View::IGeometric::GEOMETRIC_CURRENT);
    const auto minimumSize = static_cast<double>(config().size->value()) * 2.2;
    return size.x >= minimumSize && size.y >= minimumSize;
}

PHLWINDOW CCritterDirector::chooseHost() const {
    if (const auto active = Desktop::focusState()->window(); eligible(active))
        return active;

    for (const auto& window : Desktop::windowState()->windows()) {
        if (eligible(window))
            return window;
    }

    return nullptr;
}

std::vector<PHLWINDOW> CCritterDirector::eligibleTargets(PHLWINDOW host) const {
    std::vector<PHLWINDOW> targets;
    if (!eligible(host))
        return targets;

    for (const auto& window : Desktop::windowState()->windows()) {
        if (window == host || !eligible(window) || window->m_workspace != host->m_workspace || window->m_monitor != host->m_monitor)
            continue;
        targets.emplace_back(window);
    }

    const CBox hostBox    = windowRailBox(host);
    const auto hostCenter = Vector2D{hostBox.x + hostBox.width / 2.0, hostBox.y + hostBox.height / 2.0};
    std::ranges::sort(targets, [&](const auto& first, const auto& second) {
        const CBox firstBox     = windowRailBox(first);
        const CBox secondBox    = windowRailBox(second);
        const auto firstCenter  = Vector2D{firstBox.x + firstBox.width / 2.0, firstBox.y + firstBox.height / 2.0};
        const auto secondCenter = Vector2D{secondBox.x + secondBox.width / 2.0, secondBox.y + secondBox.height / 2.0};
        return distanceSquared(hostCenter, firstCenter) < distanceSquared(hostCenter, secondCenter);
    });
    return targets;
}

PHLWINDOW CCritterDirector::chooseTarget(PHLWINDOW host) {
    auto targets = eligibleTargets(host);
    if (targets.empty())
        return nullptr;

    const size_t nearbyCount = std::min<size_t>(3, targets.size());
    return targets[static_cast<size_t>(randomValue() % nearbyCount)];
}

CBox CCritterDirector::windowRailBox(PHLWINDOW window) const {
    if (!window)
        return {};

    Vector2D position = window->position(Desktop::View::IGeometric::GEOMETRIC_CURRENT) + window->m_floatingOffset;
    if (window->m_workspace && !window->m_pinned)
        position += window->m_workspace->m_renderOffset->value();
    const auto size = window->size(Desktop::View::IGeometric::GEOMETRIC_CURRENT);
    return {position.x, position.y, size.x, size.y};
}

CBox CCritterDirector::monitorBox(PHLWINDOW window) const {
    if (!window)
        return {};

    const auto monitor = window->m_monitor.lock();
    if (!monitor)
        return {};
    return {monitor->m_position.x, monitor->m_position.y, monitor->m_size.x, monitor->m_size.y};
}

bool CCritterDirector::tiledActiveWindowOccludes(PHLWINDOW host, const CBox& outwardActorBox) const {
    const auto active = m_activeWindowOverride ? m_activeWindowOverride.lock() : Desktop::focusState()->window();
    if (!host || host->m_isFloating || !active || active == host || active->m_isFloating || active->m_pinned || !active->m_isMapped || active->isHidden() ||
        !active->m_workspace || !active->m_workspace->isVisible())
        return false;
    if (active->m_workspace != host->m_workspace || active->m_monitor != host->m_monitor)
        return false;

    return windowRailBox(active).overlaps(outwardActorBox);
}

double CCritterDirector::perimeterLength(const CBox& box) const {
    return std::max(1.0, 2.0 * (box.width + box.height));
}

CCritterDirector::SRailPoint CCritterDirector::railPoint(PHLWINDOW window, double position, const Clock::time_point now,
                                                        const bool applyTransition) const {
    const auto   box       = windowRailBox(window);
    const double perimeter = perimeterLength(box);
    position               = std::fmod(position, perimeter);
    if (position < 0)
        position += perimeter;

    SLanding landing;
    if (position < box.width) {
        landing = {.edge = EEdge::TOP, .fraction = position / std::max(1.0, box.width)};
    } else if (position < box.width + box.height) {
        landing = {.edge = EEdge::RIGHT, .fraction = (position - box.width) / std::max(1.0, box.height)};
    } else if (position < 2.0 * box.width + box.height) {
        const double along = position - box.width - box.height;
        landing            = {.edge = EEdge::BOTTOM, .fraction = 1.0 - along / std::max(1.0, box.width)};
    } else {
        const double along = position - 2.0 * box.width - box.height;
        landing            = {.edge = EEdge::LEFT, .fraction = 1.0 - along / std::max(1.0, box.height)};
    }
    return railPoint(window, landing, now, applyTransition);
}

CCritterDirector::SRailPoint CCritterDirector::railPoint(PHLWINDOW window, const SLanding& landing, const Clock::time_point now,
                                                        const bool applyTransition) const {
    const CBox   box      = windowRailBox(window);
    const CBox   bounds   = monitorBox(window);
    const double fraction = clampUnit(landing.fraction);
    const double size     = static_cast<double>(config().size->value());
    const double offset   = size * 0.42;
    const double required = offset + size / 2.0 + 1.0;
    SRailPoint  point;
    point.edge     = landing.edge;
    point.fraction = fraction;

    Vector2D outwardNormal;
    double   outwardClearance = 0;

    switch (landing.edge) {
        case EEdge::TOP:
            point.contact = {box.x + box.width * fraction, box.y};
            outwardNormal    = {0.0, -1.0};
            outwardClearance = box.y - bounds.y;
            break;
        case EEdge::RIGHT:
            point.contact = {box.x + box.width, box.y + box.height * fraction};
            outwardNormal    = {1.0, 0.0};
            outwardClearance = bounds.x + bounds.width - box.x - box.width;
            break;
        case EEdge::BOTTOM:
            point.contact = {box.x + box.width * fraction, box.y + box.height};
            outwardNormal    = {0.0, 1.0};
            outwardClearance = bounds.y + bounds.height - box.y - box.height;
            break;
        case EEdge::LEFT:
            point.contact = {box.x, box.y + box.height * fraction};
            outwardNormal    = {-1.0, 0.0};
            outwardClearance = box.x - bounds.x;
            break;
    }

    const Vector2D outwardCenter = point.contact + outwardNormal * offset;
    const CBox     outwardActorBox{outwardCenter.x - size / 2.0, outwardCenter.y - size / 2.0, size, size};

    // Output clipping takes precedence. At internal rails, only the active
    // tiled window's animated box can turn the actor inward; the exact actor
    // box keeps unrelated portions of broadly adjacent windows out of it.
    if (bounds.width > 0 && bounds.height > 0 && outwardClearance < required)
        point.placement = EPlacement::MONITOR_BOUNDARY_INWARD;
    else if (tiledActiveWindowOccludes(window, outwardActorBox))
        point.placement = EPlacement::TILED_OCCLUSION_INWARD;

    double factor = placementFactor(point.placement);
    if (applyTransition && m_railTransition && m_railTransition->host.lock() == window && m_railTransition->edge == point.edge &&
        m_railTransition->destination == point.placement) {
        const auto elapsed = std::chrono::duration<double, std::milli>(now - m_railTransition->started).count();
        const auto raw     = clampUnit(elapsed / static_cast<double>(RAIL_TRANSITION_DURATION.count()));
        factor = m_railTransition->fromFactor + (m_railTransition->toFactor - m_railTransition->fromFactor) * smoothstep(raw);
        point.transitioning = raw < 1.0;
    }

    point.inward = factor <= 0.0;
    point.center = point.contact + outwardNormal * (offset * factor);

    // Keep even the transparent texture box inside the output. The correction
    // is only a few logical pixels after the rail has flipped inward, and it
    // prevents pose extremities from being clipped on truly flush windows.
    point.center = clampActorCenter(point.center, bounds, size);
    return point;
}

CCritterDirector::SLanding CCritterDirector::closestLanding(PHLWINDOW target, const Vector2D& source) const {
    const CBox   targetBox = windowRailBox(target);
    const double size       = static_cast<double>(config().size->value());
    const double horizontal = std::clamp(size * 0.8 / std::max(1.0, targetBox.width), 0.08, 0.42);
    const double vertical   = std::clamp(size * 0.8 / std::max(1.0, targetBox.height), 0.08, 0.42);

    const double xFraction = std::clamp((source.x - targetBox.x) / std::max(1.0, targetBox.width), horizontal, 1.0 - horizontal);
    const double yFraction = std::clamp((source.y - targetBox.y) / std::max(1.0, targetBox.height), vertical, 1.0 - vertical);
    const std::array candidates{
        SLanding{.edge = EEdge::TOP, .fraction = xFraction},
        SLanding{.edge = EEdge::RIGHT, .fraction = yFraction},
        SLanding{.edge = EEdge::BOTTOM, .fraction = xFraction},
        SLanding{.edge = EEdge::LEFT, .fraction = yFraction},
    };

    const auto now = Clock::now();
    return *std::ranges::min_element(candidates, [&](const auto& first, const auto& second) {
        return distanceSquared(railPoint(target, first, now, false).center, source) <
            distanceSquared(railPoint(target, second, now, false).center, source);
    });
}

double CCritterDirector::perimeterPosition(const CBox& box, const SLanding& landing) const {
    const double fraction = clampUnit(landing.fraction);
    switch (landing.edge) {
        case EEdge::TOP: return box.width * fraction;
        case EEdge::RIGHT: return box.width + box.height * fraction;
        case EEdge::BOTTOM: return box.width + box.height + box.width * (1.0 - fraction);
        case EEdge::LEFT: return 2.0 * box.width + box.height + box.height * (1.0 - fraction);
    }
    return 0;
}

std::optional<CCritterDirector::SActorState> CCritterDirector::actorState(const Clock::time_point now) const {
    const auto host = m_host.lock();
    if (!m_started || !config().enabled->value() || !eligible(host) || m_phase == EPhase::HIDDEN)
        return std::nullopt;

    const double size = static_cast<double>(config().size->value());
    if (m_phase == EPhase::AIRBORNE) {
        const auto target = m_target.lock();
        if (!eligible(target) || target->m_workspace != host->m_workspace || target->m_monitor != host->m_monitor)
            return std::nullopt;

        const auto duration = std::max(1.0, static_cast<double>(m_flightDuration.count()));
        const auto elapsed  = std::chrono::duration<double, std::milli>(now - m_phaseStarted).count();
        const double raw    = clampUnit(elapsed / duration);
        const double travel = smoothstep(raw);
        const auto   destination = railPoint(target, m_flightLanding, now, false);
        const auto   finish      = destination.center;
        const double distance = std::sqrt(distanceSquared(m_flightStart, finish));
        const double arc      = std::clamp(distance * 0.24, 48.0, 180.0) * std::sin(PI * raw);
        auto center = m_flightStart + (finish - m_flightStart) * travel + Vector2D{0.0, -arc};
        center      = clampActorCenter(center, monitorBox(host), size);

        return SActorState{
            .box           = {center.x - size / 2.0, center.y - size / 2.0, size, size},
            .edge          = EEdge::TOP,
            .poseEdge      = EEdge::TOP,
            .pose          = EPose::FLIGHT,
            .forward       = finish.x >= m_flightStart.x,
            .inward        = false,
            .transitioning = false,
            .placement     = EPlacement::AIRBORNE,
            .paletteWindow = host,
        };
    }

    const auto point = railPoint(host, m_perimeterPosition, now);
    EPose      pose  = EPose::IDLE;
    if (m_phase == EPhase::WALKING) {
        const auto frame = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_phaseStarted).count() / 150;
        pose             = frame % 2 == 0 ? EPose::WALK_A : EPose::WALK_B;
    } else if (m_phase == EPhase::CROUCHING) {
        pose = EPose::CROUCH;
    } else if (m_phase == EPhase::LANDING) {
        pose = EPose::LAND;
    }

    return SActorState{
        .box           = {point.center.x - size / 2.0, point.center.y - size / 2.0, size, size},
        .edge          = point.edge,
        .poseEdge      = point.inward ? oppositeEdge(point.edge) : point.edge,
        .pose          = pose,
        .forward       = point.inward ? m_direction < 0 : m_direction > 0,
        .inward        = point.inward,
        .transitioning = point.transitioning,
        .placement     = point.placement,
        .paletteWindow = host,
    };
}

void CCritterDirector::ensureHost(const Clock::time_point now) {
    auto host = m_host.lock();
    if (eligible(host) && config().motionEnabled->value())
        return;

    if (eligible(host) && !config().motionEnabled->value()) {
        const auto active = Desktop::focusState()->window();
        if (!eligible(active) || active == host)
            return;
        host = active;
    } else {
        host = chooseHost();
    }

    if (!host) {
        m_phase = EPhase::HIDDEN;
        m_host.reset();
        m_target.reset();
        return;
    }

    m_host              = host;
    m_target.reset();
    m_railTransition.reset();
    m_phase             = EPhase::IDLE;
    m_phaseStarted      = now;
    m_phaseDeadline     = now + randomizedIdleDuration();
    m_nextJump          = now + randomizedJumpInterval();
    m_direction         = randomValue() % 2U == 0 ? -1 : 1;
    m_perimeterPosition = windowRailBox(host).width * 0.68;
}

void CCritterDirector::enterIdle(const Clock::time_point now) {
    m_phase         = EPhase::IDLE;
    m_phaseStarted  = now;
    m_phaseDeadline = now + randomizedIdleDuration();
    m_target.reset();
}

void CCritterDirector::enterWalking(const Clock::time_point now) {
    m_phase         = EPhase::WALKING;
    m_phaseStarted  = now;
    m_phaseDeadline = now + randomizedWalkDuration();
    if (randomValue() % 5U == 0)
        m_direction *= -1;
}

void CCritterDirector::beginCrouch(const Clock::time_point now) {
    const auto host = m_host.lock();
    m_target       = chooseTarget(host);
    if (!m_target) {
        m_nextJump = now + randomizedJumpInterval();
        enterIdle(now);
        return;
    }

    m_phase         = EPhase::CROUCHING;
    m_phaseStarted  = now;
    m_phaseDeadline = now + std::chrono::milliseconds(300);
}

void CCritterDirector::beginFlight(const Clock::time_point now) {
    const auto host   = m_host.lock();
    const auto target = m_target.lock();
    if (!eligible(host) || !eligible(target) || host->m_workspace != target->m_workspace || host->m_monitor != target->m_monitor) {
        enterIdle(now);
        return;
    }

    const auto current = actorState(now);
    if (!current) {
        enterIdle(now);
        return;
    }

    m_flightStart   = {current->box.x + current->box.width / 2.0, current->box.y + current->box.height / 2.0};
    m_flightLanding = closestLanding(target, m_flightStart);
    m_flightMonitor = host->m_monitor;

    const auto targetCenter = railPoint(target, m_flightLanding, now, false).center;
    const auto distance     = std::sqrt(distanceSquared(m_flightStart, targetCenter));
    m_flightDuration = std::chrono::milliseconds(static_cast<int64_t>(std::clamp(420.0 + distance * 0.46, 540.0, 1200.0)));
    m_phase          = EPhase::AIRBORNE;
    m_phaseStarted   = now;
    m_phaseDeadline  = now + m_flightDuration;
}

void CCritterDirector::finishFlight(const Clock::time_point now) {
    const auto target = m_target.lock();
    if (!eligible(target)) {
        enterIdle(now);
        return;
    }

    m_host              = target;
    m_perimeterPosition = perimeterPosition(windowRailBox(target), m_flightLanding);
    m_target.reset();
    m_flightMonitor.reset();
    m_railTransition.reset();
    m_phase         = EPhase::LANDING;
    m_phaseStarted  = now;
    m_phaseDeadline = now + std::chrono::milliseconds(240);
    m_nextJump      = now + randomizedJumpInterval();
}

void CCritterDirector::update(const Clock::time_point now) {
    if (m_railTransition && now - m_railTransition->started >= RAIL_TRANSITION_DURATION)
        m_railTransition.reset();

    ensureHost(now);
    if (m_phase == EPhase::HIDDEN)
        return;

    const auto host = m_host.lock();
    if (!eligible(host)) {
        m_phase = EPhase::HIDDEN;
        return;
    }

    if (!config().motionEnabled->value()) {
        m_phase             = EPhase::IDLE;
        m_phaseStarted      = now;
        m_phaseDeadline     = Clock::time_point::max();
        m_perimeterPosition = windowRailBox(host).width * 0.72;
        return;
    }

    if (m_phase == EPhase::IDLE && m_phaseDeadline == Clock::time_point::max()) {
        enterIdle(now);
        m_nextJump = now + randomizedJumpInterval();
    }

    const double elapsed = m_lastTick.time_since_epoch().count() == 0 ? 0.0 : std::clamp(std::chrono::duration<double>(now - m_lastTick).count(), 0.0, 0.1);
    m_lastTick           = now;

    switch (m_phase) {
        case EPhase::HIDDEN: break;
        case EPhase::IDLE:
            if (now >= m_phaseDeadline) {
                if (now >= m_nextJump && !eligibleTargets(host).empty())
                    beginCrouch(now);
                else
                    enterWalking(now);
            }
            break;
        case EPhase::WALKING:
            m_perimeterPosition += static_cast<double>(config().walkSpeed->value()) * elapsed * static_cast<double>(m_direction);
            if (now >= m_phaseDeadline) {
                if (now >= m_nextJump && !eligibleTargets(host).empty())
                    beginCrouch(now);
                else
                    enterIdle(now);
            }
            break;
        case EPhase::CROUCHING:
            if (now >= m_phaseDeadline)
                beginFlight(now);
            break;
        case EPhase::AIRBORNE:
            if (const auto target = m_target.lock();
                !eligible(target) || target->m_workspace != host->m_workspace || target->m_monitor != host->m_monitor) {
                enterIdle(now);
                m_nextJump = now + randomizedJumpInterval();
            } else if (now >= m_phaseDeadline) {
                finishFlight(now);
            }
            break;
        case EPhase::LANDING:
            if (now >= m_phaseDeadline)
                enterIdle(now);
            break;
    }
}

void CCritterDirector::topologyChanged() {
    if (!m_started)
        return;

    const auto now      = Clock::now();
    const auto previous = m_lastActorBox;
    update(now);
    updateRailTransition(now);
    const auto current = actorState(now);
    damageActorTransition(previous, current ? std::optional{current->box} : std::nullopt);
    m_lastActorBox  = current ? std::optional{current->box} : std::nullopt;
    m_lastPlacement = current ? std::optional{current->placement} : std::nullopt;
    armTimer(now);
}

void CCritterDirector::updateRailTransition(const Clock::time_point now) {
    const auto host = m_host.lock();
    if (!m_lastActorBox || !m_lastPlacement || !eligible(host) || m_phase == EPhase::AIRBORNE)
        return;

    const auto destination = railPoint(host, m_perimeterPosition, now, false);
    if (m_railTransition && m_railTransition->host.lock() == host && m_railTransition->edge == destination.edge &&
        m_railTransition->destination == destination.placement)
        return;

    const bool occlusionChanged = (*m_lastPlacement == EPlacement::TILED_OCCLUSION_INWARD ||
                                   destination.placement == EPlacement::TILED_OCCLUSION_INWARD) &&
        *m_lastPlacement != destination.placement;
    if (!occlusionChanged)
        return;

    Vector2D normal;
    switch (destination.edge) {
        case EEdge::TOP: normal = {0.0, -1.0}; break;
        case EEdge::RIGHT: normal = {1.0, 0.0}; break;
        case EEdge::BOTTOM: normal = {0.0, 1.0}; break;
        case EEdge::LEFT: normal = {-1.0, 0.0}; break;
    }

    const auto previousCenter = Vector2D{m_lastActorBox->x + m_lastActorBox->width / 2.0, m_lastActorBox->y + m_lastActorBox->height / 2.0};
    const auto offset         = std::max(1.0, static_cast<double>(config().size->value()) * 0.42);
    const auto delta          = previousCenter - destination.contact;
    const auto fromFactor     = std::clamp((delta.x * normal.x + delta.y * normal.y) / offset, -1.0, 1.0);
    m_railTransition = SRailTransition{
        .host        = host,
        .edge        = destination.edge,
        .destination = destination.placement,
        .fromFactor  = fromFactor,
        .toFactor    = placementFactor(destination.placement),
        .started     = now,
    };
}

void CCritterDirector::activeWindowChanged(PHLWINDOW active) {
    if (!m_started)
        return;

    const auto now      = Clock::now();
    const auto previous = m_lastActorBox;
    update(now);

    // The focus event can arrive just before FocusState exposes the new
    // window. Classify this destination against the event payload, then let
    // normal state reads take over on the following frame.
    m_activeWindowOverride = active;
    updateRailTransition(now);
    m_activeWindowOverride.reset();

    const auto current = actorState(now);
    damageActorTransition(previous, current ? std::optional{current->box} : std::nullopt);
    m_lastActorBox  = current ? std::optional{current->box} : std::nullopt;
    m_lastPlacement = current ? std::optional{current->placement} : std::nullopt;
    armTimer(now);
}

void CCritterDirector::windowUpdated(PHLWINDOW window) {
    if (!m_started || (!m_host || (m_host.lock() != window && m_target.lock() != window && Desktop::focusState()->window() != window)))
        return;
    damageLastAndCurrent();
}

void CCritterDirector::onTimer(SP<CEventLoopTimer>) {
    if (!m_started)
        return;

    const auto now      = Clock::now();
    const auto previous = m_lastActorBox;
    update(now);
    updateRailTransition(now);
    const auto current = actorState(now);
    damageActorTransition(previous, current ? std::optional{current->box} : std::nullopt);
    m_lastActorBox  = current ? std::optional{current->box} : std::nullopt;
    m_lastPlacement = current ? std::optional{current->placement} : std::nullopt;
    armTimer(now);
}

void CCritterDirector::armTimer(const Clock::time_point now) {
    if (!m_timer)
        return;
    if (!m_started || !config().enabled->value() || m_phase == EPhase::HIDDEN) {
        m_timer->updateTimeout(std::nullopt);
        return;
    }

    if (m_railTransition) {
        m_timer->updateTimeout(ACTIVE_FRAME_INTERVAL);
        return;
    }
    if (!config().motionEnabled->value()) {
        m_timer->updateTimeout(std::nullopt);
        return;
    }

    if (m_phase == EPhase::IDLE) {
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(m_phaseDeadline - now);
        m_timer->updateTimeout(std::max(std::chrono::milliseconds(1), remaining));
        return;
    }

    m_timer->updateTimeout(ACTIVE_FRAME_INTERVAL);
}

void CCritterDirector::damageActorTransition(const std::optional<CBox>& before, const std::optional<CBox>& after) {
    if (before)
        g_pHyprRenderer->damageBox(before->copy().expand(3));
    if (after)
        g_pHyprRenderer->damageBox(after->copy().expand(3));
}

void CCritterDirector::damageLastAndCurrent() {
    const auto now = Clock::now();
    updateRailTransition(now);
    const auto current = actorState(now);
    damageActorTransition(m_lastActorBox, current ? std::optional{current->box} : std::nullopt);
    m_lastActorBox  = current ? std::optional{current->box} : std::nullopt;
    m_lastPlacement = current ? std::optional{current->placement} : std::nullopt;
}

void CCritterDirector::queueWindowPass(PHLWINDOW window, PHLMONITOR monitor, const float alpha) {
    if (!m_started || !config().enabled->value() || m_phase == EPhase::HIDDEN || m_phase == EPhase::AIRBORNE || m_host.lock() != window || !monitor)
        return;
    if (window->m_monitor != monitor)
        return;

    g_pHyprRenderer->m_renderPass.add(
        makeUnique<CCritterPassElement>(CCritterPassElement::SData{.director = this, .window = window, .alpha = alpha, .flight = false}));
}

void CCritterDirector::queueFlightPass() {
    if (!m_started || !config().enabled->value() || m_phase != EPhase::AIRBORNE)
        return;

    const auto monitor = g_pHyprRenderer->m_renderData.pMonitor.lock();
    if (!monitor || m_flightMonitor.lock() != monitor)
        return;

    g_pHyprRenderer->m_renderPass.add(
        makeUnique<CCritterPassElement>(CCritterPassElement::SData{.director = this, .window = {}, .alpha = 1.F, .flight = true}));
}

void CCritterDirector::drawPass(PHLMONITOR monitor, const PHLWINDOWREF window, const bool flight, float alpha) {
    if (!monitor || !m_started || !config().enabled->value())
        return;
    if (flight != (m_phase == EPhase::AIRBORNE))
        return;
    if (!flight && window.lock() != m_host.lock())
        return;

    const auto state = actorState(Clock::now());
    if (!state)
        return;

    const auto paletteWindow = state->paletteWindow.lock();
    if (!paletteWindow || paletteWindow->m_monitor != monitor)
        return;

    if (flight && paletteWindow->m_workspace)
        alpha *= paletteWindow->m_workspace->m_alpha->value();

    const auto texture = textureFor(paletteWindow, state->poseEdge, state->forward, state->pose);
    if (!texture)
        return;

    CBox renderBox = state->box.copy().translate(-monitor->m_position).scale(monitor->m_scale).round();
    CHyprOpenGLImpl::STextureRenderData textureData;
    textureData.a        = alpha;
    textureData.allowDim = false;
    g_pHyprOpenGL->renderTexture(texture, renderBox, textureData);
}

void CCritterDirector::refreshDecorations() {
    for (const auto& window : Desktop::windowState()->windows()) {
        for (const auto& decoration : window->m_windowDecorations) {
            if (decoration->getDisplayName() == DISPLAY_NAME)
                g_pDecorationPositioner->repositionDeco(decoration.get());
        }
    }
}

std::chrono::milliseconds CCritterDirector::randomizedIdleDuration() {
    return std::chrono::milliseconds(900 + static_cast<int64_t>(randomValue() % 1900U));
}

std::chrono::milliseconds CCritterDirector::randomizedWalkDuration() {
    return std::chrono::milliseconds(2200 + static_cast<int64_t>(randomValue() % 3100U));
}

std::chrono::milliseconds CCritterDirector::randomizedJumpInterval() {
    const auto base   = static_cast<double>(config().jumpIntervalMs->value());
    const auto jitter = 0.76 + static_cast<double>(randomValue() % 4900U) / 10000.0;
    return std::chrono::milliseconds(static_cast<int64_t>(base * jitter));
}

uint64_t CCritterDirector::randomValue() {
    return m_random();
}

SP<Render::ITexture> CCritterDirector::textureFor(PHLWINDOW paletteWindow, const EEdge edge, const bool forward, const EPose pose) {
    const auto palette = resolvedPalette(paletteWindow);
    const auto key     = paletteKey(palette);
    if (key != m_textureKey) {
        m_textures.fill(nullptr);
        m_textureKey = key;
    }

    const size_t index = static_cast<size_t>(pose) * 8U + static_cast<size_t>(edge) * 2U + static_cast<size_t>(forward ? 1U : 0U);
    if (!m_textures[index])
        m_textures[index] = createCritterTexture(palette, edge, forward, pose);
    return m_textures[index];
}
}
