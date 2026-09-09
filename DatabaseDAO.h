#ifndef DATABASE_DAO_H
#define DATABASE_DAO_H

#include <string>
#include <vector>
#include <mysql/mysql.h>
#include "Order.h"
#include "User.h"

struct InventoryItem {
    int itemId;
    int stockQty;
};

struct AuditEntry {
    int id;
    std::string username;
    std::string action;
    std::string result;
    std::string createdAt;
};

class DatabaseDAO {
public:
    DatabaseDAO();
    ~DatabaseDAO();

    bool connect();
    void disconnect();

    // ---- Auth ----
    // Looks up username, compares password_hash. Returns valid User on match.
    User authenticateUser(const std::string& username, const std::string& passwordHash);
    bool addUser(const std::string& username, const std::string& passwordHash, const std::string& role);

    // ---- Orders ----
    int insertOrder(int itemId, int qty, const std::string& requestedBy);
    bool updateOrderStatus(int orderId, OrderStatus status);
    std::vector<Order> listOrdersByUser(const std::string& username);
    std::vector<Order> listAllOrders();

    // ---- Inventory ----
    bool deductStockIfAvailable(int itemId, int qty); // atomic conditional UPDATE
    bool addItem(int itemId, int initialStock);
    bool deleteItem(int itemId);
    bool updateStock(int itemId, int newStock);
    std::vector<InventoryItem> listInventory();
    int getStock(int itemId);

    // ---- Audit ----
    bool logAudit(const std::string& username, const std::string& action, const std::string& result);
    std::vector<AuditEntry> listAuditLog();

private:
    MYSQL* conn_;
};

#endif // DATABASE_DAO_H
