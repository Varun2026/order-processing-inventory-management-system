#ifndef ORDER_STATUS_H
#define ORDER_STATUS_H

#include <string>

enum class OrderStatus {
    PENDING,
    PROCESSING,
    FULFILLED,
    REJECTED
};

inline std::string orderStatusToString(OrderStatus s) {
    switch (s) {
        case OrderStatus::PENDING:    return "PENDING";
        case OrderStatus::PROCESSING: return "PROCESSING";
        case OrderStatus::FULFILLED:  return "FULFILLED";
        case OrderStatus::REJECTED:   return "REJECTED";
        default:                      return "UNKNOWN";
    }
}

#endif // ORDER_STATUS_H
