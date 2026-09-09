#include <iostream>
#include "DatabaseDAO.h"
#include "InventoryManager.h"
#include "ReportGenerator.h"
#include "MenuCLI.h"

int main() {
    DatabaseDAO dao;
    if (!dao.connect()) {
        std::cerr << "Could not connect to the database. Check Config.h settings.\n";
        return 1;
    }

    InventoryManager inventory(dao);
    ReportGenerator reports(dao);
    MenuCLI menu(dao, inventory, reports);

    menu.run();

    dao.disconnect();
    return 0;
}
