#include "MenuCLI.h"
#include "RBACManager.h"
#include <iostream>
#include <limits>
#include <thread>
#include <vector>

MenuCLI::MenuCLI(DatabaseDAO& dao, InventoryManager& inventory, ReportGenerator& reports)
    : dao_(dao), inventory_(inventory), reports_(reports), auth_(dao) {}

int MenuCLI::readInt(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
        try {
            return std::stoi(line);
        } catch (...) {
            std::cout << "Please enter a valid number.\n";
        }
    }
}

void MenuCLI::showBanner() {
    std::cout << "\n==================================================\n";
    std::cout << "   Order Processing & Inventory Management System\n";
    std::cout << "   CPOC_Team20\n";
    std::cout << "==================================================\n";
}

void MenuCLI::run() {
    showBanner();
    currentUser_ = auth_.login(3);
    if (!currentUser_.isValid()) {
        std::cout << "Exiting.\n";
        return;
    }

    switch (currentUser_.role) {
        case Role::ORDER_MANAGER:   orderManagerMenu();   break;
        case Role::DB_ADMIN:        dbAdminMenu();        break;
        case Role::BACKEND_MANAGER: backendManagerMenu(); break;
        default:
            std::cout << "Unrecognized role. Contact your DB Admin.\n";
    }

    std::cout << "\nGoodbye, " << currentUser_.username << ".\n";
}

// ---------------- Order Manager menu ----------------

void MenuCLI::orderManagerMenu() {
    while (true) {
        std::cout << "\n--- Order Manager Menu (" << currentUser_.username << ") ---\n";
        std::cout << "1. Place Order\n";
        std::cout << "2. View My Orders\n";
        std::cout << "3. View All Orders\n";
        std::cout << "4. Run Flash Sale Demo (concurrent orders)\n";
        std::cout << "5. Logout\n";
        int choice = readInt("Select an option: ");

        switch (choice) {
            case 1: placeOrder(); break;
            case 2: viewMyOrders(); break;
            case 3: viewAllOrders(); break;
            case 4: runFlashSaleDemo(); break;
            case 5: return;
            default: std::cout << "Invalid option.\n";
        }
    }
}

// ---------------- DB Admin menu ----------------

void MenuCLI::dbAdminMenu() {
    while (true) {
        std::cout << "\n--- DB Admin Menu (" << currentUser_.username << ") ---\n";
        std::cout << "1. View Inventory\n";
        std::cout << "2. Add Inventory Item\n";
        std::cout << "3. Update Inventory Stock\n";
        std::cout << "4. Delete Inventory Item\n";
        std::cout << "5. Add New User\n";
        std::cout << "6. View Audit Log\n";
        std::cout << "7. View All Orders\n";
        std::cout << "8. Logout\n";
        int choice = readInt("Select an option: ");

        switch (choice) {
            case 1: viewInventory(); break;
            case 2: addInventoryItem(); break;
            case 3: updateInventoryStock(); break;
            case 4: deleteInventoryItem(); break;
            case 5: addNewUser(); break;
            case 6: viewAuditLog(); break;
            case 7: viewAllOrders(); break;
            case 8: return;
            default: std::cout << "Invalid option.\n";
        }
    }
}

// ---------------- Backend Manager menu ----------------

void MenuCLI::backendManagerMenu() {
    while (true) {
        std::cout << "\n--- Backend Manager Menu (" << currentUser_.username << ") ---\n";
        std::cout << "1. Generate Orders Report (CSV)\n";
        std::cout << "2. View Audit Log\n";
        std::cout << "3. View All Orders\n";
        std::cout << "4. Logout\n";
        int choice = readInt("Select an option: ");

        switch (choice) {
            case 1: generateOrdersReport(); break;
            case 2: viewAuditLog(); break;
            case 3: viewAllOrders(); break;
            case 4: return;
            default: std::cout << "Invalid option.\n";
        }
    }
}

// ---------------- Shared actions ----------------

void MenuCLI::placeOrder() {
    if (!RBACManager::isAuthorized(currentUser_.role, Permission::ADD_ORDER)) {
        std::cout << "You are not authorized to place orders.\n";
        return;
    }
    int itemId = readInt("Item ID: ");
    int qty = readInt("Quantity: ");

    OrderManager om(dao_, inventory_, false);
    om.processOrder(itemId, qty, currentUser_.username);
}

void MenuCLI::viewMyOrders() {
    auto orders = dao_.listOrdersByUser(currentUser_.username);
    std::cout << "\nOrderID | ItemID | Qty | Status\n";
    std::cout << "--------------------------------\n";
    for (const auto& o : orders) {
        std::cout << o.id << " | " << o.itemId << " | " << o.quantity
                  << " | " << orderStatusToString(o.status) << "\n";
    }
    if (orders.empty()) std::cout << "(no orders yet)\n";
}

void MenuCLI::viewAllOrders() {
    auto orders = dao_.listAllOrders();
    std::cout << "\nOrderID | ItemID | Qty | RequestedBy | Status\n";
    std::cout << "-----------------------------------------------\n";
    for (const auto& o : orders) {
        std::cout << o.id << " | " << o.itemId << " | " << o.quantity
                  << " | " << o.requestedBy << " | " << orderStatusToString(o.status) << "\n";
    }
    if (orders.empty()) std::cout << "(no orders yet)\n";
}

void MenuCLI::runFlashSaleDemo() {
    std::cout << "\n=== FLASH SALE DEMO ===\n";
    std::cout << "This simulates many customers hitting the same item at once,\n";
    std::cout << "so you can see thread interleaving and stock protection live.\n\n";

    int itemId = readInt("Item ID to flash-sale: ");
    int currentStock = dao_.getStock(itemId);
    if (currentStock < 0) {
        std::cout << "Item " << itemId << " not found in inventory.\n";
        return;
    }
    std::cout << "Current stock for item " << itemId << ": " << currentStock << "\n";

    int numOrders = readInt("How many concurrent orders to simulate: ");
    int qtyEach = readInt("Quantity per order: ");

    std::cout << "\nLaunching " << numOrders << " concurrent order threads...\n\n";

    // Each thread gets its OWN database connection — sharing one MYSQL*
    // across threads corrupts memory. Every thread opens a short-lived
    // connection to the same order_system database instead.
    std::vector<std::thread> threads;
    for (int i = 0; i < numOrders; ++i) {
        std::string customerName = currentUser_.username + "_sim" + std::to_string(i + 1);
        threads.emplace_back([itemId, qtyEach, customerName]() {
            mysql_thread_init();
            DatabaseDAO threadDao;
            if (!threadDao.connect()) {
                std::cerr << "[" << customerName << "] Could not open DB connection for this thread.\n";
                mysql_thread_end();
                return;
            }
            InventoryManager threadInventory(threadDao);
            OrderManager om(threadDao, threadInventory, /*verboseThreadDemo=*/true);
            om.processOrder(itemId, qtyEach, customerName);
            threadDao.disconnect();
            mysql_thread_end();
        });
    }
    for (auto& t : threads) t.join();

    int finalStock = dao_.getStock(itemId);
    std::cout << "\n=== Flash Sale Complete ===\n";
    std::cout << "Stock before: " << currentStock << "  ->  Stock after: " << finalStock << "\n";
    std::cout << "Check 'View All Orders' to see which were FULFILLED vs REJECTED.\n";
}

void MenuCLI::viewInventory() {
    auto items = inventory_.listAll();
    std::cout << "\nItemID | StockQty\n";
    std::cout << "-----------------\n";
    for (const auto& item : items) {
        std::cout << item.itemId << " | " << item.stockQty << "\n";
    }
    if (items.empty()) std::cout << "(no items)\n";
}

void MenuCLI::addInventoryItem() {
    if (!RBACManager::isAuthorized(currentUser_.role, Permission::UPDATE_INVENTORY)) {
        std::cout << "You are not authorized to modify inventory.\n";
        return;
    }
    int itemId = readInt("New Item ID: ");
    int stock = readInt("Initial Stock Quantity: ");
    bool ok = inventory_.addItem(itemId, stock);
    dao_.logAudit(currentUser_.username, "ADD_ITEM #" + std::to_string(itemId), ok ? "SUCCESS" : "FAILED");
    std::cout << (ok ? "Item added.\n" : "Failed to add item (may already exist).\n");
}

void MenuCLI::updateInventoryStock() {
    if (!RBACManager::isAuthorized(currentUser_.role, Permission::UPDATE_INVENTORY)) {
        std::cout << "You are not authorized to modify inventory.\n";
        return;
    }
    int itemId = readInt("Item ID: ");
    int newStock = readInt("New Stock Quantity: ");
    bool ok = inventory_.updateStock(itemId, newStock);
    dao_.logAudit(currentUser_.username, "UPDATE_STOCK #" + std::to_string(itemId), ok ? "SUCCESS" : "FAILED");
    std::cout << (ok ? "Stock updated.\n" : "Failed to update stock (item may not exist).\n");
}

void MenuCLI::deleteInventoryItem() {
    if (!RBACManager::isAuthorized(currentUser_.role, Permission::UPDATE_INVENTORY)) {
        std::cout << "You are not authorized to modify inventory.\n";
        return;
    }
    int itemId = readInt("Item ID to delete: ");
    std::cout << "Warning: this will fail if orders reference this item.\n";
    bool ok = inventory_.deleteItem(itemId);
    dao_.logAudit(currentUser_.username, "DELETE_ITEM #" + std::to_string(itemId), ok ? "SUCCESS" : "FAILED");
    std::cout << (ok ? "Item deleted.\n" : "Failed to delete item (it may not exist, or orders reference it).\n");
}

void MenuCLI::addNewUser() {
    if (!RBACManager::isAuthorized(currentUser_.role, Permission::MANAGE_USERS)) {
        std::cout << "You are not authorized to manage users.\n";
        return;
    }
    std::cout << "Username: ";
    std::string username;
    std::getline(std::cin, username);

    std::cout << "Role (ORDER_MANAGER / DB_ADMIN / BACKEND_MANAGER): ";
    std::string role;
    std::getline(std::cin, role);

    std::cout << "Temporary Password: ";
    std::string password;
    std::getline(std::cin, password);

    std::string hashed = AuthManager::hashPassword(password);
    bool ok = dao_.addUser(username, hashed, role);
    dao_.logAudit(currentUser_.username, "ADD_USER " + username, ok ? "SUCCESS" : "FAILED");
    std::cout << (ok ? "User created.\n" : "Failed to create user (username may already exist).\n");
}

void MenuCLI::viewAuditLog() {
    auto entries = dao_.listAuditLog();
    std::cout << "\nID | Username | Action | Result | CreatedAt\n";
    std::cout << "------------------------------------------------\n";
    for (const auto& e : entries) {
        std::cout << e.id << " | " << e.username << " | " << e.action
                  << " | " << e.result << " | " << e.createdAt << "\n";
    }
    if (entries.empty()) std::cout << "(no audit entries)\n";
}

void MenuCLI::generateOrdersReport() {
    std::string path = "orders_report.csv";
    reports_.generateOrdersCSV(path);
    dao_.logAudit(currentUser_.username, "GENERATE_REPORT", "SUCCESS");
}
