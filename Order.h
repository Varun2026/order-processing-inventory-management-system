#ifndef ORDER_H
#define ORDER_H

#include <string>
#include "OrderStatus.h"

struct Order {
    int id = -1;
    int itemId = -1;
    int quantity = 0;
    std::string requestedBy;
    OrderStatus status = OrderStatus::PENDING;
};

#endif // ORDER_H
