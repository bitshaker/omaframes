#pragma once

#include <hyprland/src/render/decorations/IHyprWindowDecoration.hpp>

namespace OmaFrames::Packs::Critter {
class CCritterDecoration : public IHyprWindowDecoration {
  public:
    explicit CCritterDecoration(PHLWINDOW window);
    ~CCritterDecoration() override;

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
    PHLWINDOWREF m_window;
    double       m_lastExtent = 0;
};
}
