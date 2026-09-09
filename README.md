# Order Processing & Inventory Management System — Menu-Driven Edition

CPOC_Team20 | Multithreaded C++17 backend with role-based login, live flash-sale
demo, and CSV reporting. Connects to your **existing** `order_system` MySQL
database — this is a clean rebuild of the same engine you already deployed,
not a new project.

---

## 1. What changed from the previous version

- **Old version:** flag-based CLI (`./engine --user alice order add ...`), no password.
- **New version:** menu-driven, asks for **username + password** at startup,
  hides the password while typing, then shows a menu built for that person's role.
- Same database, same tables (`users`, `inventory`, `orders`, `audit_log`) —
  only one new column is added (`password_hash` on `users`).
- The flash-sale demo is now a **menu option**, not a shell `for` loop — pick
  "Run Flash Sale Demo", tell it the item and how many concurrent customers to
  simulate, and it launches real threads against the real database live.

---

## 2. One-time setup on the server (do this once)

### Step 2.1 — Clone the project

```bash
cd ~
git clone https://github.com/Varun2026/Order_processing_and_inventory_monitoring_system.git OrderProcessingSystem_v2
cd OrderProcessingSystem_v2
```

(Or unzip this package directly into a new folder on the server if you're
not pushing it to GitHub first.)

### Step 2.2 — Run the database migration (adds login credentials)

This does **not** touch your existing orders/inventory/audit data — it only
adds a `password_hash` column to `users` and sets demo passwords:

```bash
mysql -u root -p < sql/migration_add_auth.sql
```

Default demo logins created by the migration:

| Username | Password  | Role            |
|----------|-----------|-----------------|
| alice    | alice123  | ORDER_MANAGER   |
| bob      | bob123    | DB_ADMIN        |
| carol    | carol123  | BACKEND_MANAGER |

Passwords are never stored in plaintext — the migration stores SHA-256
hashes, and the program hashes whatever you type before comparing it.

### Step 2.3 — Check `Config.h`

Open `Config.h` and confirm `DB_PASSWORD` matches your MySQL root password
(everything else should already match your existing setup — same host,
same `order_system` database, same port 3306).

---

## 3. Compile (single line — do not press Enter until the end)

```bash
g++ -std=c++17 -Wall -Wextra -I. -pthread $(mysql_config --cflags) AuthManager.cpp DatabaseDAO.cpp MenuCLI.cpp main.cpp RBACManager.cpp ReportGenerator.cpp -o engine -lssl -lcrypto $(mysql_config --libs)
```

> Use Tab-completion for filenames to avoid stray spaces breaking the build.

---

## 4. Run it

```bash
./engine
```

You'll see a login prompt. Type a username, press Enter, then type the
password (it won't show on screen — that's intentional) and press Enter.

---

## 5. What each role sees after logging in

**Order Manager (e.g. alice)**
1. Place Order
2. View My Orders
3. View All Orders
4. Run Flash Sale Demo (concurrent orders)
5. Logout

**DB Admin (e.g. bob)**
1. View Inventory
2. Add Inventory Item
3. Update Inventory Stock
4. Delete Inventory Item
5. Add New User
6. View Audit Log
7. View All Orders
8. Logout

**Backend Manager (e.g. carol)**
1. Generate Orders Report (CSV)
2. View Audit Log
3. View All Orders
4. Logout

---

## 6. Running the flash-sale demo (for your presentation)

1. Log in as `alice` (or any Order Manager).
2. Choose **"Run Flash Sale Demo"**.
3. Enter the item ID to target (e.g. `101`) — it'll show you current stock.
4. Enter how many concurrent orders to simulate (e.g. `10`).
5. Enter quantity per order (e.g. `1`).
6. Watch the live output: you'll see multiple thread IDs interleaving as they
   process orders in parallel, each order transitioning
   `PENDING -> PROCESSING -> FULFILLED/REJECTED`.
7. It reports stock before/after automatically. Use option
   "View All Orders" afterward to see the full breakdown of who got
   FULFILLED vs REJECTED — this proves no overselling happened even under
   concurrent load.

Tip for a dramatic demo: set stock low first —
```sql
UPDATE inventory SET stock_qty = 5 WHERE item_id = 101;
```
— then simulate 10 concurrent orders. You should see exactly 5 FULFILLED
and 5 REJECTED, with final stock at 0.

---

## 7. Generating the CSV report

Log in as `carol` (Backend Manager) → "Generate Orders Report (CSV)". This
writes `orders_report.csv` in the current directory, with columns:
`OrderID, ItemID, Quantity, RequestedBy, Status`.

---

## 8. File overview

| File | Purpose |
|---|---|
| `main.cpp` | Entry point — connects to DB, launches the menu |
| `MenuCLI.h/.cpp` | All menu screens, role routing, flash-sale demo trigger |
| `AuthManager.h/.cpp` | Login prompt, hidden password entry, SHA-256 hashing |
| `DatabaseDAO.h/.cpp` | All MySQL queries (auth, orders, inventory, audit) |
| `OrderManager.h` | Order lifecycle + thread-safe processing logic |
| `InventoryManager.h` | Thread-safe wrapper around stock operations |
| `RBACManager.h/.cpp` | Role → permission rules |
| `ReportGenerator.h/.cpp` | CSV export for orders and audit log |
| `ThreadPool.h` | Generic thread pool (available for future batch processing) |
| `Order.h`, `OrderStatus.h`, `User.h` | Simple data models |
| `Config.h` | Database connection settings |
| `sql/migration_add_auth.sql` | One-time migration adding login credentials |

---

## 9. Known limitations (fine to mention if asked in your presentation)

- Passwords are SHA-256 hashed but not salted — acceptable for a training
  project; a production system would use bcrypt/argon2 with per-user salts.
- New users are added via the DB Admin's "Add New User" menu option now
  (this replaces the earlier gap where users had to be inserted manually
  via SQL).
