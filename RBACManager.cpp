#include "RBACManager.h"
#include <iostream>

bool RBACManager::isAuthorized(Role role, Permission perm) {
    switch (role) {
        case Role::ORDER_MANAGER:
            return perm == Permission::ADD_ORDER || perm == Permission::LIST_ORDERS;
        case Role::DB_ADMIN:
            return perm == Permission::UPDATE_INVENTORY ||
                   perm == Permission::MANAGE_USERS ||
                   perm == Permission::LIST_ORDERS;
        case Role::BACKEND_MANAGER:
            return perm == Permission::GENERATE_REPORT ||
                   perm == Permission::VIEW_AUDIT_LOG ||
                   perm == Permission::LIST_ORDERS;
        default:
            return false;
    }
}

// Audit logging itself is done via DatabaseDAO (needs a DB connection);
// this hook is kept for any in-process notifications/logging if needed later.
void RBACManager::logAudit(const std::string& username, const std::string& action, bool result) {
    (void)username; (void)action; (void)result;
}
