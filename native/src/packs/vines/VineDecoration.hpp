#pragma once

// Native decoration supplied by the OmaFrames Vines pack.
#define WLR_USE_UNSTABLE

#include <chrono>

#include <hyprland/src/helpers/signal/Signal.hpp>
#include <hyprland/src/render/decorations/IHyprWindowDecoration.hpp>

namespace OmaFrames::Packs::Vines {
class CVineDecoration : public IHyprWindowDecoration {
  public:
    explicit CVineDecoration(PHLWINDOW window);
    ~CVineDecoration() override;

    SDecorationPositioningInfo getPositioningInfo() override;
    void                       onPositioningReply(const SDecorationPositioningReply& reply) override;
    void                       draw(PHLMONITOR monitor, const float& alpha) override;
    eDecorationType            getDecorationType() override;
    void                       updateWindow(PHLWINDOW window) override;
    void                       damageEntire() override;
    uint64_t                   getDecorationFlags() override;
    eDecorationLayer           getDecorationLayer() override;
    std::string                getDisplayName() override;

  private:
    double growthProgress() const;
    bool   animationRunning() const;
    void   drawPass(PHLMONITOR monitor, const float& alpha);

    SBoxExtents                           m_extents;
    PHLWINDOWREF                          m_window;
    CBox                                  m_lastRelativeBox;
    CBox                                  m_assignedGeometry;
    Vector2D                              m_lastWindowPosition;
    Vector2D                              m_lastWindowSize;
    double                                m_lastExtent = 0;
    std::chrono::steady_clock::time_point m_growthStartedAt;
    CHyprSignalListener                   m_tickListener;

    friend class CVinePassElement;
};
}
