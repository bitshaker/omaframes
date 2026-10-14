#pragma once

// Render-pass adapter supplied by the OmaFrames Accents pack.
#include <hyprland/src/render/pass/PassElement.hpp>

#include "AccentPack.hpp"

namespace OmaFrames::Packs::Accents {
class CAccentDecoration;

class CAccentPassElement : public IPassElement {
  public:
    struct SData {
        CAccentDecoration *decoration = nullptr;
        float alpha = 1.F;
    };

    explicit CAccentPassElement(const SData &data);
    ~CAccentPassElement() override = default;

    std::vector<UP<IPassElement>> draw() override;
    bool needsLiveBlur() override;
    bool needsPrecomputeBlur() override;

    const char *passName() override { return PASS_NAME; }

    ePassElementType type() override { return EK_CUSTOM; }

  private:
    SData m_data;
};
} // namespace OmaFrames::Packs::Accents
