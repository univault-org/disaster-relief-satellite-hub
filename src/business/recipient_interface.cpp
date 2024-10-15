#include "business/recipient_interface.h"
#include "network/satellite_hub.h"

RecipientInterface::RecipientInterface(SatelliteHub* hub) : m_hub(hub) {}

bool RecipientInterface::placePreorder(const Preorder& preorder) {
    m_preorders.push_back(preorder);
    return true;
}

bool RecipientInterface::cancelPreorder(const std::string& itemName) {
    auto it = std::remove_if(m_preorders.begin(), m_preorders.end(),
                             [&itemName](const Preorder& p) { return p.itemName == itemName; });
    if (it != m_preorders.end()) {
        m_preorders.erase(it, m_preorders.end());
        return true;
    }
    return false;
}

std::vector<Preorder> RecipientInterface::getPreorders() const {
    return m_preorders;
}
