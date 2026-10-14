#include "AccentPassElement.hpp"

#include <hyprland/src/render/Renderer.hpp>

#include "AccentDecoration.hpp"

namespace OmaFrames::Packs::Accents {
CAccentPassElement::CAccentPassElement(const SData &data) : m_data(data) {}

std::vector<UP<IPassElement>> CAccentPassElement::draw() {
    m_data.decoration->drawPass(g_pHyprRenderer->m_renderData.pMonitor.lock(), m_data.alpha);
    return {};
}

bool CAccentPassElement::needsLiveBlur() { return false; }

bool CAccentPassElement::needsPrecomputeBlur() { return false; }
} // namespace OmaFrames::Packs::Accents
