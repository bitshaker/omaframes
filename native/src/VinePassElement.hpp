#pragma once

#include <hyprland/src/render/pass/PassElement.hpp>

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
        return "CVinePassElement";
    }

    ePassElementType type() override {
        return EK_CUSTOM;
    }

  private:
    SData m_data;
};
