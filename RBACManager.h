#ifndef RBAC_MANAGER_H
#define RBAC_MANAGER_H

#include <string>
#include "User.h"

enum class Permission {
    ADD_ORDER,
    LIST_ORDERS,
    UPDATE_INVENTORY,
    MANAGE_USERS,
    GENERATE_REPORT,
    VIEW_AUDIT_LOG
};

class RBACManager {
public:
    static bool isAuthorized(Role role, Permission perm);
    static void logAudit(const std::string& username, const std::string& action, bool result);
};

#endif // RBAC_MANAGER_H
