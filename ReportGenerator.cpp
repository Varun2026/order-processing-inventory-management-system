#include "ReportGenerator.h"
#include <fstream>
#include <iostream>

bool ReportGenerator::generateOrdersCSV(const std::string& filepath) {
    auto orders = dao_.listAllOrders();
    std::ofstream out(filepath);
    if (!out.is_open()) {
        std::cerr << "Could not open " << filepath << " for writing.\n";
        return false;
    }

    out << "OrderID,ItemID,Quantity,RequestedBy,Status\n";
    for (const auto& o : orders) {
        out << o.id << "," << o.itemId << "," << o.quantity << ","
            << o.requestedBy << "," << orderStatusToString(o.status) << "\n";
    }
    out.close();
    std::cout << "Report written to " << filepath << " (" << orders.size() << " rows)\n";
    return true;
}

bool ReportGenerator::generateAuditLogCSV(const std::string& filepath) {
    auto entries = dao_.listAuditLog();
    std::ofstream out(filepath);
    if (!out.is_open()) {
        std::cerr << "Could not open " << filepath << " for writing.\n";
        return false;
    }

    out << "ID,Username,Action,Result,CreatedAt\n";
    for (const auto& e : entries) {
        out << e.id << "," << e.username << "," << e.action << ","
            << e.result << "," << e.createdAt << "\n";
    }
    out.close();
    std::cout << "Audit report written to " << filepath << " (" << entries.size() << " rows)\n";
    return true;
}
