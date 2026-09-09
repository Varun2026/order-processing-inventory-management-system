#ifndef MENU_CLI_H
#define MENU_CLI_H

#include "DatabaseDAO.h"
#include "InventoryManager.h"
#include "OrderManager.h"
#include "AuthManager.h"
#include "ReportGenerator.h"
#include "User.h"

class MenuCLI {
public:
    MenuCLI(DatabaseDAO& dao, InventoryManager& inventory, ReportGenerator& reports);

    // Main entry point: shows login, then the role-appropriate menu loop.
    void run();

private:
    DatabaseDAO& dao_;
    InventoryManager& inventory_;
    ReportGenerator& reports_;
    AuthManager auth_;
    User currentUser_;

    void showBanner();
    void orderManagerMenu();
    void dbAdminMenu();
    void backendManagerMenu();

    // Shared actions
    void placeOrder();
    void viewMyOrders();
    void viewAllOrders();
    void runFlashSaleDemo();
    void viewInventory();
    void addInventoryItem();
    void updateInventoryStock();
    void deleteInventoryItem();
    void addNewUser();
    void viewAuditLog();
    void generateOrdersReport();

    int readInt(const std::string& prompt);
};

#endif // MENU_CLI_H
