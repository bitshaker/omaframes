#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include <hyprland/src/desktop/DesktopTypes.hpp>
#include <hyprland/src/helpers/math/Math.hpp>
#include <hyprland/src/helpers/signal/Signal.hpp>
#include <hyprland/src/render/Texture.hpp>

class CEventLoopTimer;

namespace OmaFrames::Packs::Critter {
class CCritterDirector {
  public:
    enum class EPhase : uint8_t {
        HIDDEN,
        IDLE,
        WALKING,
        CROUCHING,
        AIRBORNE,
        LANDING,
    };

    enum class EEdge : uint8_t {
        TOP,
        RIGHT,
        BOTTOM,
        LEFT,
    };

    enum class EPose : uint8_t {
        IDLE,
        WALK_A,
        WALK_B,
        CROUCH,
        FLIGHT,
        LAND,
        COUNT,
    };

    enum class EPlacement : uint8_t {
        OUTWARD,
        MONITOR_BOUNDARY_INWARD,
        TILED_OCCLUSION_INWARD,
        AIRBORNE,
    };

    void start();
    void stop();
    void topologyChanged();
    void windowUpdated(PHLWINDOW window);
    void queueWindowPass(PHLWINDOW window, PHLMONITOR monitor, float alpha);
    void drawPass(PHLMONITOR monitor, PHLWINDOWREF window, bool flight, float alpha);
    std::string status(bool json) const;
    bool        forceJump(std::string& error);

    double decorationExtent() const;

  private:
    using Clock = std::chrono::steady_clock;

    struct SRailPoint {
        Vector2D contact;
        Vector2D center;
        EEdge      edge          = EEdge::TOP;
        double     fraction      = 0;
        EPlacement placement     = EPlacement::OUTWARD;
        bool       inward       = false;
        bool       transitioning = false;
    };

    struct SLanding {
        EEdge  edge     = EEdge::TOP;
        double fraction = 0.5;
    };

    struct SActorState {
        CBox         box;
        EEdge        edge          = EEdge::TOP;
        EEdge        poseEdge      = EEdge::TOP;
        EPose        pose          = EPose::IDLE;
        bool         forward       = true;
        bool         inward        = false;
        bool         transitioning = false;
        EPlacement   placement     = EPlacement::OUTWARD;
        PHLWINDOWREF paletteWindow;
    };

    struct SRailTransition {
        PHLWINDOWREF      host;
        EEdge             edge        = EEdge::TOP;
        EPlacement        destination = EPlacement::OUTWARD;
        double            fromFactor  = 1.0;
        double            toFactor    = 1.0;
        Clock::time_point started{};
    };

    bool                         eligible(PHLWINDOW window) const;
    PHLWINDOW                    chooseHost() const;
    PHLWINDOW                    chooseTarget(PHLWINDOW host);
    std::vector<PHLWINDOW>       eligibleTargets(PHLWINDOW host) const;
    CBox                         windowRailBox(PHLWINDOW window) const;
    CBox                         monitorBox(PHLWINDOW window) const;
    bool                         tiledActiveWindowOccludes(PHLWINDOW host, const CBox& outwardActorBox) const;
    double                       perimeterLength(const CBox& box) const;
    SRailPoint                   railPoint(PHLWINDOW window, double perimeterPosition, Clock::time_point now, bool applyTransition = true) const;
    SRailPoint                   railPoint(PHLWINDOW window, const SLanding& landing, Clock::time_point now, bool applyTransition = true) const;
    SLanding                     closestLanding(PHLWINDOW target, const Vector2D& source) const;
    double                       perimeterPosition(const CBox& box, const SLanding& landing) const;
    std::optional<SActorState>   actorState(Clock::time_point now) const;
    void                         ensureHost(Clock::time_point now);
    void                         enterIdle(Clock::time_point now);
    void                         enterWalking(Clock::time_point now);
    void                         beginCrouch(Clock::time_point now);
    void                         beginFlight(Clock::time_point now);
    void                         finishFlight(Clock::time_point now);
    void                         update(Clock::time_point now);
    void                         updateRailTransition(Clock::time_point now);
    void                         activeWindowChanged(PHLWINDOW active);
    void                         onTimer(SP<CEventLoopTimer> timer);
    void                         armTimer(Clock::time_point now);
    void                         damageActorTransition(const std::optional<CBox>& before, const std::optional<CBox>& after);
    void                         damageLastAndCurrent();
    void                         queueFlightPass();
    void                         refreshDecorations();
    std::chrono::milliseconds    randomizedIdleDuration();
    std::chrono::milliseconds    randomizedWalkDuration();
    std::chrono::milliseconds    randomizedJumpInterval();
    SP<Render::ITexture>         textureFor(PHLWINDOW paletteWindow, EEdge edge, bool forward, EPose pose);
    uint64_t                     randomValue();

    bool                         m_started = false;
    EPhase                       m_phase   = EPhase::HIDDEN;
    PHLWINDOWREF                 m_host;
    PHLWINDOWREF                 m_target;
    PHLMONITORREF                m_flightMonitor;
    double                       m_perimeterPosition = 0;
    int                          m_direction         = 1;
    Vector2D                     m_flightStart;
    SLanding                     m_flightLanding;
    std::chrono::milliseconds    m_flightDuration{700};
    Clock::time_point            m_phaseStarted{};
    Clock::time_point            m_phaseDeadline{};
    Clock::time_point            m_nextJump{};
    Clock::time_point            m_lastTick{};
    std::optional<CBox>            m_lastActorBox;
    std::optional<EPlacement>      m_lastPlacement;
    std::optional<SRailTransition> m_railTransition;
    PHLWINDOWREF                   m_activeWindowOverride;
    SP<CEventLoopTimer>          m_timer;
    std::mt19937_64              m_random{std::random_device{}()};

    static constexpr size_t      TEXTURE_COUNT = static_cast<size_t>(EPose::COUNT) * 4U * 2U;
    std::array<SP<Render::ITexture>, TEXTURE_COUNT> m_textures;
    uint64_t                                      m_textureKey = UINT64_MAX;

    CHyprSignalListener m_renderListener;
    CHyprSignalListener m_activeWindowListener;
    CHyprSignalListener m_closeWindowListener;
    CHyprSignalListener m_moveWindowListener;
    CHyprSignalListener m_fullscreenListener;
    CHyprSignalListener m_floatingListener;
    CHyprSignalListener m_workspaceListener;
    CHyprSignalListener m_monitorLayoutListener;
    CHyprSignalListener m_configListener;
};

CCritterDirector& director();
}
