#pragma once

#include <hyprland/src/render/pass/PassElement.hpp>

#include "CritterPack.hpp"

namespace OmaFrames::Packs::Critter {
class CCritterDirector;

class CCritterPassElement : public IPassElement {
  public:
    struct SData {
        CCritterDirector* director = nullptr;
        PHLWINDOWREF      window;
        float             alpha  = 1.F;
        bool              flight = false;
    };

    explicit CCritterPassElement(const SData& data);
    ~CCritterPassElement() override = default;

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
