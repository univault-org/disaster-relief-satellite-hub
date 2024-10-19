#ifndef RECIPIENT_INTERFACE_H
#define RECIPIENT_INTERFACE_H

#include <string>
#include <vector>

class SatelliteHub;

struct Preorder {
    std::string itemName;
    int quantity;
};

class RecipientInterface {
public:
    RecipientInterface() = default;
    explicit RecipientInterface(SatelliteHub* hub); 

    bool placePreorder(const Preorder& preorder);
    bool cancelPreorder(const std::string& itemName);
    std::vector<Preorder> getPreorders() const;

private:
    SatelliteHub* m_hub = nullptr;
    std::vector<Preorder> m_preorders;
};

#endif // RECIPIENT_INTERFACE_H
