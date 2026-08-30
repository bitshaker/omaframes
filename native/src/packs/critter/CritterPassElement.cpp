#include "CritterPassElement.hpp"

#include <hyprland/src/render/Renderer.hpp>

#include "CritterDirector.hpp"

namespace OmaFrames::Packs::Critter {
CCritterPassElement::CCritterPassElement(const SData& data) : m_data(data) {}

std::vector<UP<IPassElement>> CCritterPassElement::draw() {
    if (m_data.director)
        m_data.director->drawPass(g_pHyprRenderer->m_renderData.pMonitor.lock(), m_data.window, m_data.flight, m_data.alpha);
    return {};
}

bool CCritterPassElement::needsLiveBlur() {
    return false;
}

bool CCritterPassElement::needsPrecomputeBlur() {
    return false;
}
}
