#ifndef REPORT_GENERATOR_H
#define REPORT_GENERATOR_H

#include <string>
#include "DatabaseDAO.h"

class ReportGenerator {
public:
    explicit ReportGenerator(DatabaseDAO& dao) : dao_(dao) {}

    // Writes all orders to a CSV file. Returns true on success.
    bool generateOrdersCSV(const std::string& filepath);

    // Writes the audit log to a CSV file. Returns true on success.
    bool generateAuditLogCSV(const std::string& filepath);

private:
    DatabaseDAO& dao_;
};

#endif // REPORT_GENERATOR_H
