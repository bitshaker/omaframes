#pragma once

// Render-pass adapter supplied by the OmaFrames Vines pack.
#include <hyprland/src/render/pass/PassElement.hpp>

#include "VinePack.hpp"

namespace OmaFrames::Packs::Vines {
class CVineDecoration;

class CVinePassElement : public IPassElement {
  public:
    struct SData {
        CVineDecoration* decoration = nullptr;
        float            alpha      = 1.F;
    };

    explicit CVinePassElement(const SData& data);
    ~CVinePassElement() override = default;

    std::vector<UP<IPassElement>> draw() override;
    bool                          needsLiveBlur() override;
    bool                          needsPrecomputeBlur() override;

    const char* passName() override {
        return PASS_NAME.data();
    }

    ePassElementType type() override {
        return EK_CUSTOM;
    }

  private:
    SData m_data;
};
}
