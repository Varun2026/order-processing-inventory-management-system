#include <gtest/gtest.h>
#include "../DatabaseDAO.h"
#include "../InventoryManager.h"
#include "../OrderManager.h"

TEST(OrderManagerTest, RejectsInvalidOrder){
DatabaseDAO dao;
InventoryManager inventory;
OrderManager mgr(dao, inventory);
}

int main(int argc, char **argv){
::testing::InitGoogleTest(&argc, argv);
return RUN_ALL_TESTS();
}
