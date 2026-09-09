#ifndef USER_H
#define USER_H

#include <string>

enum class Role {
    ORDER_MANAGER,
    DB_ADMIN,
    BACKEND_MANAGER,
    UNKNOWN
};

inline Role roleFromString(const std::string& s) {
    if (s == "ORDER_MANAGER")   return Role::ORDER_MANAGER;
    if (s == "DB_ADMIN")        return Role::DB_ADMIN;
    if (s == "BACKEND_MANAGER") return Role::BACKEND_MANAGER;
    return Role::UNKNOWN;
}

inline std::string roleToString(Role r) {
    switch (r) {
        case Role::ORDER_MANAGER:   return "ORDER_MANAGER";
        case Role::DB_ADMIN:        return "DB_ADMIN";
        case Role::BACKEND_MANAGER: return "BACKEND_MANAGER";
        default:                    return "UNKNOWN";
    }
}

struct User {
    int id = -1;
    std::string username;
    Role role = Role::UNKNOWN;
    bool isValid() const { return id != -1; }
};

#endif // USER_H
