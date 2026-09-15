#include <cassert>
#include <iostream>
#include "../RBACManager.h"
#include "../User.h"

static int passed = 0;
static int failed = 0;

void check(bool condition, const std::string& testName) {
    if (condition) { std::cout << "  [PASS] " << testName << "\n"; ++passed; }
    else { std::cout << "  [FAIL] " << testName << "\n"; ++failed; }
}

int main() {
    std::cout << "Running RBACManager tests...\n\n";

    check(RBACManager::isAuthorized(Role::ORDER_MANAGER, Permission::ADD_ORDER), "ORDER_MANAGER can ADD_ORDER");
    check(RBACManager::isAuthorized(Role::ORDER_MANAGER, Permission::LIST_ORDERS), "ORDER_MANAGER can LIST_ORDERS");
    check(!RBACManager::isAuthorized(Role::ORDER_MANAGER, Permission::UPDATE_INVENTORY), "ORDER_MANAGER cannot UPDATE_INVENTORY");
    check(!RBACManager::isAuthorized(Role::ORDER_MANAGER, Permission::MANAGE_USERS), "ORDER_MANAGER cannot MANAGE_USERS");
    check(!RBACManager::isAuthorized(Role::ORDER_MANAGER, Permission::GENERATE_REPORT), "ORDER_MANAGER cannot GENERATE_REPORT");
    check(!RBACManager::isAuthorized(Role::ORDER_MANAGER, Permission::VIEW_AUDIT_LOG), "ORDER_MANAGER cannot VIEW_AUDIT_LOG");

    check(RBACManager::isAuthorized(Role::DB_ADMIN, Permission::UPDATE_INVENTORY), "DB_ADMIN can UPDATE_INVENTORY");
    check(RBACManager::isAuthorized(Role::DB_ADMIN, Permission::MANAGE_USERS), "DB_ADMIN can MANAGE_USERS");
    check(RBACManager::isAuthorized(Role::DB_ADMIN, Permission::LIST_ORDERS), "DB_ADMIN can LIST_ORDERS");
    check(!RBACManager::isAuthorized(Role::DB_ADMIN, Permission::ADD_ORDER), "DB_ADMIN cannot ADD_ORDER");
    check(!RBACManager::isAuthorized(Role::DB_ADMIN, Permission::GENERATE_REPORT), "DB_ADMIN cannot GENERATE_REPORT");
    check(!RBACManager::isAuthorized(Role::DB_ADMIN, Permission::VIEW_AUDIT_LOG), "DB_ADMIN cannot VIEW_AUDIT_LOG");

    check(RBACManager::isAuthorized(Role::BACKEND_MANAGER, Permission::GENERATE_REPORT), "BACKEND_MANAGER can GENERATE_REPORT");
    check(RBACManager::isAuthorized(Role::BACKEND_MANAGER, Permission::VIEW_AUDIT_LOG), "BACKEND_MANAGER can VIEW_AUDIT_LOG");
    check(RBACManager::isAuthorized(Role::BACKEND_MANAGER, Permission::LIST_ORDERS), "BACKEND_MANAGER can LIST_ORDERS");
    check(!RBACManager::isAuthorized(Role::BACKEND_MANAGER, Permission::ADD_ORDER), "BACKEND_MANAGER cannot ADD_ORDER");
    check(!RBACManager::isAuthorized(Role::BACKEND_MANAGER, Permission::UPDATE_INVENTORY), "BACKEND_MANAGER cannot UPDATE_INVENTORY");
    check(!RBACManager::isAuthorized(Role::BACKEND_MANAGER, Permission::MANAGE_USERS), "BACKEND_MANAGER cannot MANAGE_USERS");

    check(!RBACManager::isAuthorized(Role::UNKNOWN, Permission::ADD_ORDER), "UNKNOWN role cannot ADD_ORDER");
    check(!RBACManager::isAuthorized(Role::UNKNOWN, Permission::LIST_ORDERS), "UNKNOWN role cannot LIST_ORDERS (fail-safe default)");

    std::cout << "\n" << passed << " passed, " << failed << " failed.\n";
    return failed == 0 ? 0 : 1;
}
