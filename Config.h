#ifndef CONFIG_H
#define CONFIG_H

#include <string>

// ============================================================
// Database connection settings.
// These MUST match your EXISTING order_system database —
// this project connects to the same database you already set up,
// it does not create a new one.
// ============================================================
namespace Config {
    inline const std::string DB_HOST     = "127.0.0.1";
    inline const std::string DB_USER     = "root";
    inline const std::string DB_PASSWORD = "";   // set your MySQL root password here
    inline const std::string DB_NAME     = "order_system";
    inline const unsigned int DB_PORT    = 3306;

    inline const int MAX_LOGIN_ATTEMPTS  = 3;
    inline const int THREAD_POOL_SIZE    = 4;
}

#endif // CONFIG_H
