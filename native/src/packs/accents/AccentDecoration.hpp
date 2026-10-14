#pragma once

// Native decoration shared by Fireflies, Frost and Railway.
#define WLR_USE_UNSTABLE

#include "AccentPack.hpp"
#include <array>
#include <chrono>
#include <cstdint>

#include <hyprland/src/render/Texture.hpp>
#include <hyprland/src/render/decorations/IHyprWindowDecoration.hpp>

class CEventLoopTimer;

namespace OmaFrames::Packs::Accents {
class CAccentDecoration : public IHyprWindowDecoration {
  public:
    explicit CAccentDecoration(PHLWINDOW window, Kind kind);
    ~CAccentDecoration() override;

    SDecorationPositioningInfo getPositioningInfo() override;
    void onPositioningReply(const SDecorationPositioningReply &reply) override;
    void draw(PHLMONITOR monitor, const float &alpha) override;
    eDecorationType getDecorationType() override;
    void updateWindow(PHLWINDOW window) override;
    void damageEntire() override;
    uint64_t getDecorationFlags() override;
    eDecorationLayer getDecorationLayer() override;
    std::string getDisplayName() override;

  private:
    void onAnimationTimer(SP<CEventLoopTimer> timer);
    void drawPass(PHLMONITOR monitor, const float &alpha);
    Kind m_kind;
    bool m_wasEnabled = false;
    std::array<SP<Render::ITexture>, 4> m_crystals;
    uint64_t m_crystalKey = UINT64_MAX;
    SBoxExtents m_extents;
    PHLWINDOWREF m_window;
    CBox m_lastRelativeBox;
    CBox m_assignedGeometry;
    Vector2D m_lastWindowPosition;
    Vector2D m_lastWindowSize;

    std::chrono::steady_clock::time_point m_growthStartedAt;
    SP<CEventLoopTimer> m_animationTimer;

    friend class CAccentPassElement;
};
} // namespace OmaFrames::Packs::Accents
