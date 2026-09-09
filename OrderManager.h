#ifndef ORDER_MANAGER_H
#define ORDER_MANAGER_H

#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <mutex>
#include "DatabaseDAO.h"
#include "InventoryManager.h"
#include "Order.h"

// Handles the lifecycle of a single order: PENDING -> PROCESSING -> FULFILLED/REJECTED.
// Designed to be safely called from many threads at once (used by the
// thread pool and by the flash-sale demo).
class OrderManager {
public:
    OrderManager(DatabaseDAO& dao, InventoryManager& inventory, bool verboseThreadDemo = false)
        : dao_(dao), inventory_(inventory), verboseThreadDemo_(verboseThreadDemo) {}

    void processOrder(int itemId, int qty, const std::string& requestedBy) {
        std::ostringstream tidStream;
        tidStream << std::this_thread::get_id();
        std::string tid = tidStream.str();

        int orderId = dao_.insertOrder(itemId, qty, requestedBy);
        if (orderId == -1) {
            std::lock_guard<std::mutex> lock(coutMtx_);
            std::cout << "[Thread " << tid << "] Failed to create order for "
                      << requestedBy << " (DB error)\n";
            return;
        }

        dao_.updateOrderStatus(orderId, OrderStatus::PROCESSING);
        {
            std::lock_guard<std::mutex> lock(coutMtx_);
            std::cout << "[Thread " << tid << "] Order #" << orderId << " (" << requestedBy
                      << ", item " << itemId << ", qty " << qty << ") -> PROCESSING\n";
        }

        // Artificial delay so concurrent processing is visibly demonstrable
        // during the flash-sale demo (shows real interleaving of threads).
        if (verboseThreadDemo_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        bool ok = inventory_.tryDeduct(itemId, qty);
        OrderStatus finalStatus = ok ? OrderStatus::FULFILLED : OrderStatus::REJECTED;
        dao_.updateOrderStatus(orderId, finalStatus);
        dao_.logAudit(requestedBy, "ORDER #" + std::to_string(orderId), orderStatusToString(finalStatus));

        {
            std::lock_guard<std::mutex> lock(coutMtx_);
            std::cout << "[Thread " << tid << "] Order #" << orderId << " (" << requestedBy
                      << ", item " << itemId << ", qty " << qty << ") -> "
                      << orderStatusToString(finalStatus) << "\n";
        }
    }

private:
    DatabaseDAO& dao_;
    InventoryManager& inventory_;
    bool verboseThreadDemo_;
    inline static std::mutex coutMtx_;
};

#endif // ORDER_MANAGER_H
