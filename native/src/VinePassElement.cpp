#include "VinePassElement.hpp"

#include <hyprland/src/render/Renderer.hpp>

#include "VineDecoration.hpp"

CVinePassElement::CVinePassElement(const SData& data) : m_data(data) {}

std::vector<UP<IPassElement>> CVinePassElement::draw() {
    m_data.decoration->drawPass(g_pHyprRenderer->m_renderData.pMonitor.lock(), m_data.alpha);
    return {};
}

bool CVinePassElement::needsLiveBlur() {
    return false;
}

bool CVinePassElement::needsPrecomputeBlur() {
    return false;
}
