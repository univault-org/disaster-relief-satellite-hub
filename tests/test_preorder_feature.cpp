#include <gtest/gtest.h>
#include "business/recipient_interface.h"

class RecipientInterfaceTest : public ::testing::Test {
protected:
    RecipientInterface recipient;
};

TEST_F(RecipientInterfaceTest, PlacePreorder) {
    Preorder newPreorder{"Water Bottles", 10};
    EXPECT_TRUE(recipient.placePreorder(newPreorder));
    
    std::vector<Preorder> preorders = recipient.getPreorders();
    EXPECT_EQ(1, preorders.size());
    EXPECT_EQ("Water Bottles", preorders[0].itemName);
    EXPECT_EQ(10, preorders[0].quantity);
}

TEST_F(RecipientInterfaceTest, CancelPreorder) {
    Preorder newPreorder{"Water Bottles", 10};
    recipient.placePreorder(newPreorder);
    
    EXPECT_TRUE(recipient.cancelPreorder("Water Bottles"));
    
    std::vector<Preorder> preorders = recipient.getPreorders();
    EXPECT_EQ(0, preorders.size());
}
