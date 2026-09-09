#include "DatabaseDAO.h"
#include "Config.h"
#include <iostream>
#include <sstream>
#include <cstring>

DatabaseDAO::DatabaseDAO() : conn_(nullptr) {}

DatabaseDAO::~DatabaseDAO() {
    disconnect();
}

bool DatabaseDAO::connect() {
    conn_ = mysql_init(nullptr);
    if (!conn_) {
        std::cerr << "mysql_init failed\n";
        return false;
    }
    if (!mysql_real_connect(conn_,
                             Config::DB_HOST.c_str(),
                             Config::DB_USER.c_str(),
                             Config::DB_PASSWORD.c_str(),
                             Config::DB_NAME.c_str(),
                             Config::DB_PORT,
                             nullptr, 0)) {
        std::cerr << "DB connection failed: " << mysql_error(conn_) << "\n";
        conn_ = nullptr;
        return false;
    }
    return true;
}

void DatabaseDAO::disconnect() {
    if (conn_) {
        mysql_close(conn_);
        conn_ = nullptr;
    }
}

// ---------------- Auth ----------------

User DatabaseDAO::authenticateUser(const std::string& username, const std::string& passwordHash) {
    User result;
    if (!conn_) return result;

    char escUser[256];
    mysql_real_escape_string(conn_, escUser, username.c_str(), username.length());

    std::ostringstream q;
    q << "SELECT id, username, role, password_hash FROM users WHERE username='" << escUser << "'";

    if (mysql_query(conn_, q.str().c_str())) {
        std::cerr << "Query failed: " << mysql_error(conn_) << "\n";
        return result;
    }

    MYSQL_RES* res = mysql_store_result(conn_);
    if (!res) return result;

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row && row[3] && passwordHash == row[3]) {
        result.id = std::stoi(row[0]);
        result.username = row[1];
        result.role = roleFromString(row[2]);
    }
    mysql_free_result(res);
    return result;
}

bool DatabaseDAO::addUser(const std::string& username, const std::string& passwordHash, const std::string& role) {
    if (!conn_) return false;
    char escUser[256], escHash[256], escRole[128];
    mysql_real_escape_string(conn_, escUser, username.c_str(), username.length());
    mysql_real_escape_string(conn_, escHash, passwordHash.c_str(), passwordHash.length());
    mysql_real_escape_string(conn_, escRole, role.c_str(), role.length());

    std::ostringstream q;
    q << "INSERT INTO users (username, role, password_hash) VALUES ('"
      << escUser << "', '" << escRole << "', '" << escHash << "')";

    if (mysql_query(conn_, q.str().c_str())) {
        std::cerr << "Insert user failed: " << mysql_error(conn_) << "\n";
        return false;
    }
    return true;
}

// ---------------- Orders ----------------

int DatabaseDAO::insertOrder(int itemId, int qty, const std::string& requestedBy) {
    if (!conn_) return -1;
    char escUser[256];
    mysql_real_escape_string(conn_, escUser, requestedBy.c_str(), requestedBy.length());

    std::ostringstream q;
    q << "INSERT INTO orders (item_id, qty, requested_by, status) VALUES ("
      << itemId << ", " << qty << ", '" << escUser << "', 'PENDING')";

    if (mysql_query(conn_, q.str().c_str())) {
        std::cerr << "Insert order failed: " << mysql_error(conn_) << "\n";
        return -1;
    }
    return static_cast<int>(mysql_insert_id(conn_));
}

bool DatabaseDAO::updateOrderStatus(int orderId, OrderStatus status) {
    if (!conn_) return false;
    std::ostringstream q;
    q << "UPDATE orders SET status='" << orderStatusToString(status)
      << "' WHERE id=" << orderId;

    if (mysql_query(conn_, q.str().c_str())) {
        std::cerr << "Update order status failed: " << mysql_error(conn_) << "\n";
        return false;
    }
    return true;
}

std::vector<Order> DatabaseDAO::listOrdersByUser(const std::string& username) {
    std::vector<Order> orders;
    if (!conn_) return orders;
    char escUser[256];
    mysql_real_escape_string(conn_, escUser, username.c_str(), username.length());

    std::ostringstream q;
    q << "SELECT id, item_id, qty, requested_by, status FROM orders WHERE requested_by='"
      << escUser << "' ORDER BY id DESC";

    if (mysql_query(conn_, q.str().c_str())) {
        std::cerr << "Query failed: " << mysql_error(conn_) << "\n";
        return orders;
    }
    MYSQL_RES* res = mysql_store_result(conn_);
    if (!res) return orders;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        Order o;
        o.id = std::stoi(row[0]);
        o.itemId = std::stoi(row[1]);
        o.quantity = std::stoi(row[2]);
        o.requestedBy = row[3];
        std::string st = row[4];
        if (st == "PENDING") o.status = OrderStatus::PENDING;
        else if (st == "PROCESSING") o.status = OrderStatus::PROCESSING;
        else if (st == "FULFILLED") o.status = OrderStatus::FULFILLED;
        else o.status = OrderStatus::REJECTED;
        orders.push_back(o);
    }
    mysql_free_result(res);
    return orders;
}

std::vector<Order> DatabaseDAO::listAllOrders() {
    std::vector<Order> orders;
    if (!conn_) return orders;

    const char* q = "SELECT id, item_id, qty, requested_by, status FROM orders ORDER BY id DESC";
    if (mysql_query(conn_, q)) {
        std::cerr << "Query failed: " << mysql_error(conn_) << "\n";
        return orders;
    }
    MYSQL_RES* res = mysql_store_result(conn_);
    if (!res) return orders;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        Order o;
        o.id = std::stoi(row[0]);
        o.itemId = std::stoi(row[1]);
        o.quantity = std::stoi(row[2]);
        o.requestedBy = row[3];
        std::string st = row[4];
        if (st == "PENDING") o.status = OrderStatus::PENDING;
        else if (st == "PROCESSING") o.status = OrderStatus::PROCESSING;
        else if (st == "FULFILLED") o.status = OrderStatus::FULFILLED;
        else o.status = OrderStatus::REJECTED;
        orders.push_back(o);
    }
    mysql_free_result(res);
    return orders;
}

// ---------------- Inventory ----------------

bool DatabaseDAO::deductStockIfAvailable(int itemId, int qty) {
    if (!conn_) return false;
    // Atomic, DB-level conditional update: only succeeds if enough stock exists.
    // Combined with InnoDB row locking, this prevents overselling under concurrency.
    std::ostringstream q;
    q << "UPDATE inventory SET stock_qty = stock_qty - " << qty
      << " WHERE item_id = " << itemId << " AND stock_qty >= " << qty;

    if (mysql_query(conn_, q.str().c_str())) {
        std::cerr << "Deduct stock failed: " << mysql_error(conn_) << "\n";
        return false;
    }
    return mysql_affected_rows(conn_) > 0;
}

bool DatabaseDAO::addItem(int itemId, int initialStock) {
    if (!conn_) return false;
    std::ostringstream q;
    q << "INSERT INTO inventory (item_id, stock_qty) VALUES (" << itemId << ", " << initialStock << ")";
    if (mysql_query(conn_, q.str().c_str())) {
        std::cerr << "Add item failed: " << mysql_error(conn_) << "\n";
        return false;
    }
    return true;
}

bool DatabaseDAO::deleteItem(int itemId) {
    if (!conn_) return false;
    std::ostringstream q;
    q << "DELETE FROM inventory WHERE item_id = " << itemId;
    if (mysql_query(conn_, q.str().c_str())) {
        std::cerr << "Delete item failed: " << mysql_error(conn_) << "\n";
        return false;
    }
    return mysql_affected_rows(conn_) > 0;
}

bool DatabaseDAO::updateStock(int itemId, int newStock) {
    if (!conn_) return false;
    std::ostringstream q;
    q << "UPDATE inventory SET stock_qty = " << newStock << " WHERE item_id = " << itemId;
    if (mysql_query(conn_, q.str().c_str())) {
        std::cerr << "Update stock failed: " << mysql_error(conn_) << "\n";
        return false;
    }
    return mysql_affected_rows(conn_) > 0;
}

std::vector<InventoryItem> DatabaseDAO::listInventory() {
    std::vector<InventoryItem> items;
    if (!conn_) return items;

    const char* q = "SELECT item_id, stock_qty FROM inventory ORDER BY item_id";
    if (mysql_query(conn_, q)) {
        std::cerr << "Query failed: " << mysql_error(conn_) << "\n";
        return items;
    }
    MYSQL_RES* res = mysql_store_result(conn_);
    if (!res) return items;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        items.push_back({std::stoi(row[0]), std::stoi(row[1])});
    }
    mysql_free_result(res);
    return items;
}

int DatabaseDAO::getStock(int itemId) {
    if (!conn_) return -1;
    std::ostringstream q;
    q << "SELECT stock_qty FROM inventory WHERE item_id = " << itemId;
    if (mysql_query(conn_, q.str().c_str())) return -1;

    MYSQL_RES* res = mysql_store_result(conn_);
    if (!res) return -1;
    MYSQL_ROW row = mysql_fetch_row(res);
    int stock = row ? std::stoi(row[0]) : -1;
    mysql_free_result(res);
    return stock;
}

// ---------------- Audit ----------------

bool DatabaseDAO::logAudit(const std::string& username, const std::string& action, const std::string& result) {
    if (!conn_) return false;
    char escUser[256], escAction[512], escResult[128];
    mysql_real_escape_string(conn_, escUser, username.c_str(), username.length());
    mysql_real_escape_string(conn_, escAction, action.c_str(), action.length());
    mysql_real_escape_string(conn_, escResult, result.c_str(), result.length());

    std::ostringstream q;
    q << "INSERT INTO audit_log (username, action, result) VALUES ('"
      << escUser << "', '" << escAction << "', '" << escResult << "')";

    if (mysql_query(conn_, q.str().c_str())) {
        std::cerr << "Audit log insert failed: " << mysql_error(conn_) << "\n";
        return false;
    }
    return true;
}

std::vector<AuditEntry> DatabaseDAO::listAuditLog() {
    std::vector<AuditEntry> entries;
    if (!conn_) return entries;

    const char* q = "SELECT id, username, action, result, created_at FROM audit_log ORDER BY id DESC LIMIT 100";
    if (mysql_query(conn_, q)) {
        std::cerr << "Query failed: " << mysql_error(conn_) << "\n";
        return entries;
    }
    MYSQL_RES* res = mysql_store_result(conn_);
    if (!res) return entries;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        entries.push_back({std::stoi(row[0]), row[1], row[2], row[3], row[4] ? row[4] : ""});
    }
    mysql_free_result(res);
    return entries;
}
