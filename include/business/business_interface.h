#ifndef BUSINESS_INTERFACE_H
#define BUSINESS_INTERFACE_H

#include <string>
#include <vector>

class SatelliteHub;

struct Listing {
    std::string name;
    int quantity;
    double price;
};

class BusinessInterface {
public:
    BusinessInterface() = default;
    explicit BusinessInterface(SatelliteHub* hub);  

    bool createListing(const Listing& listing);
    bool updateListing(const std::string& name, int newQuantity, double newPrice);
    std::vector<Listing> getListings() const;

private:
    SatelliteHub* m_hub = nullptr;
    std::vector<Listing> m_listings;
};

#endif // BUSINESS_INTERFACE_H
