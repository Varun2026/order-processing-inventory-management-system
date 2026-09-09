#ifndef INVENTORY_MANAGER_H
#define INVENTORY_MANAGER_H

#include <mutex>
#include "DatabaseDAO.h"

// Thin, thread-safe wrapper around inventory operations.
// The real concurrency safety comes from the DB-level conditional
// UPDATE in DatabaseDAO::deductStockIfAvailable (InnoDB row locking),
// this mutex adds an extra in-process guard for local operations.
class InventoryManager {
public:
    explicit InventoryManager(DatabaseDAO& dao) : dao_(dao) {}

    bool tryDeduct(int itemId, int qty) {
        std::lock_guard<std::mutex> lock(mtx_);
        return dao_.deductStockIfAvailable(itemId, qty);
    }

    bool addItem(int itemId, int stock) {
        std::lock_guard<std::mutex> lock(mtx_);
        return dao_.addItem(itemId, stock);
    }

    bool deleteItem(int itemId) {
        std::lock_guard<std::mutex> lock(mtx_);
        return dao_.deleteItem(itemId);
    }

    bool updateStock(int itemId, int newStock) {
        std::lock_guard<std::mutex> lock(mtx_);
        return dao_.updateStock(itemId, newStock);
    }

    std::vector<InventoryItem> listAll() {
        std::lock_guard<std::mutex> lock(mtx_);
        return dao_.listInventory();
    }

private:
    DatabaseDAO& dao_;
    std::mutex mtx_;
};

#endif // INVENTORY_MANAGER_H
