#include "business/business_interface.h"
#include "network/satellite_hub.h"

BusinessInterface::BusinessInterface(SatelliteHub* hub) : m_hub(hub) {}

bool BusinessInterface::createListing(const Listing& listing) {
    m_listings.push_back(listing);
    return true;
}

bool BusinessInterface::updateListing(const std::string& name, int newQuantity, double newPrice) {
    for (auto& listing : m_listings) {
        if (listing.name == name) {
            listing.quantity = newQuantity;
            listing.price = newPrice;
            return true;
        }
    }
    return false;
}

std::vector<Listing> BusinessInterface::getListings() const {
    return m_listings;
}
