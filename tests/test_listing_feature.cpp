#include <gtest/gtest.h>
#include "business/business_interface.h"

class BusinessInterfaceTest : public ::testing::Test {
protected:
    BusinessInterface business;
};

TEST_F(BusinessInterfaceTest, CreateListing) {
    Listing newListing{"Water Bottles", 100, 1.5};
    EXPECT_TRUE(business.createListing(newListing));
    
    std::vector<Listing> listings = business.getListings();
    EXPECT_EQ(1, listings.size());
    EXPECT_EQ("Water Bottles", listings[0].name);
    EXPECT_EQ(100, listings[0].quantity);
    EXPECT_DOUBLE_EQ(1.5, listings[0].price);
}

TEST_F(BusinessInterfaceTest, UpdateListing) {
    Listing newListing{"Water Bottles", 100, 1.5};
    business.createListing(newListing);
    
    EXPECT_TRUE(business.updateListing("Water Bottles", 80, 2.0));
    
    std::vector<Listing> listings = business.getListings();
    EXPECT_EQ(1, listings.size());
    EXPECT_EQ("Water Bottles", listings[0].name);
    EXPECT_EQ(80, listings[0].quantity);
    EXPECT_DOUBLE_EQ(2.0, listings[0].price);
}
