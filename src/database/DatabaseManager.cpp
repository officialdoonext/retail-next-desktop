#include "DatabaseManager.h"
#include "../core/Constants.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <cmath>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::DatabaseManager()
    : m_connected(false)
    , m_currentUser(QStringLiteral("Administrator"))
    , m_currentRole(QStringLiteral("Store Admin"))
    , m_businessType(RetailNext::BusinessType::GeneralRetail)
    , m_storeName(QStringLiteral("RetailNext Flagship Store"))
{
}

DatabaseManager::~DatabaseManager() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

void DatabaseManager::setCurrentUser(const QString& user, const QString& role) {
    m_currentUser = user;
    m_currentRole = role;
}

bool DatabaseManager::initialize(const QString& customPath) {
    if (m_connected && m_db.isOpen()) {
        return true;
    }

    if (!customPath.isEmpty()) {
        m_dbPath = customPath;
    } else {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (dataDir.isEmpty()) {
            dataDir = QDir::currentPath();
        }
        QDir dir(dataDir);
        if (!dir.exists()) {
            dir.mkpath(QStringLiteral("."));
        }
        m_dbPath = dir.filePath(RetailNext::DB_FILENAME);
    }

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    m_db.setDatabaseName(m_dbPath);

    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        m_connected = false;
        qWarning() << "Failed to open SQLite database:" << m_lastError;
        return false;
    }

    m_connected = true;

    // WAL mode and foreign keys for performance and crash resilience
    QSqlQuery pragmaQuery(m_db);
    pragmaQuery.exec(QStringLiteral("PRAGMA foreign_keys = ON;"));
    pragmaQuery.exec(QStringLiteral("PRAGMA journal_mode = WAL;"));

    if (!createTables()) {
        qWarning() << "Failed creating database schema:" << m_lastError;
        return false;
    }

    seedInitialData();
    return true;
}

bool DatabaseManager::createTables() {
    QSqlQuery query(m_db);

    const char* ddl[] = {
        // Business Settings
        "CREATE TABLE IF NOT EXISTS business_settings ("
        "  key TEXT PRIMARY KEY,"
        "  value TEXT NOT NULL,"
        "  description TEXT"
        ");",

        // Branches
        "CREATE TABLE IF NOT EXISTS branches ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  code TEXT NOT NULL UNIQUE,"
        "  name TEXT NOT NULL,"
        "  address TEXT,"
        "  phone TEXT,"
        "  gst_number TEXT,"
        "  is_active INTEGER NOT NULL DEFAULT 1"
        ");",

        // Registers
        "CREATE TABLE IF NOT EXISTS registers ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  branch_id INTEGER DEFAULT 1,"
        "  code TEXT NOT NULL UNIQUE,"
        "  name TEXT NOT NULL,"
        "  is_open INTEGER NOT NULL DEFAULT 1"
        ");",

        // Categories
        "CREATE TABLE IF NOT EXISTS categories ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name TEXT NOT NULL UNIQUE,"
        "  code TEXT,"
        "  description TEXT,"
        "  is_active INTEGER NOT NULL DEFAULT 1"
        ");",

        // Brands
        "CREATE TABLE IF NOT EXISTS brands ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name TEXT NOT NULL UNIQUE,"
        "  code TEXT,"
        "  is_active INTEGER NOT NULL DEFAULT 1"
        ");",

        // Units
        "CREATE TABLE IF NOT EXISTS units ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name TEXT NOT NULL UNIQUE,"
        "  code TEXT NOT NULL UNIQUE,"
        "  is_decimal_allowed INTEGER NOT NULL DEFAULT 0"
        ");",

        // Products
        "CREATE TABLE IF NOT EXISTS products ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  sku TEXT NOT NULL UNIQUE,"
        "  barcode TEXT NOT NULL UNIQUE,"
        "  name TEXT NOT NULL,"
        "  category_id INTEGER DEFAULT 1,"
        "  brand_id INTEGER DEFAULT 1,"
        "  unit_id INTEGER DEFAULT 1,"
        "  cost_price REAL NOT NULL DEFAULT 0.0,"
        "  selling_price REAL NOT NULL DEFAULT 0.0,"
        "  mrp REAL NOT NULL DEFAULT 0.0,"
        "  wholesale_price REAL NOT NULL DEFAULT 0.0,"
        "  tax_rate REAL NOT NULL DEFAULT 0.0,"
        "  current_stock REAL NOT NULL DEFAULT 0.0,"
        "  reorder_level REAL NOT NULL DEFAULT 5.0,"
        "  is_active INTEGER NOT NULL DEFAULT 1,"
        "  business_type_tag TEXT DEFAULT 'GeneralRetail'"
        ");",

        // Product Variants
        "CREATE TABLE IF NOT EXISTS product_variants ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  product_id INTEGER NOT NULL,"
        "  sku TEXT NOT NULL UNIQUE,"
        "  barcode TEXT NOT NULL UNIQUE,"
        "  variant_name TEXT NOT NULL,"
        "  attribute_size TEXT,"
        "  attribute_color TEXT,"
        "  price REAL NOT NULL DEFAULT 0.0,"
        "  stock_qty REAL NOT NULL DEFAULT 0.0"
        ");",

        // Customers
        "CREATE TABLE IF NOT EXISTS customers ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  code TEXT NOT NULL UNIQUE,"
        "  name TEXT NOT NULL,"
        "  phone TEXT,"
        "  email TEXT,"
        "  address TEXT,"
        "  gst_no TEXT,"
        "  customer_group TEXT DEFAULT 'Retail',"
        "  loyalty_points INTEGER DEFAULT 0,"
        "  store_credit REAL DEFAULT 0.0,"
        "  outstanding_balance REAL DEFAULT 0.0,"
        "  credit_limit REAL DEFAULT 0.0,"
        "  is_active INTEGER DEFAULT 1"
        ");",

        // Suppliers
        "CREATE TABLE IF NOT EXISTS suppliers ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  code TEXT NOT NULL UNIQUE,"
        "  name TEXT NOT NULL,"
        "  contact_person TEXT,"
        "  phone TEXT,"
        "  email TEXT,"
        "  address TEXT,"
        "  gst_no TEXT,"
        "  payment_terms TEXT DEFAULT 'Net 30',"
        "  credit_limit REAL DEFAULT 0.0,"
        "  outstanding_balance REAL DEFAULT 0.0,"
        "  is_active INTEGER DEFAULT 1"
        ");",

        // Stock movements ledger
        "CREATE TABLE IF NOT EXISTS stock_movements ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  product_id INTEGER NOT NULL,"
        "  reference_no TEXT NOT NULL,"
        "  movement_type TEXT NOT NULL,"
        "  quantity REAL NOT NULL,"
        "  unit_cost REAL DEFAULT 0.0,"
        "  balance_after REAL NOT NULL,"
        "  user_name TEXT,"
        "  notes TEXT,"
        "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",

        // Sales Invoices
        "CREATE TABLE IF NOT EXISTS sales_invoices ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  invoice_no TEXT NOT NULL UNIQUE,"
        "  branch_id INTEGER DEFAULT 1,"
        "  register_id INTEGER DEFAULT 1,"
        "  cashier_name TEXT NOT NULL,"
        "  customer_id INTEGER DEFAULT 1,"
        "  customer_name TEXT NOT NULL,"
        "  subtotal REAL NOT NULL DEFAULT 0.0,"
        "  discount_amount REAL NOT NULL DEFAULT 0.0,"
        "  tax_amount REAL NOT NULL DEFAULT 0.0,"
        "  round_off REAL NOT NULL DEFAULT 0.0,"
        "  net_payable REAL NOT NULL DEFAULT 0.0,"
        "  tendered_amount REAL NOT NULL DEFAULT 0.0,"
        "  change_due REAL NOT NULL DEFAULT 0.0,"
        "  payment_mode TEXT NOT NULL DEFAULT 'Cash',"
        "  payment_status TEXT NOT NULL DEFAULT 'Paid',"
        "  notes TEXT,"
        "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",

        // Sales Invoice Items
        "CREATE TABLE IF NOT EXISTS sales_invoice_items ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  invoice_id INTEGER NOT NULL,"
        "  product_id INTEGER NOT NULL,"
        "  variant_id INTEGER DEFAULT 0,"
        "  barcode TEXT,"
        "  item_name TEXT NOT NULL,"
        "  quantity REAL NOT NULL,"
        "  unit_price REAL NOT NULL,"
        "  discount_amount REAL DEFAULT 0.0,"
        "  tax_rate REAL DEFAULT 0.0,"
        "  tax_amount REAL DEFAULT 0.0,"
        "  line_total REAL NOT NULL"
        ");",

        // Held bills
        "CREATE TABLE IF NOT EXISTS held_bills ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  customer_name TEXT NOT NULL,"
        "  cart_json TEXT NOT NULL,"
        "  total_amount REAL NOT NULL DEFAULT 0.0,"
        "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",

        // Purchase Orders
        "CREATE TABLE IF NOT EXISTS purchase_orders ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  po_number TEXT NOT NULL UNIQUE,"
        "  supplier_id INTEGER NOT NULL,"
        "  order_date DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "  expected_date TEXT,"
        "  total_amount REAL NOT NULL DEFAULT 0.0,"
        "  status TEXT DEFAULT 'Received',"
        "  notes TEXT"
        ");",

        // Purchase Invoices
        "CREATE TABLE IF NOT EXISTS purchase_invoices ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  invoice_no TEXT NOT NULL UNIQUE,"
        "  supplier_invoice_no TEXT,"
        "  supplier_id INTEGER NOT NULL,"
        "  invoice_date DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "  subtotal REAL NOT NULL DEFAULT 0.0,"
        "  tax_amount REAL NOT NULL DEFAULT 0.0,"
        "  total_amount REAL NOT NULL DEFAULT 0.0,"
        "  payment_status TEXT DEFAULT 'Unpaid',"
        "  notes TEXT"
        ");",

        // Sales returns
        "CREATE TABLE IF NOT EXISTS sales_returns ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  return_no TEXT NOT NULL UNIQUE,"
        "  original_invoice_no TEXT NOT NULL,"
        "  customer_id INTEGER DEFAULT 1,"
        "  return_date DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "  refund_amount REAL NOT NULL,"
        "  refund_method TEXT NOT NULL,"
        "  reason TEXT"
        ");",

        // Cash register sessions
        "CREATE TABLE IF NOT EXISTS cash_register_sessions ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  register_id INTEGER DEFAULT 1,"
        "  cashier_name TEXT NOT NULL,"
        "  opened_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "  closed_at DATETIME,"
        "  opening_cash REAL NOT NULL DEFAULT 0.0,"
        "  cash_sales REAL NOT NULL DEFAULT 0.0,"
        "  cash_in REAL NOT NULL DEFAULT 0.0,"
        "  cash_out REAL NOT NULL DEFAULT 0.0,"
        "  expected_cash REAL NOT NULL DEFAULT 0.0,"
        "  counted_cash REAL NOT NULL DEFAULT 0.0,"
        "  variance REAL NOT NULL DEFAULT 0.0,"
        "  status TEXT NOT NULL DEFAULT 'Open'"
        ");",

        // Cash in/out transactions
        "CREATE TABLE IF NOT EXISTS cash_transactions ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  session_id INTEGER NOT NULL,"
        "  trans_type TEXT NOT NULL,"
        "  amount REAL NOT NULL,"
        "  reason TEXT NOT NULL,"
        "  reference TEXT,"
        "  user_name TEXT,"
        "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",

        // Expenses
        "CREATE TABLE IF NOT EXISTS expenses ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  expense_no TEXT NOT NULL UNIQUE,"
        "  category_name TEXT NOT NULL,"
        "  payee TEXT NOT NULL,"
        "  amount REAL NOT NULL DEFAULT 0.0,"
        "  tax_amount REAL NOT NULL DEFAULT 0.0,"
        "  payment_method TEXT NOT NULL DEFAULT 'Cash',"
        "  branch TEXT DEFAULT 'Main Store',"
        "  notes TEXT,"
        "  created_by TEXT,"
        "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",

        // Users
        "CREATE TABLE IF NOT EXISTS users ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username TEXT NOT NULL UNIQUE,"
        "  display_name TEXT NOT NULL,"
        "  phone TEXT,"
        "  password TEXT,"
        "  role TEXT NOT NULL DEFAULT 'Cashier',"
        "  permissions TEXT,"
        "  is_active INTEGER NOT NULL DEFAULT 1,"
        "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",

        // Stores
        "CREATE TABLE IF NOT EXISTS stores ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  code TEXT NOT NULL UNIQUE,"
        "  name TEXT NOT NULL,"
        "  phone TEXT,"
        "  email TEXT,"
        "  city TEXT,"
        "  address TEXT,"
        "  gst_number TEXT,"
        "  is_active INTEGER NOT NULL DEFAULT 1,"
        "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",

        // Warehouses
        "CREATE TABLE IF NOT EXISTS warehouses ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  code TEXT NOT NULL UNIQUE,"
        "  name TEXT NOT NULL,"
        "  phone TEXT,"
        "  email TEXT,"
        "  city TEXT,"
        "  address TEXT,"
        "  is_active INTEGER NOT NULL DEFAULT 1,"
        "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");",

        // Audit logs
        "CREATE TABLE IF NOT EXISTS audit_logs ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  user_name TEXT,"
        "  action_type TEXT NOT NULL,"
        "  entity_type TEXT NOT NULL,"
        "  entity_id TEXT,"
        "  description TEXT,"
        "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");"
    };

    for (const char* sql : ddl) {
        if (!query.exec(QString::fromUtf8(sql))) {
            m_lastError = query.lastError().text();
            qWarning() << "Failed executing DDL:" << m_lastError;
            return false;
        }
    }

    // Migrate users table if exists from older version without phone/password/permissions
    query.exec(QStringLiteral("ALTER TABLE users ADD COLUMN phone TEXT;"));
    query.exec(QStringLiteral("ALTER TABLE users ADD COLUMN password TEXT;"));
    query.exec(QStringLiteral("ALTER TABLE users ADD COLUMN permissions TEXT;"));

    return true;
}

bool DatabaseManager::seedInitialData() {
    QSqlQuery query(m_db);

    // 1. Settings
    query.exec(QStringLiteral("SELECT COUNT(*) FROM business_settings;"));
    if (query.next() && query.value(0).toInt() == 0) {
        query.prepare(QStringLiteral("INSERT INTO business_settings (key, value, description) VALUES (:k, :v, :d);"));
        auto addSetting = [&](const QString& k, const QString& v, const QString& d) {
            query.bindValue(QStringLiteral(":k"), k);
            query.bindValue(QStringLiteral(":v"), v);
            query.bindValue(QStringLiteral(":d"), d);
            query.exec();
        };

        addSetting(QStringLiteral("business_name"), QStringLiteral("RetailNext Flagship Store"), QStringLiteral("Registered Store Name"));
        addSetting(QStringLiteral("business_type"), QStringLiteral("GeneralRetail"), QStringLiteral("Selected Profile"));
        addSetting(QStringLiteral("currency_symbol"), QStringLiteral("$"), QStringLiteral("Store Currency Symbol"));
        addSetting(QStringLiteral("tax_mode"), QStringLiteral("Exclusive"), QStringLiteral("Tax Calculation Mode"));
        addSetting(QStringLiteral("branch_name"), QStringLiteral("Main Store (Flagship)"), QStringLiteral("Current Branch"));
        addSetting(QStringLiteral("register_code"), QStringLiteral("REG-01"), QStringLiteral("Default POS Terminal Register"));
        addSetting(QStringLiteral("receipt_header"), QStringLiteral("RetailNext Superstore\n100 Market Square, Tech City\nTel: +1 (800) 555-0199\nGST/Tax ID: TX-88992211"), QStringLiteral("Receipt Header"));
        addSetting(QStringLiteral("receipt_footer"), QStringLiteral("Thank you for shopping with us!\nExchange within 14 days with original receipt."), QStringLiteral("Receipt Footer"));
    }

    // 2. Branches & Registers
    query.exec(QStringLiteral("SELECT COUNT(*) FROM branches;"));
    if (query.next() && query.value(0).toInt() == 0) {
        query.exec(QStringLiteral(
            "INSERT INTO branches (code, name, address, phone, gst_number) "
            "VALUES ('BR-001', 'Main Store (Flagship)', '100 Market Square, Tech City', '555-0199', 'TX-88992211');"
        ));
        query.exec(QStringLiteral(
            "INSERT INTO registers (branch_id, code, name, is_open) "
            "VALUES (1, 'REG-01', 'Main Checkout Register #1', 1);"
        ));
    }

    // 3. Categories
    query.exec(QStringLiteral("SELECT COUNT(*) FROM categories;"));
    if (query.next() && query.value(0).toInt() == 0) {
        query.exec(QStringLiteral("INSERT INTO categories (name, code, description) VALUES "
            "('Groceries & Staples', 'GROC', 'Everyday staples, grains and pantry'),"
            "('Beverages', 'BEV', 'Juices, water, and soft drinks'),"
            "('Dairy & Eggs', 'DAIRY', 'Fresh dairy products and eggs'),"
            "('Apparel & Fashion', 'APP', 'Clothing, shirts, jeans and footwear'),"
            "('Electronics & Gadgets', 'ELEC', 'Accessories, cables and electronics'),"
            "('Personal Care', 'PC', 'Soaps, shampoos and hygiene');"));
    }

    // 4. Brands
    query.exec(QStringLiteral("SELECT COUNT(*) FROM brands;"));
    if (query.next() && query.value(0).toInt() == 0) {
        query.exec(QStringLiteral("INSERT INTO brands (name, code) VALUES "
            "('Nestle', 'NEST'),"
            "('Heritage', 'HERI'),"
            "('Kellogg''s', 'KELL'),"
            "('Zara Classics', 'ZARA'),"
            "('Apple', 'AAPL'),"
            "('Logitech', 'LOGI'),"
            "('General Retail', 'GEN');"));
    }

    // 5. Units
    query.exec(QStringLiteral("SELECT COUNT(*) FROM units;"));
    if (query.next() && query.value(0).toInt() == 0) {
        query.exec(QStringLiteral("INSERT INTO units (name, code, is_decimal_allowed) VALUES "
            "('Pieces', 'PCS', 0),"
            "('Kilograms', 'KG', 1),"
            "('Grams', 'G', 1),"
            "('Litres', 'LTR', 1),"
            "('Pack', 'PAC', 0),"
            "('Box', 'BOX', 0);"));
    }

    // 6. Users
    query.exec(QStringLiteral("SELECT COUNT(*) FROM users;"));
    if (query.next() && query.value(0).toInt() == 0) {
        query.exec(QStringLiteral("INSERT INTO users (username, display_name, phone, password, role, permissions) VALUES "
            "('admin', 'Administrator', '+1 555-0100', 'admin123', 'Store Admin', 'Dashboard:view,edit;Masters:view,edit;Purchase:view,edit;Inventory:view,edit;Sales:view,edit;Customers:view,edit;Accounts:view,edit;Reports:view,edit;Access:view,edit;Settings:view,edit'),"
            "('cashier1', 'Alex Johnson', '+1 555-0101', 'cashier1', 'Cashier', 'Dashboard:view;Sales:view,edit;Customers:view'),"
            "('manager', 'Sarah Miller', '+1 555-0102', 'manager1', 'Manager', 'Dashboard:view,edit;Masters:view,edit;Purchase:view,edit;Inventory:view,edit;Sales:view,edit;Customers:view,edit;Accounts:view,edit;Reports:view,edit');"));
    } else {
        // Upgrade existing seeded users with credentials and permissions if missing
        query.exec(QStringLiteral("UPDATE users SET phone = '+1 555-0100', password = 'admin123', permissions = 'Dashboard:view,edit;Masters:view,edit;Purchase:view,edit;Inventory:view,edit;Sales:view,edit;Customers:view,edit;Accounts:view,edit;Reports:view,edit;Access:view,edit;Settings:view,edit' WHERE username = 'admin' AND (password IS NULL OR password = '');"));
        query.exec(QStringLiteral("UPDATE users SET phone = '+1 555-0101', password = 'cashier1', permissions = 'Dashboard:view;Sales:view,edit;Customers:view' WHERE username = 'cashier1' AND (password IS NULL OR password = '');"));
        query.exec(QStringLiteral("UPDATE users SET phone = '+1 555-0102', password = 'manager1', permissions = 'Dashboard:view,edit;Masters:view,edit;Purchase:view,edit;Inventory:view,edit;Sales:view,edit;Customers:view,edit;Accounts:view,edit;Reports:view,edit' WHERE username = 'manager' AND (password IS NULL OR password = '');"));
    }

    // 6b. Stores
    query.exec(QStringLiteral("SELECT COUNT(*) FROM stores;"));
    if (query.next() && query.value(0).toInt() == 0) {
        query.exec(QStringLiteral(
            "INSERT INTO stores (code, name, phone, email, city, address, gst_number) VALUES "
            "('STR-001', 'Main Flagship Store', '+1 555-0199', 'flagship@retailnext.com', 'Tech City', '100 Market Square, Suite 100', 'TX-88992211'),"
            "('STR-002', 'Downtown Branch Store', '+1 555-0144', 'downtown@retailnext.com', 'Tech City', '24 Commercial Avenue, Downtown', 'TX-88992222');"
        ));
    }

    // 6c. Warehouses
    query.exec(QStringLiteral("SELECT COUNT(*) FROM warehouses;"));
    if (query.next() && query.value(0).toInt() == 0) {
        query.exec(QStringLiteral(
            "INSERT INTO warehouses (code, name, phone, email, city, address) VALUES "
            "('WH-001', 'Central Logistics Depot', '+1 555-0800', 'logistics@retailnext.com', 'Tech City', 'Harbor Bay Industrial Park, Bldg 4'),"
            "('WH-002', 'North Hub Fulfillment Center', '+1 555-0811', 'hubnorth@retailnext.com', 'Tech City', '88 North Logistics Expressway');"
        ));
    }

    // 7. Customers
    query.exec(QStringLiteral("SELECT COUNT(*) FROM customers;"));
    if (query.next() && query.value(0).toInt() == 0) {
        query.exec(QStringLiteral("INSERT INTO customers (code, name, phone, email, address, gst_no, customer_group, loyalty_points, store_credit, outstanding_balance, credit_limit) VALUES "
            "('CUST-0001', 'Walk-in Customer', '000-000-0000', '', 'Counter Checkout', '', 'Retail', 0, 0.0, 0.0, 0.0),"
            "('CUST-0002', 'Sarah Jenkins', '555-0142', 'sarah.j@example.com', '14 Elm Street, City', 'TX-771122', 'VIP', 160, 15.0, 45.50, 500.0),"
            "('CUST-0003', 'Robert Smith', '555-0198', 'r.smith@example.com', '45 North Avenue, City', '', 'Retail', 85, 0.0, 0.0, 200.0),"
            "('CUST-0004', 'Acme Wholesale Corp', '555-0275', 'acme@wholesale.com', 'Building 4, Industrial Zone', 'TX-990011', 'Wholesale', 740, 0.0, 1250.0, 5000.0),"
            "('CUST-0005', 'Emily Davis', '555-0311', 'emily.d@example.com', '88 Sunset Blvd', '', 'Retail', 40, 0.0, 0.0, 300.0);"));
    }

    // 8. Suppliers
    query.exec(QStringLiteral("SELECT COUNT(*) FROM suppliers;"));
    if (query.next() && query.value(0).toInt() == 0) {
        query.exec(QStringLiteral("INSERT INTO suppliers (code, name, contact_person, phone, email, address, gst_no, payment_terms, credit_limit, outstanding_balance) VALUES "
            "('SUP-001', 'Metro Wholesale Distributors', 'David Miller', '555-1100', 'sales@metrowholesale.com', 'Harbor Bay Logistics Hub', 'TX-445566', 'Net 30', 10000.0, 1840.0),"
            "('SUP-002', 'Acme Fresh Farm Supplies', 'Maria Garcia', '555-1122', 'orders@acmefresh.com', 'Green Valley Farms #12', 'TX-223344', 'Net 15', 5000.0, 520.0),"
            "('SUP-003', 'Global Electronics Supply Ltd', 'Kenji Sato', '555-1133', 'kenji@globalsupply.com', 'Tech Export Zone, Bld 3', 'TX-887766', 'Immediate', 2000.0, 0.0);"));
    }

    // 9. Products & Inventory
    query.exec(QStringLiteral("SELECT COUNT(*) FROM products;"));
    if (query.next() && query.value(0).toInt() == 0) {
        query.exec(QStringLiteral("INSERT INTO products (sku, barcode, name, category_id, brand_id, unit_id, cost_price, selling_price, mrp, wholesale_price, tax_rate, current_stock, reorder_level, is_active, business_type_tag) VALUES "
            "('SKU-1001', '890103000001', 'Organic Whole Milk 1L', 3, 2, 4, 1.20, 1.80, 2.00, 1.50, 5.0, 42.0, 15.0, 1, 'GroceriesProvisions'),"
            "('SKU-1002', '890103000002', 'Premium Basmati Rice 5kg', 1, 7, 2, 7.50, 11.99, 13.50, 10.00, 5.0, 28.0, 10.0, 1, 'GroceriesProvisions'),"
            "('SKU-1003', '890103000003', 'Extra Virgin Olive Oil 500ml', 1, 7, 1, 6.20, 9.50, 10.50, 8.20, 5.0, 15.0, 8.0, 1, 'GeneralRetail'),"
            "('SKU-1004', '890103000004', 'Farm Fresh Free-Range Eggs 12pk', 3, 2, 6, 2.10, 3.49, 3.99, 2.80, 0.0, 18.0, 8.0, 1, 'GroceriesProvisions'),"
            "('SKU-1005', '890103000005', 'Cotton Oxford Shirt - Blue (M)', 4, 4, 1, 14.00, 29.99, 34.99, 24.00, 12.0, 16.0, 6.0, 1, 'ClothingFashion'),"
            "('SKU-1006', '890103000006', 'Cotton Oxford Shirt - Black (L)', 4, 4, 1, 14.00, 29.99, 34.99, 24.00, 12.0, 12.0, 6.0, 1, 'ClothingFashion'),"
            "('SKU-1007', '890103000007', 'Slim Fit Stretch Denim Jeans (32)', 4, 4, 1, 18.00, 44.99, 49.99, 35.00, 12.0, 22.0, 5.0, 1, 'ClothingFashion'),"
            "('SKU-1008', '890103000008', 'Wireless Bluetooth Optical Mouse', 5, 6, 1, 8.50, 19.99, 24.99, 15.00, 18.0, 7.0, 4.0, 1, 'Electronics'),"
            "('SKU-1009', '890103000009', 'Braided Fast Charging USB-C Cable', 5, 6, 1, 2.20, 6.99, 8.99, 4.50, 18.0, 3.0, 10.0, 1, 'Electronics'),"
            "('SKU-1010', '890103000010', 'Sparkling Mineral Water 750ml', 2, 1, 1, 0.80, 1.75, 2.00, 1.30, 5.0, 35.0, 12.0, 1, 'Supermarket'),"
            "('SKU-1011', '890103000011', 'Roasted California Almonds 200g', 1, 7, 5, 3.40, 5.99, 6.50, 4.80, 5.0, 2.0, 8.0, 1, 'GroceriesProvisions'),"
            "('SKU-1012', '890103000012', 'Fresh Artisan Sourdough Loaf', 1, 7, 1, 1.50, 3.25, 3.75, 2.50, 0.0, 0.0, 5.0, 1, 'Bakery');"));

        // Initial stock ledger entries
        query.exec(QStringLiteral("INSERT INTO stock_movements (product_id, reference_no, movement_type, quantity, unit_cost, balance_after, user_name, notes) VALUES "
            "(1, 'OPEN-001', 'Opening Stock', 42.0, 1.20, 42.0, 'Administrator', 'Initial store inventory balance'),"
            "(2, 'OPEN-002', 'Opening Stock', 28.0, 7.50, 28.0, 'Administrator', 'Initial store inventory balance'),"
            "(3, 'OPEN-003', 'Opening Stock', 15.0, 6.20, 15.0, 'Administrator', 'Initial store inventory balance'),"
            "(4, 'OPEN-004', 'Opening Stock', 18.0, 2.10, 18.0, 'Administrator', 'Initial store inventory balance'),"
            "(5, 'OPEN-005', 'Opening Stock', 16.0, 14.00, 16.0, 'Administrator', 'Initial store inventory balance'),"
            "(6, 'OPEN-006', 'Opening Stock', 12.0, 14.00, 12.0, 'Administrator', 'Initial store inventory balance'),"
            "(7, 'OPEN-007', 'Opening Stock', 22.0, 18.00, 22.0, 'Administrator', 'Initial store inventory balance'),"
            "(8, 'OPEN-008', 'Opening Stock', 7.0, 8.50, 7.0, 'Administrator', 'Initial store inventory balance'),"
            "(9, 'OPEN-009', 'Opening Stock', 3.0, 2.20, 3.0, 'Administrator', 'Initial store inventory balance'),"
            "(10, 'OPEN-010', 'Opening Stock', 35.0, 0.80, 35.0, 'Administrator', 'Initial store inventory balance'),"
            "(11, 'OPEN-011', 'Opening Stock', 2.0, 3.40, 2.0, 'Administrator', 'Initial store inventory balance'),"
            "(12, 'OPEN-012', 'Opening Stock', 0.0, 1.50, 0.0, 'Administrator', 'Initial store inventory balance');"));
    }

    // 10. Cash Register Session
    query.exec(QStringLiteral("SELECT COUNT(*) FROM cash_register_sessions;"));
    if (query.next() && query.value(0).toInt() == 0) {
        query.exec(QStringLiteral(
            "INSERT INTO cash_register_sessions (register_id, cashier_name, opened_at, opening_cash, cash_sales, cash_in, cash_out, expected_cash, counted_cash, variance, status) "
            "VALUES (1, 'Administrator', datetime('now', '-4 hours'), 250.00, 145.50, 50.00, 25.00, 420.50, 0.0, 0.0, 'Open');"
        ));
    }

    // 11. Initial Sales Invoices & Items
    query.exec(QStringLiteral("SELECT COUNT(*) FROM sales_invoices;"));
    if (query.next() && query.value(0).toInt() == 0) {
        // Invoice 1: Cash
        query.exec(QStringLiteral(
            "INSERT INTO sales_invoices (invoice_no, branch_id, register_id, cashier_name, customer_id, customer_name, subtotal, discount_amount, tax_amount, round_off, net_payable, tendered_amount, change_due, payment_mode, payment_status, notes, created_at) "
            "VALUES ('INV-2026-0840', 1, 1, 'Administrator', 1, 'Walk-in Customer', 43.33, 0.00, 2.17, 0.00, 45.50, 50.00, 4.50, 'Cash', 'Paid', 'Quick counter checkout', datetime('now', '-3 hours'));"
        ));
        query.exec(QStringLiteral("INSERT INTO sales_invoice_items (invoice_id, product_id, item_name, quantity, unit_price, discount_amount, tax_rate, tax_amount, line_total) VALUES "
            "(1, 1, 'Organic Whole Milk 1L', 2, 1.80, 0.0, 5.0, 0.18, 3.60),"
            "(1, 2, 'Premium Basmati Rice 5kg', 1, 11.99, 0.0, 5.0, 0.60, 11.99),"
            "(1, 5, 'Cotton Oxford Shirt - Blue (M)', 1, 29.99, 0.0, 12.0, 3.60, 29.99);"));

        // Invoice 2: Card
        query.exec(QStringLiteral(
            "INSERT INTO sales_invoices (invoice_no, branch_id, register_id, cashier_name, customer_id, customer_name, subtotal, discount_amount, tax_amount, round_off, net_payable, tendered_amount, change_due, payment_mode, payment_status, notes, created_at) "
            "VALUES ('INV-2026-0841', 1, 1, 'Administrator', 3, 'Robert Smith', 74.98, 5.00, 8.40, 0.02, 78.40, 78.40, 0.00, 'Card', 'Paid', 'Card terminal auth #9921', datetime('now', '-2 hours'));"
        ));
        query.exec(QStringLiteral("INSERT INTO sales_invoice_items (invoice_id, product_id, item_name, quantity, unit_price, discount_amount, tax_rate, tax_amount, line_total) VALUES "
            "(2, 7, 'Slim Fit Stretch Denim Jeans (32)', 1, 44.99, 5.00, 12.0, 4.80, 39.99),"
            "(2, 8, 'Wireless Bluetooth Optical Mouse', 1, 19.99, 0.0, 18.0, 3.60, 19.99);"));

        // Invoice 3: UPI
        query.exec(QStringLiteral(
            "INSERT INTO sales_invoices (invoice_no, branch_id, register_id, cashier_name, customer_id, customer_name, subtotal, discount_amount, tax_amount, round_off, net_payable, tendered_amount, change_due, payment_mode, payment_status, notes, created_at) "
            "VALUES ('INV-2026-0842', 1, 1, 'Alex Johnson', 2, 'Sarah Jenkins', 28.50, 0.00, 1.42, 0.08, 30.00, 30.00, 0.00, 'UPI', 'Paid', 'UPI txn ref 998811', datetime('now', '-45 minutes'));"
        ));
        query.exec(QStringLiteral("INSERT INTO sales_invoice_items (invoice_id, product_id, item_name, quantity, unit_price, discount_amount, tax_rate, tax_amount, line_total) VALUES "
            "(3, 3, 'Extra Virgin Olive Oil 500ml', 1, 9.50, 0.0, 5.0, 0.48, 9.50),"
            "(3, 10, 'Sparkling Mineral Water 750ml', 2, 1.75, 0.0, 5.0, 0.18, 3.50);"));

        // Invoice 4: Wholesale Credit
        query.exec(QStringLiteral(
            "INSERT INTO sales_invoices (invoice_no, branch_id, register_id, cashier_name, customer_id, customer_name, subtotal, discount_amount, tax_amount, round_off, net_payable, tendered_amount, change_due, payment_mode, payment_status, notes, created_at) "
            "VALUES ('INV-2026-0843', 1, 1, 'Administrator', 4, 'Acme Wholesale Corp', 450.00, 20.00, 21.50, 0.00, 451.50, 0.00, 0.00, 'Credit', 'Unpaid', 'PO-8891 Net 30 Terms', datetime('now', '-15 minutes'));"
        ));
    }

    // 12. Expenses
    query.exec(QStringLiteral("SELECT COUNT(*) FROM expenses;"));
    if (query.next() && query.value(0).toInt() == 0) {
        query.exec(QStringLiteral("INSERT INTO expenses (expense_no, category_name, payee, amount, tax_amount, payment_method, branch, notes, created_by, created_at) VALUES "
            "('EXP-2026-001', 'Utilities', 'City Power & Electric', 145.00, 0.0, 'Bank Transfer', 'Main Store', 'Monthly electricity bill', 'Administrator', datetime('now', '-2 days')),"
            "('EXP-2026-002', 'Packaging & Supplies', 'Apex Paper & Carton Co', 35.00, 2.50, 'Cash', 'Main Store', '10 rolls thermal receipt paper', 'Alex Johnson', datetime('now', '-5 hours')),"
            "('EXP-2026-003', 'Repairs & Maintenance', 'Local Aircon Service', 85.00, 0.0, 'Cash', 'Main Store', 'AC filter maintenance', 'Administrator', datetime('now', '-1 day'));"));
    }

    // 13. Purchase Invoices
    query.exec(QStringLiteral("SELECT COUNT(*) FROM purchase_invoices;"));
    if (query.next() && query.value(0).toInt() == 0) {
        query.exec(QStringLiteral("INSERT INTO purchase_invoices (invoice_no, supplier_invoice_no, supplier_id, invoice_date, subtotal, tax_amount, total_amount, payment_status, notes) VALUES "
            "('PINV-2026-001', 'MW-99021', 1, datetime('now', '-3 days'), 1800.00, 90.00, 1890.00, 'Unpaid', 'Pallet delivery of dry groceries'),"
            "('PINV-2026-002', 'AF-4412', 2, datetime('now', '-1 day'), 520.00, 0.00, 520.00, 'Paid', 'Dairy and eggs morning delivery');"));
    }

    return true;
}

bool DatabaseManager::isConnected() const {
    return m_connected && m_db.isOpen();
}

QString DatabaseManager::lastError() const {
    return m_lastError;
}

QString DatabaseManager::databasePath() const {
    return m_dbPath;
}

QString DatabaseManager::currentUser() const {
    return m_currentUser;
}

QString DatabaseManager::currentRole() const {
    return m_currentRole;
}

RetailNext::BusinessType DatabaseManager::businessType() const {
    return m_businessType;
}

void DatabaseManager::setBusinessType(RetailNext::BusinessType type) {
    m_businessType = type;
    updateBusinessSetting(QStringLiteral("business_type"), RetailNext::businessTypeToString(type));
}

QString DatabaseManager::storeName() const {
    return getBusinessSetting(QStringLiteral("business_name"), m_storeName);
}

QString DatabaseManager::currencySymbol() const {
    return getBusinessSetting(QStringLiteral("currency_symbol"), QStringLiteral("$"));
}

QString DatabaseManager::taxMode() const {
    return getBusinessSetting(QStringLiteral("tax_mode"), QStringLiteral("Exclusive"));
}

QString DatabaseManager::branchName() const {
    return getBusinessSetting(QStringLiteral("branch_name"), QStringLiteral("Main Store (Flagship)"));
}

QString DatabaseManager::registerCode() const {
    return getBusinessSetting(QStringLiteral("register_code"), QStringLiteral("REG-01"));
}

bool DatabaseManager::updateBusinessSetting(const QString& key, const QString& val) {
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("INSERT INTO business_settings (key, value) VALUES (:k, :v) ON CONFLICT(key) DO UPDATE SET value = :v;"));
    query.bindValue(QStringLiteral(":k"), key);
    query.bindValue(QStringLiteral(":v"), val);
    return query.exec();
}

QString DatabaseManager::getBusinessSetting(const QString& key, const QString& defaultVal) const {
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT value FROM business_settings WHERE key = :k LIMIT 1;"));
    query.bindValue(QStringLiteral(":k"), key);
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return defaultVal;
}

RetailNext::DashboardSummary DatabaseManager::getDashboardSummary(const QString& period) {
    RetailNext::DashboardSummary s;
    QSqlQuery q(m_db);

    QString dateFilter = "date(created_at) = date('now')";
    if (period.compare("week", Qt::CaseInsensitive) == 0) {
        dateFilter = "date(created_at) >= date('now', '-7 days')";
    } else if (period.compare("month", Qt::CaseInsensitive) == 0) {
        dateFilter = "date(created_at) >= date('now', 'start of month')";
    } else if (period.compare("all", Qt::CaseInsensitive) == 0) {
        dateFilter = "1=1";
    }

    QString salesSql = QString("SELECT "
                               "  COALESCE(SUM(subtotal), 0.0), "
                               "  COALESCE(SUM(discount_amount), 0.0), "
                               "  COALESCE(SUM(net_payable), 0.0), "
                               "  COUNT(*), "
                               "  COALESCE(AVG(net_payable), 0.0), "
                               "  COALESCE(SUM(CASE WHEN payment_mode = 'Cash' THEN net_payable ELSE 0 END), 0.0), "
                               "  COALESCE(SUM(CASE WHEN payment_mode = 'Card' THEN net_payable ELSE 0 END), 0.0), "
                               "  COALESCE(SUM(CASE WHEN payment_mode = 'UPI' THEN net_payable ELSE 0 END), 0.0), "
                               "  COALESCE(SUM(CASE WHEN payment_mode = 'Credit' THEN net_payable ELSE 0 END), 0.0) "
                               "FROM sales_invoices WHERE %1;").arg(dateFilter);

    if (q.exec(salesSql) && q.next()) {
        s.grossSales = q.value(0).toDouble();
        s.totalDiscounts = q.value(1).toDouble();
        s.netSales = q.value(2).toDouble();
        s.transactionCount = q.value(3).toInt();
        s.avgBillValue = q.value(4).toDouble();
        s.cashSales = q.value(5).toDouble();
        s.cardSales = q.value(6).toDouble();
        s.upiSales = q.value(7).toDouble();
        s.creditSales = q.value(8).toDouble();
    }

    QString itemsSql = QString("SELECT "
                               "  COALESCE(SUM(sii.quantity), 0.0), "
                               "  COALESCE(SUM(sii.quantity * p.cost_price), 0.0) "
                               "FROM sales_invoice_items sii "
                               "JOIN sales_invoices si ON si.id = sii.invoice_id "
                               "JOIN products p ON p.id = sii.product_id "
                               "WHERE %1;").arg(dateFilter);

    if (q.exec(itemsSql) && q.next()) {
        s.itemsSold = q.value(0).toDouble();
        double costOfGoodsSold = q.value(1).toDouble();
        s.estimatedProfit = s.netSales - costOfGoodsSold;
        if (s.netSales > 0.0) {
            s.profitMargin = (s.estimatedProfit / s.netSales) * 100.0;
        }
    }

    if (q.exec(QStringLiteral("SELECT "
                              "  COALESCE(SUM(current_stock * cost_price), 0.0), "
                              "  COALESCE(SUM(CASE WHEN current_stock > 0 AND current_stock <= reorder_level THEN 1 ELSE 0 END), 0), "
                              "  COALESCE(SUM(CASE WHEN current_stock <= 0 THEN 1 ELSE 0 END), 0) "
                              "FROM products WHERE is_active = 1;")) && q.next()) {
        s.totalStockValue = q.value(0).toDouble();
        s.lowStockCount = q.value(1).toInt();
        s.outOfStockCount = q.value(2).toInt();
    }

    if (q.exec(QStringLiteral("SELECT COALESCE(SUM(outstanding_balance), 0.0) FROM customers;")) && q.next()) {
        s.receivables = q.value(0).toDouble();
    }

    if (q.exec(QStringLiteral("SELECT COALESCE(SUM(outstanding_balance), 0.0) FROM suppliers;")) && q.next()) {
        s.payables = q.value(0).toDouble();
    }

    return s;
}

QList<RetailNext::SaleInvoiceSummary> DatabaseManager::getRecentInvoices(int limit) {
    QList<RetailNext::SaleInvoiceSummary> list;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "SELECT id, invoice_no, created_at, cashier_name, customer_name, "
        "subtotal, discount_amount, tax_amount, round_off, net_payable, "
        "tendered_amount, change_due, payment_mode, payment_status, "
        "(SELECT COUNT(*) FROM sales_invoice_items WHERE invoice_id = sales_invoices.id) as item_cnt "
        "FROM sales_invoices ORDER BY id DESC LIMIT :lim;"
    ));
    q.bindValue(QStringLiteral(":lim"), limit);

    if (q.exec()) {
        while (q.next()) {
            RetailNext::SaleInvoiceSummary s;
            s.id = q.value(0).toInt();
            s.invoiceNo = q.value(1).toString();
            s.createdAt = q.value(2).toString();
            s.cashierName = q.value(3).toString();
            s.customerName = q.value(4).toString();
            s.subtotal = q.value(5).toDouble();
            s.discountAmount = q.value(6).toDouble();
            s.taxAmount = q.value(7).toDouble();
            s.roundOff = q.value(8).toDouble();
            s.netPayable = q.value(9).toDouble();
            s.tenderedAmount = q.value(10).toDouble();
            s.changeDue = q.value(11).toDouble();
            s.paymentMode = q.value(12).toString();
            s.paymentStatus = q.value(13).toString();
            s.itemCount = q.value(14).toInt();
            list.append(s);
        }
    }
    return list;
}

QList<RetailNext::ProductItem> DatabaseManager::getTopSellingProducts(int limit) {
    QList<RetailNext::ProductItem> list;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "SELECT p.id, p.sku, p.barcode, p.name, c.name, b.name, u.code, "
        "p.cost_price, p.selling_price, p.mrp, p.tax_rate, p.current_stock, "
        "COALESCE(SUM(sii.quantity), 0.0) as total_sold "
        "FROM products p "
        "LEFT JOIN sales_invoice_items sii ON sii.product_id = p.id "
        "LEFT JOIN categories c ON c.id = p.category_id "
        "LEFT JOIN brands b ON b.id = p.brand_id "
        "LEFT JOIN units u ON u.id = p.unit_id "
        "GROUP BY p.id "
        "ORDER BY total_sold DESC LIMIT :lim;"
    ));
    q.bindValue(QStringLiteral(":lim"), limit);

    if (q.exec()) {
        while (q.next()) {
            RetailNext::ProductItem p;
            p.id = q.value(0).toInt();
            p.sku = q.value(1).toString();
            p.barcode = q.value(2).toString();
            p.name = q.value(3).toString();
            p.categoryName = q.value(4).toString();
            p.brandName = q.value(5).toString();
            p.unitCode = q.value(6).toString();
            p.costPrice = q.value(7).toDouble();
            p.sellingPrice = q.value(8).toDouble();
            p.mrp = q.value(9).toDouble();
            p.taxRate = q.value(10).toDouble();
            p.currentStock = q.value(11).toDouble();
            list.append(p);
        }
    }
    return list;
}

QList<RetailNext::ProductItem> DatabaseManager::getLowStockAlerts(int limit) {
    QList<RetailNext::ProductItem> list;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "SELECT p.id, p.sku, p.barcode, p.name, c.name, b.name, u.code, "
        "p.cost_price, p.selling_price, p.mrp, p.tax_rate, p.current_stock, p.reorder_level "
        "FROM products p "
        "LEFT JOIN categories c ON c.id = p.category_id "
        "LEFT JOIN brands b ON b.id = p.brand_id "
        "LEFT JOIN units u ON u.id = p.unit_id "
        "WHERE p.is_active = 1 AND p.current_stock <= p.reorder_level "
        "ORDER BY p.current_stock ASC LIMIT :lim;"
    ));
    q.bindValue(QStringLiteral(":lim"), limit);

    if (q.exec()) {
        while (q.next()) {
            RetailNext::ProductItem p;
            p.id = q.value(0).toInt();
            p.sku = q.value(1).toString();
            p.barcode = q.value(2).toString();
            p.name = q.value(3).toString();
            p.categoryName = q.value(4).toString();
            p.brandName = q.value(5).toString();
            p.unitCode = q.value(6).toString();
            p.costPrice = q.value(7).toDouble();
            p.sellingPrice = q.value(8).toDouble();
            p.mrp = q.value(9).toDouble();
            p.taxRate = q.value(10).toDouble();
            p.currentStock = q.value(11).toDouble();
            p.reorderLevel = q.value(12).toDouble();
            list.append(p);
        }
    }
    return list;
}

QList<RetailNext::ProductItem> DatabaseManager::searchProducts(const QString& text, int categoryId, int stockFilter) {
    QList<RetailNext::ProductItem> list;
    QString sql = QStringLiteral(
        "SELECT p.id, p.sku, p.barcode, p.name, p.category_id, c.name, "
        "p.brand_id, b.name, p.unit_id, u.code, p.cost_price, p.selling_price, "
        "p.mrp, p.wholesale_price, p.tax_rate, p.current_stock, p.reorder_level, "
        "p.is_active, p.business_type_tag "
        "FROM products p "
        "LEFT JOIN categories c ON c.id = p.category_id "
        "LEFT JOIN brands b ON b.id = p.brand_id "
        "LEFT JOIN units u ON u.id = p.unit_id "
        "WHERE 1=1 "
    );

    if (!text.isEmpty()) {
        sql += QStringLiteral(" AND (p.name LIKE '%%1%' OR p.sku LIKE '%%1%' OR p.barcode LIKE '%%1%') ").arg(text);
    }
    if (categoryId > 0) {
        sql += QStringLiteral(" AND p.category_id = %1 ").arg(categoryId);
    }
    if (stockFilter == 1) {
        sql += QStringLiteral(" AND p.current_stock > 0 ");
    } else if (stockFilter == 2) {
        sql += QStringLiteral(" AND p.current_stock > 0 AND p.current_stock <= p.reorder_level ");
    } else if (stockFilter == 3) {
        sql += QStringLiteral(" AND p.current_stock <= 0 ");
    }

    sql += QStringLiteral(" ORDER BY p.id DESC;");

    QSqlQuery q(m_db);
    if (q.exec(sql)) {
        while (q.next()) {
            RetailNext::ProductItem p;
            p.id = q.value(0).toInt();
            p.sku = q.value(1).toString();
            p.barcode = q.value(2).toString();
            p.name = q.value(3).toString();
            p.categoryId = q.value(4).toInt();
            p.categoryName = q.value(5).toString();
            p.brandId = q.value(6).toInt();
            p.brandName = q.value(7).toString();
            p.unitId = q.value(8).toInt();
            p.unitCode = q.value(9).toString();
            p.costPrice = q.value(10).toDouble();
            p.sellingPrice = q.value(11).toDouble();
            p.mrp = q.value(12).toDouble();
            p.wholesalePrice = q.value(13).toDouble();
            p.taxRate = q.value(14).toDouble();
            p.currentStock = q.value(15).toDouble();
            p.reorderLevel = q.value(16).toDouble();
            p.isActive = q.value(17).toInt() == 1;
            p.businessTypeTag = q.value(18).toString();
            list.append(p);
        }
    }
    return list;
}

RetailNext::ProductItem DatabaseManager::getProductByBarcodeOrSku(const QString& code) {
    RetailNext::ProductItem p;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "SELECT p.id, p.sku, p.barcode, p.name, p.category_id, c.name, "
        "p.brand_id, b.name, p.unit_id, u.code, p.cost_price, p.selling_price, "
        "p.mrp, p.wholesale_price, p.tax_rate, p.current_stock, p.reorder_level, "
        "p.is_active, p.business_type_tag "
        "FROM products p "
        "LEFT JOIN categories c ON c.id = p.category_id "
        "LEFT JOIN brands b ON b.id = p.brand_id "
        "LEFT JOIN units u ON u.id = p.unit_id "
        "WHERE p.barcode = :code OR p.sku = :code LIMIT 1;"
    ));
    q.bindValue(QStringLiteral(":code"), code.trimmed());

    if (q.exec() && q.next()) {
        p.id = q.value(0).toInt();
        p.sku = q.value(1).toString();
        p.barcode = q.value(2).toString();
        p.name = q.value(3).toString();
        p.categoryId = q.value(4).toInt();
        p.categoryName = q.value(5).toString();
        p.brandId = q.value(6).toInt();
        p.brandName = q.value(7).toString();
        p.unitId = q.value(8).toInt();
        p.unitCode = q.value(9).toString();
        p.costPrice = q.value(10).toDouble();
        p.sellingPrice = q.value(11).toDouble();
        p.mrp = q.value(12).toDouble();
        p.wholesalePrice = q.value(13).toDouble();
        p.taxRate = q.value(14).toDouble();
        p.currentStock = q.value(15).toDouble();
        p.reorderLevel = q.value(16).toDouble();
        p.isActive = q.value(17).toInt() == 1;
        p.businessTypeTag = q.value(18).toString();
    }
    return p;
}

RetailNext::ProductItem DatabaseManager::getProductById(int id) {
    RetailNext::ProductItem p;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "SELECT p.id, p.sku, p.barcode, p.name, p.category_id, c.name, "
        "p.brand_id, b.name, p.unit_id, u.code, p.cost_price, p.selling_price, "
        "p.mrp, p.wholesale_price, p.tax_rate, p.current_stock, p.reorder_level, "
        "p.is_active, p.business_type_tag "
        "FROM products p "
        "LEFT JOIN categories c ON c.id = p.category_id "
        "LEFT JOIN brands b ON b.id = p.brand_id "
        "LEFT JOIN units u ON u.id = p.unit_id "
        "WHERE p.id = :id LIMIT 1;"
    ));
    q.bindValue(QStringLiteral(":id"), id);

    if (q.exec() && q.next()) {
        p.id = q.value(0).toInt();
        p.sku = q.value(1).toString();
        p.barcode = q.value(2).toString();
        p.name = q.value(3).toString();
        p.categoryId = q.value(4).toInt();
        p.categoryName = q.value(5).toString();
        p.brandId = q.value(6).toInt();
        p.brandName = q.value(7).toString();
        p.unitId = q.value(8).toInt();
        p.unitCode = q.value(9).toString();
        p.costPrice = q.value(10).toDouble();
        p.sellingPrice = q.value(11).toDouble();
        p.mrp = q.value(12).toDouble();
        p.wholesalePrice = q.value(13).toDouble();
        p.taxRate = q.value(14).toDouble();
        p.currentStock = q.value(15).toDouble();
        p.reorderLevel = q.value(16).toDouble();
        p.isActive = q.value(17).toInt() == 1;
        p.businessTypeTag = q.value(18).toString();
    }
    return p;
}

bool DatabaseManager::saveProduct(const RetailNext::ProductItem& prod, bool isNew) {
    QSqlQuery q(m_db);
    if (isNew) {
        q.prepare(QStringLiteral(
            "INSERT INTO products (sku, barcode, name, category_id, brand_id, unit_id, "
            "cost_price, selling_price, mrp, wholesale_price, tax_rate, current_stock, "
            "reorder_level, is_active, business_type_tag) "
            "VALUES (:sku, :bar, :name, :cat, :brand, :unit, :cost, :sell, :mrp, :ws, :tax, :stock, :reorder, :act, :btype);"
        ));
    } else {
        q.prepare(QStringLiteral(
            "UPDATE products SET sku = :sku, barcode = :bar, name = :name, category_id = :cat, "
            "brand_id = :brand, unit_id = :unit, cost_price = :cost, selling_price = :sell, "
            "mrp = :mrp, wholesale_price = :ws, tax_rate = :tax, current_stock = :stock, "
            "reorder_level = :reorder, is_active = :act, business_type_tag = :btype "
            "WHERE id = :id;"
        ));
        q.bindValue(QStringLiteral(":id"), prod.id);
    }

    q.bindValue(QStringLiteral(":sku"), prod.sku);
    q.bindValue(QStringLiteral(":bar"), prod.barcode);
    q.bindValue(QStringLiteral(":name"), prod.name);
    q.bindValue(QStringLiteral(":cat"), prod.categoryId);
    q.bindValue(QStringLiteral(":brand"), prod.brandId);
    q.bindValue(QStringLiteral(":unit"), prod.unitId);
    q.bindValue(QStringLiteral(":cost"), prod.costPrice);
    q.bindValue(QStringLiteral(":sell"), prod.sellingPrice);
    q.bindValue(QStringLiteral(":mrp"), prod.mrp);
    q.bindValue(QStringLiteral(":ws"), prod.wholesalePrice);
    q.bindValue(QStringLiteral(":tax"), prod.taxRate);
    q.bindValue(QStringLiteral(":stock"), prod.currentStock);
    q.bindValue(QStringLiteral(":reorder"), prod.reorderLevel);
    q.bindValue(QStringLiteral(":act"), prod.isActive ? 1 : 0);
    q.bindValue(QStringLiteral(":btype"), prod.businessTypeTag);

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }

    if (isNew && prod.currentStock > 0.0) {
        int newId = q.lastInsertId().toInt();
        QSqlQuery smQuery(m_db);
        smQuery.prepare(QStringLiteral(
            "INSERT INTO stock_movements (product_id, reference_no, movement_type, quantity, unit_cost, balance_after, user_name, notes) "
            "VALUES (:pid, :ref, 'Opening Stock', :qty, :cost, :bal, :user, 'Initial product entry');"
        ));
        smQuery.bindValue(QStringLiteral(":pid"), newId);
        smQuery.bindValue(QStringLiteral(":ref"), QStringLiteral("INIT-%1").arg(newId));
        smQuery.bindValue(QStringLiteral(":qty"), prod.currentStock);
        smQuery.bindValue(QStringLiteral(":cost"), prod.costPrice);
        smQuery.bindValue(QStringLiteral(":bal"), prod.currentStock);
        smQuery.bindValue(QStringLiteral(":user"), m_currentUser);
        smQuery.exec();
    }

    return true;
}

bool DatabaseManager::deleteProduct(int id) {
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("UPDATE products SET is_active = 0 WHERE id = :id;"));
    q.bindValue(QStringLiteral(":id"), id);
    return q.exec();
}

bool DatabaseManager::bulkUpdatePrices(int categoryId, double percentChange) {
    QSqlQuery q(m_db);
    double multiplier = 1.0 + (percentChange / 100.0);
    if (categoryId > 0) {
        q.prepare(QStringLiteral("UPDATE products SET selling_price = ROUND(selling_price * :m, 2) WHERE category_id = :cat;"));
        q.bindValue(QStringLiteral(":cat"), categoryId);
    } else {
        q.prepare(QStringLiteral("UPDATE products SET selling_price = ROUND(selling_price * :m, 2);"));
    }
    q.bindValue(QStringLiteral(":m"), multiplier);
    return q.exec();
}

QStringList DatabaseManager::getCategoriesList() {
    QStringList res;
    QSqlQuery q(QStringLiteral("SELECT name FROM categories WHERE is_active = 1 ORDER BY name;"), m_db);
    while (q.next()) {
        res.append(q.value(0).toString());
    }
    return res;
}

QStringList DatabaseManager::getBrandsList() {
    QStringList res;
    QSqlQuery q(QStringLiteral("SELECT name FROM brands WHERE is_active = 1 ORDER BY name;"), m_db);
    while (q.next()) {
        res.append(q.value(0).toString());
    }
    return res;
}

QStringList DatabaseManager::getUnitsList() {
    QStringList res;
    QSqlQuery q(QStringLiteral("SELECT code FROM units ORDER BY code;"), m_db);
    while (q.next()) {
        res.append(q.value(0).toString());
    }
    return res;
}

RetailNext::POSSaleResult DatabaseManager::processPOSSale(
    const QString& customerName,
    int customerId,
    const QList<RetailNext::CartItem>& items,
    double subtotal,
    double discount,
    double tax,
    double roundOff,
    double netPayable,
    double tendered,
    double change,
    const QString& paymentMode,
    const QString& notes
) {
    RetailNext::POSSaleResult res;
    if (items.isEmpty()) {
        res.success = false;
        res.errorMessage = QStringLiteral("Cart is empty.");
        return res;
    }

    if (!m_db.transaction()) {
        res.success = false;
        res.errorMessage = QStringLiteral("Failed to begin transaction: ") + m_db.lastError().text();
        return res;
    }

    QString todayCode = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd"));
    QSqlQuery countQ(m_db);
    countQ.exec(QStringLiteral("SELECT COUNT(*) FROM sales_invoices;"));
    int currentCount = 1;
    if (countQ.next()) {
        currentCount = countQ.value(0).toInt() + 1;
    }
    QString invoiceNo = QStringLiteral("INV-%1-%2").arg(todayCode).arg(currentCount, 4, 10, QChar('0'));
    res.invoiceNo = invoiceNo;

    // 1. Insert into sales_invoices
    QSqlQuery invQ(m_db);
    invQ.prepare(QStringLiteral(
        "INSERT INTO sales_invoices (invoice_no, branch_id, register_id, cashier_name, customer_id, customer_name, "
        "subtotal, discount_amount, tax_amount, round_off, net_payable, tendered_amount, change_due, "
        "payment_mode, payment_status, notes) "
        "VALUES (:inv, 1, 1, :cashier, :cid, :cname, :sub, :disc, :tax, :ro, :net, :tender, :change, :pmode, :pstatus, :notes);"
    ));
    invQ.bindValue(QStringLiteral(":inv"), invoiceNo);
    invQ.bindValue(QStringLiteral(":cashier"), m_currentUser);
    invQ.bindValue(QStringLiteral(":cid"), customerId);
    invQ.bindValue(QStringLiteral(":cname"), customerName);
    invQ.bindValue(QStringLiteral(":sub"), subtotal);
    invQ.bindValue(QStringLiteral(":disc"), discount);
    invQ.bindValue(QStringLiteral(":tax"), tax);
    invQ.bindValue(QStringLiteral(":ro"), roundOff);
    invQ.bindValue(QStringLiteral(":net"), netPayable);
    invQ.bindValue(QStringLiteral(":tender"), tendered);
    invQ.bindValue(QStringLiteral(":change"), change);
    invQ.bindValue(QStringLiteral(":pmode"), paymentMode);
    invQ.bindValue(QStringLiteral(":pstatus"), paymentMode == "Credit" ? QStringLiteral("Unpaid") : QStringLiteral("Paid"));
    invQ.bindValue(QStringLiteral(":notes"), notes);

    if (!invQ.exec()) {
        m_db.rollback();
        res.success = false;
        res.errorMessage = QStringLiteral("Failed to create sales invoice: ") + invQ.lastError().text();
        return res;
    }

    int invoiceId = invQ.lastInsertId().toInt();

    // 2. Insert items, deduct stock, write stock movement ledger
    for (const auto& item : items) {
        QSqlQuery itemQ(m_db);
        itemQ.prepare(QStringLiteral(
            "INSERT INTO sales_invoice_items (invoice_id, product_id, variant_id, barcode, item_name, quantity, "
            "unit_price, discount_amount, tax_rate, tax_amount, line_total) "
            "VALUES (:invid, :pid, :vid, :bar, :name, :qty, :price, :disc, :taxr, :taxa, :tot);"
        ));
        itemQ.bindValue(QStringLiteral(":invid"), invoiceId);
        itemQ.bindValue(QStringLiteral(":pid"), item.productId);
        itemQ.bindValue(QStringLiteral(":vid"), item.variantId);
        itemQ.bindValue(QStringLiteral(":bar"), item.barcode);
        itemQ.bindValue(QStringLiteral(":name"), item.name);
        itemQ.bindValue(QStringLiteral(":qty"), item.qty);
        itemQ.bindValue(QStringLiteral(":price"), item.unitPrice);
        itemQ.bindValue(QStringLiteral(":disc"), item.discountAmount);
        itemQ.bindValue(QStringLiteral(":taxr"), item.taxRate);
        itemQ.bindValue(QStringLiteral(":taxa"), item.taxAmount);
        itemQ.bindValue(QStringLiteral(":tot"), item.lineTotal);

        if (!itemQ.exec()) {
            m_db.rollback();
            res.success = false;
            res.errorMessage = QStringLiteral("Failed saving invoice item: ") + itemQ.lastError().text();
            return res;
        }

        QSqlQuery stockQ(m_db);
        stockQ.prepare(QStringLiteral("UPDATE products SET current_stock = current_stock - :qty WHERE id = :pid;"));
        stockQ.bindValue(QStringLiteral(":qty"), item.qty);
        stockQ.bindValue(QStringLiteral(":pid"), item.productId);
        stockQ.exec();

        double balanceAfter = 0.0;
        double cost = 0.0;
        QSqlQuery balQ(m_db);
        balQ.prepare(QStringLiteral("SELECT current_stock, cost_price FROM products WHERE id = :pid;"));
        balQ.bindValue(QStringLiteral(":pid"), item.productId);
        if (balQ.exec() && balQ.next()) {
            balanceAfter = balQ.value(0).toDouble();
            cost = balQ.value(1).toDouble();
        }

        QSqlQuery smQ(m_db);
        smQ.prepare(QStringLiteral(
            "INSERT INTO stock_movements (product_id, reference_no, movement_type, quantity, unit_cost, balance_after, user_name, notes) "
            "VALUES (:pid, :ref, 'POS Sale', :qty, :cost, :bal, :user, :notes);"
        ));
        smQ.bindValue(QStringLiteral(":pid"), item.productId);
        smQ.bindValue(QStringLiteral(":ref"), invoiceNo);
        smQ.bindValue(QStringLiteral(":qty"), -item.qty);
        smQ.bindValue(QStringLiteral(":cost"), cost);
        smQ.bindValue(QStringLiteral(":bal"), balanceAfter);
        smQ.bindValue(QStringLiteral(":user"), m_currentUser);
        smQ.bindValue(QStringLiteral(":notes"), QStringLiteral("Sale at Register #1"));
        smQ.exec();
    }

    // 3. Update Cash Register Session
    if (paymentMode == "Cash") {
        QSqlQuery crQ(m_db);
        crQ.prepare(QStringLiteral(
            "UPDATE cash_register_sessions SET cash_sales = cash_sales + :net, "
            "expected_cash = opening_cash + cash_sales + cash_in - cash_out "
            "WHERE status = 'Open' ORDER BY id DESC LIMIT 1;"
        ));
        crQ.bindValue(QStringLiteral(":net"), netPayable);
        crQ.exec();
    }

    // 4. Update Customer points or credit balance
    if (customerId > 1) {
        if (paymentMode == "Credit") {
            QSqlQuery custBalQ(m_db);
            custBalQ.prepare(QStringLiteral("UPDATE customers SET outstanding_balance = outstanding_balance + :net WHERE id = :cid;"));
            custBalQ.bindValue(QStringLiteral(":net"), netPayable);
            custBalQ.bindValue(QStringLiteral(":cid"), customerId);
            custBalQ.exec();
        } else {
            int earnedPoints = static_cast<int>(netPayable / 10.0);
            QSqlQuery ptsQ(m_db);
            ptsQ.prepare(QStringLiteral("UPDATE customers SET loyalty_points = loyalty_points + :pts WHERE id = :cid;"));
            ptsQ.bindValue(QStringLiteral(":pts"), earnedPoints);
            ptsQ.bindValue(QStringLiteral(":cid"), customerId);
            ptsQ.exec();
        }
    }

    if (!m_db.commit()) {
        m_db.rollback();
        res.success = false;
        res.errorMessage = QStringLiteral("Failed to commit sale transaction.");
        return res;
    }

    res.success = true;
    return res;
}

bool DatabaseManager::holdBill(const QString& customerName, const QList<RetailNext::CartItem>& items, double total) {
    if (items.isEmpty()) return false;

    QJsonArray arr;
    for (const auto& item : items) {
        QJsonObject obj;
        obj["productId"] = item.productId;
        obj["variantId"] = item.variantId;
        obj["barcode"] = item.barcode;
        obj["name"] = item.name;
        obj["variantName"] = item.variantName;
        obj["qty"] = item.qty;
        obj["unit"] = item.unit;
        obj["unitPrice"] = item.unitPrice;
        obj["discountPercent"] = item.discountPercent;
        obj["discountAmount"] = item.discountAmount;
        obj["taxRate"] = item.taxRate;
        obj["taxAmount"] = item.taxAmount;
        obj["lineTotal"] = item.lineTotal;
        arr.append(obj);
    }

    QJsonDocument doc(arr);
    QString jsonStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("INSERT INTO held_bills (customer_name, cart_json, total_amount) VALUES (:cname, :json, :tot);"));
    q.bindValue(QStringLiteral(":cname"), customerName.isEmpty() ? QStringLiteral("Walk-in Customer") : customerName);
    q.bindValue(QStringLiteral(":json"), jsonStr);
    q.bindValue(QStringLiteral(":tot"), total);
    return q.exec();
}

QList<QPair<int, QString>> DatabaseManager::getHeldBills() {
    QList<QPair<int, QString>> res;
    QSqlQuery q(QStringLiteral("SELECT id, customer_name, total_amount, created_at FROM held_bills ORDER BY id DESC;"), m_db);
    while (q.next()) {
        int id = q.value(0).toInt();
        QString desc = QStringLiteral("#%1: %2 ($%3) - %4")
            .arg(id)
            .arg(q.value(1).toString())
            .arg(q.value(2).toDouble(), 0, 'f', 2)
            .arg(q.value(3).toString());
        res.append(qMakePair(id, desc));
    }
    return res;
}

QList<RetailNext::CartItem> DatabaseManager::recallHeldBill(int holdId) {
    QList<RetailNext::CartItem> items;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT cart_json FROM held_bills WHERE id = :id LIMIT 1;"));
    q.bindValue(QStringLiteral(":id"), holdId);

    if (q.exec() && q.next()) {
        QByteArray jsonBytes = q.value(0).toString().toUtf8();
        QJsonDocument doc = QJsonDocument::fromJson(jsonBytes);
        if (doc.isArray()) {
            QJsonArray arr = doc.array();
            for (const auto& val : arr) {
                QJsonObject obj = val.toObject();
                RetailNext::CartItem item;
                item.productId = obj["productId"].toInt();
                item.variantId = obj["variantId"].toInt();
                item.barcode = obj["barcode"].toString();
                item.name = obj["name"].toString();
                item.variantName = obj["variantName"].toString();
                item.qty = obj["qty"].toDouble();
                item.unit = obj["unit"].toString();
                item.unitPrice = obj["unitPrice"].toDouble();
                item.discountPercent = obj["discountPercent"].toDouble();
                item.discountAmount = obj["discountAmount"].toDouble();
                item.taxRate = obj["taxRate"].toDouble();
                item.taxAmount = obj["taxAmount"].toDouble();
                item.lineTotal = obj["lineTotal"].toDouble();
                items.append(item);
            }
        }
    }
    return items;
}

bool DatabaseManager::deleteHeldBill(int holdId) {
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("DELETE FROM held_bills WHERE id = :id;"));
    q.bindValue(QStringLiteral(":id"), holdId);
    return q.exec();
}

QList<RetailNext::SaleInvoiceSummary> DatabaseManager::searchInvoices(const QString& query, const QString& dateFrom, const QString& dateTo) {
    QList<RetailNext::SaleInvoiceSummary> list;
    QString sql = QStringLiteral(
        "SELECT id, invoice_no, created_at, cashier_name, customer_name, "
        "subtotal, discount_amount, tax_amount, round_off, net_payable, "
        "tendered_amount, change_due, payment_mode, payment_status, "
        "(SELECT COUNT(*) FROM sales_invoice_items WHERE invoice_id = sales_invoices.id) as item_cnt "
        "FROM sales_invoices WHERE 1=1 "
    );

    if (!query.isEmpty()) {
        sql += QStringLiteral(" AND (invoice_no LIKE '%%1%' OR customer_name LIKE '%%1%') ").arg(query);
    }
    if (!dateFrom.isEmpty()) {
        sql += QStringLiteral(" AND date(created_at) >= '%1' ").arg(dateFrom);
    }
    if (!dateTo.isEmpty()) {
        sql += QStringLiteral(" AND date(created_at) <= '%1' ").arg(dateTo);
    }

    sql += QStringLiteral(" ORDER BY id DESC;");

    QSqlQuery q(m_db);
    if (q.exec(sql)) {
        while (q.next()) {
            RetailNext::SaleInvoiceSummary s;
            s.id = q.value(0).toInt();
            s.invoiceNo = q.value(1).toString();
            s.createdAt = q.value(2).toString();
            s.cashierName = q.value(3).toString();
            s.customerName = q.value(4).toString();
            s.subtotal = q.value(5).toDouble();
            s.discountAmount = q.value(6).toDouble();
            s.taxAmount = q.value(7).toDouble();
            s.roundOff = q.value(8).toDouble();
            s.netPayable = q.value(9).toDouble();
            s.tenderedAmount = q.value(10).toDouble();
            s.changeDue = q.value(11).toDouble();
            s.paymentMode = q.value(12).toString();
            s.paymentStatus = q.value(13).toString();
            s.itemCount = q.value(14).toInt();
            list.append(s);
        }
    }
    return list;
}

QList<RetailNext::CartItem> DatabaseManager::getInvoiceItems(int invoiceId) {
    QList<RetailNext::CartItem> list;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "SELECT product_id, variant_id, barcode, item_name, quantity, "
        "unit_price, discount_amount, tax_rate, tax_amount, line_total "
        "FROM sales_invoice_items WHERE invoice_id = :id;"
    ));
    q.bindValue(QStringLiteral(":id"), invoiceId);

    if (q.exec()) {
        while (q.next()) {
            RetailNext::CartItem item;
            item.productId = q.value(0).toInt();
            item.variantId = q.value(1).toInt();
            item.barcode = q.value(2).toString();
            item.name = q.value(3).toString();
            item.qty = q.value(4).toDouble();
            item.unitPrice = q.value(5).toDouble();
            item.discountAmount = q.value(6).toDouble();
            item.taxRate = q.value(7).toDouble();
            item.taxAmount = q.value(8).toDouble();
            item.lineTotal = q.value(9).toDouble();
            list.append(item);
        }
    }
    return list;
}

bool DatabaseManager::processSalesReturn(int invoiceId, const QString& invoiceNo, int customerId, double refundAmount, const QString& reason, const QString& refundMode) {
    if (!m_db.transaction()) return false;

    QString returnNo = QStringLiteral("RET-%1-%2").arg(QDateTime::currentDateTime().toString("yyyyMMdd")).arg(invoiceId);

    QSqlQuery retQ(m_db);
    retQ.prepare(QStringLiteral(
        "INSERT INTO sales_returns (return_no, original_invoice_no, customer_id, refund_amount, refund_method, reason) "
        "VALUES (:rno, :inv, :cid, :amt, :mode, :reason);"
    ));
    retQ.bindValue(QStringLiteral(":rno"), returnNo);
    retQ.bindValue(QStringLiteral(":inv"), invoiceNo);
    retQ.bindValue(QStringLiteral(":cid"), customerId);
    retQ.bindValue(QStringLiteral(":amt"), refundAmount);
    retQ.bindValue(QStringLiteral(":mode"), refundMode);
    retQ.bindValue(QStringLiteral(":reason"), reason);

    if (!retQ.exec()) {
        m_db.rollback();
        return false;
    }

    QSqlQuery updInv(m_db);
    updInv.prepare(QStringLiteral("UPDATE sales_invoices SET payment_status = 'Returned' WHERE id = :id;"));
    updInv.bindValue(QStringLiteral(":id"), invoiceId);
    updInv.exec();

    auto items = getInvoiceItems(invoiceId);
    for (const auto& item : items) {
        QSqlQuery stockQ(m_db);
        stockQ.prepare(QStringLiteral("UPDATE products SET current_stock = current_stock + :qty WHERE id = :pid;"));
        stockQ.bindValue(QStringLiteral(":qty"), item.qty);
        stockQ.bindValue(QStringLiteral(":pid"), item.productId);
        stockQ.exec();

        QSqlQuery smQ(m_db);
        smQ.prepare(QStringLiteral(
            "INSERT INTO stock_movements (product_id, reference_no, movement_type, quantity, unit_cost, balance_after, user_name, notes) "
            "VALUES (:pid, :ref, 'Sales Return', :qty, :cost, (SELECT current_stock FROM products WHERE id = :pid), :user, :notes);"
        ));
        smQ.bindValue(QStringLiteral(":pid"), item.productId);
        smQ.bindValue(QStringLiteral(":ref"), returnNo);
        smQ.bindValue(QStringLiteral(":qty"), item.qty);
        smQ.bindValue(QStringLiteral(":cost"), item.unitPrice);
        smQ.bindValue(QStringLiteral(":user"), m_currentUser);
        smQ.bindValue(QStringLiteral(":notes"), QStringLiteral("Return for Inv %1: %2").arg(invoiceNo, reason));
        smQ.exec();
    }

    return m_db.commit();
}

QList<RetailNext::StockMovementRecord> DatabaseManager::getStockMovements(int limit, int productId) {
    QList<RetailNext::StockMovementRecord> list;
    QString sql = QStringLiteral(
        "SELECT sm.id, sm.product_id, p.name, p.sku, sm.reference_no, "
        "sm.movement_type, sm.quantity, sm.unit_cost, sm.balance_after, sm.user_name, sm.notes, sm.created_at "
        "FROM stock_movements sm "
        "LEFT JOIN products p ON p.id = sm.product_id "
        "WHERE 1=1 "
    );

    if (productId > 0) {
        sql += QStringLiteral(" AND sm.product_id = %1 ").arg(productId);
    }
    sql += QStringLiteral(" ORDER BY sm.id DESC LIMIT %1;").arg(limit);

    QSqlQuery q(m_db);
    if (q.exec(sql)) {
        while (q.next()) {
            RetailNext::StockMovementRecord r;
            r.id = q.value(0).toInt();
            r.productId = q.value(1).toInt();
            r.productName = q.value(2).toString();
            r.sku = q.value(3).toString();
            r.referenceNo = q.value(4).toString();
            r.movementType = q.value(5).toString();
            r.quantity = q.value(6).toDouble();
            r.unitCost = q.value(7).toDouble();
            r.balanceAfter = q.value(8).toDouble();
            r.userName = q.value(9).toString();
            r.notes = q.value(10).toString();
            r.createdAt = q.value(11).toString();
            list.append(r);
        }
    }
    return list;
}

bool DatabaseManager::recordStockAdjustment(int productId, double countedQty, const QString& reason, const QString& notes) {
    if (!m_db.transaction()) return false;

    double currentStock = 0.0;
    double cost = 0.0;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT current_stock, cost_price FROM products WHERE id = :id;"));
    q.bindValue(QStringLiteral(":id"), productId);
    if (q.exec() && q.next()) {
        currentStock = q.value(0).toDouble();
        cost = q.value(1).toDouble();
    } else {
        m_db.rollback();
        return false;
    }

    double diff = countedQty - currentStock;

    QSqlQuery upd(m_db);
    upd.prepare(QStringLiteral("UPDATE products SET current_stock = :newStock WHERE id = :id;"));
    upd.bindValue(QStringLiteral(":newStock"), countedQty);
    upd.bindValue(QStringLiteral(":id"), productId);
    if (!upd.exec()) {
        m_db.rollback();
        return false;
    }

    QString ref = QStringLiteral("ADJ-%1-%2").arg(QDateTime::currentDateTime().toString("yyyyMMdd")).arg(productId);
    QSqlQuery sm(m_db);
    sm.prepare(QStringLiteral(
        "INSERT INTO stock_movements (product_id, reference_no, movement_type, quantity, unit_cost, balance_after, user_name, notes) "
        "VALUES (:pid, :ref, 'Stock Adjustment', :qty, :cost, :bal, :user, :notes);"
    ));
    sm.bindValue(QStringLiteral(":pid"), productId);
    sm.bindValue(QStringLiteral(":ref"), ref);
    sm.bindValue(QStringLiteral(":qty"), diff);
    sm.bindValue(QStringLiteral(":cost"), cost);
    sm.bindValue(QStringLiteral(":bal"), countedQty);
    sm.bindValue(QStringLiteral(":user"), m_currentUser);
    sm.bindValue(QStringLiteral(":notes"), QStringLiteral("%1: %2").arg(reason, notes));
    if (!sm.exec()) {
        m_db.rollback();
        return false;
    }

    return m_db.commit();
}

bool DatabaseManager::recordStockTransfer(int productId, const QString& fromBranch, const QString& toBranch, double qty, const QString& notes) {
    if (!m_db.transaction()) return false;

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("UPDATE products SET current_stock = current_stock - :qty WHERE id = :id;"));
    q.bindValue(QStringLiteral(":qty"), qty);
    q.bindValue(QStringLiteral(":id"), productId);
    if (!q.exec()) {
        m_db.rollback();
        return false;
    }

    QString ref = QStringLiteral("TRF-%1-%2").arg(QDateTime::currentDateTime().toString("yyyyMMdd")).arg(productId);
    QSqlQuery sm(m_db);
    sm.prepare(QStringLiteral(
        "INSERT INTO stock_movements (product_id, reference_no, movement_type, quantity, unit_cost, balance_after, user_name, notes) "
        "VALUES (:pid, :ref, 'Branch Transfer', :qty, 0.0, (SELECT current_stock FROM products WHERE id = :pid), :user, :notes);"
    ));
    sm.bindValue(QStringLiteral(":pid"), productId);
    sm.bindValue(QStringLiteral(":ref"), ref);
    sm.bindValue(QStringLiteral(":qty"), -qty);
    sm.bindValue(QStringLiteral(":user"), m_currentUser);
    sm.bindValue(QStringLiteral(":notes"), QStringLiteral("Transfer from %1 to %2. %3").arg(fromBranch, toBranch, notes));
    sm.exec();

    return m_db.commit();
}

QList<RetailNext::CustomerRecord> DatabaseManager::searchCustomers(const QString& query, const QString& group) {
    QList<RetailNext::CustomerRecord> list;
    QString sql = QStringLiteral("SELECT id, code, name, phone, email, address, gst_no, customer_group, loyalty_points, store_credit, outstanding_balance, credit_limit, is_active FROM customers WHERE 1=1 ");

    if (!query.isEmpty()) {
        sql += QStringLiteral(" AND (name LIKE '%%1%' OR phone LIKE '%%1%' OR code LIKE '%%1%') ").arg(query);
    }
    if (!group.isEmpty() && group != "All Groups") {
        sql += QStringLiteral(" AND customer_group = '%1' ").arg(group);
    }
    sql += QStringLiteral(" ORDER BY name ASC;");

    QSqlQuery q(m_db);
    if (q.exec(sql)) {
        while (q.next()) {
            RetailNext::CustomerRecord c;
            c.id = q.value(0).toInt();
            c.code = q.value(1).toString();
            c.name = q.value(2).toString();
            c.phone = q.value(3).toString();
            c.email = q.value(4).toString();
            c.address = q.value(5).toString();
            c.gstNo = q.value(6).toString();
            c.customerGroup = q.value(7).toString();
            c.loyaltyPoints = q.value(8).toInt();
            c.storeCredit = q.value(9).toDouble();
            c.outstandingBalance = q.value(10).toDouble();
            c.creditLimit = q.value(11).toDouble();
            c.isActive = q.value(12).toInt() == 1;
            list.append(c);
        }
    }
    return list;
}

RetailNext::CustomerRecord DatabaseManager::getCustomerById(int id) {
    RetailNext::CustomerRecord c;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT id, code, name, phone, email, address, gst_no, customer_group, loyalty_points, store_credit, outstanding_balance, credit_limit, is_active FROM customers WHERE id = :id LIMIT 1;"));
    q.bindValue(QStringLiteral(":id"), id);
    if (q.exec() && q.next()) {
        c.id = q.value(0).toInt();
        c.code = q.value(1).toString();
        c.name = q.value(2).toString();
        c.phone = q.value(3).toString();
        c.email = q.value(4).toString();
        c.address = q.value(5).toString();
        c.gstNo = q.value(6).toString();
        c.customerGroup = q.value(7).toString();
        c.loyaltyPoints = q.value(8).toInt();
        c.storeCredit = q.value(9).toDouble();
        c.outstandingBalance = q.value(10).toDouble();
        c.creditLimit = q.value(11).toDouble();
        c.isActive = q.value(12).toInt() == 1;
    }
    return c;
}

bool DatabaseManager::saveCustomer(const RetailNext::CustomerRecord& cust, bool isNew) {
    QSqlQuery q(m_db);
    if (isNew) {
        q.prepare(QStringLiteral(
            "INSERT INTO customers (code, name, phone, email, address, gst_no, customer_group, loyalty_points, store_credit, outstanding_balance, credit_limit, is_active) "
            "VALUES (:code, :name, :phone, :email, :addr, :gst, :grp, :pts, :sc, :bal, :clim, :act);"
        ));
    } else {
        q.prepare(QStringLiteral(
            "UPDATE customers SET code = :code, name = :name, phone = :phone, email = :email, address = :addr, "
            "gst_no = :gst, customer_group = :grp, loyalty_points = :pts, store_credit = :sc, "
            "outstanding_balance = :bal, credit_limit = :clim, is_active = :act WHERE id = :id;"
        ));
        q.bindValue(QStringLiteral(":id"), cust.id);
    }

    q.bindValue(QStringLiteral(":code"), cust.code);
    q.bindValue(QStringLiteral(":name"), cust.name);
    q.bindValue(QStringLiteral(":phone"), cust.phone);
    q.bindValue(QStringLiteral(":email"), cust.email);
    q.bindValue(QStringLiteral(":addr"), cust.address);
    q.bindValue(QStringLiteral(":gst"), cust.gstNo);
    q.bindValue(QStringLiteral(":grp"), cust.customerGroup);
    q.bindValue(QStringLiteral(":pts"), cust.loyaltyPoints);
    q.bindValue(QStringLiteral(":sc"), cust.storeCredit);
    q.bindValue(QStringLiteral(":bal"), cust.outstandingBalance);
    q.bindValue(QStringLiteral(":clim"), cust.creditLimit);
    q.bindValue(QStringLiteral(":act"), cust.isActive ? 1 : 0);

    return q.exec();
}

bool DatabaseManager::recordCustomerPayment(int customerId, double amount, const QString& paymentMode, const QString& notes) {
    if (!m_db.transaction()) return false;

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("UPDATE customers SET outstanding_balance = MAX(0.0, outstanding_balance - :amt) WHERE id = :id;"));
    q.bindValue(QStringLiteral(":amt"), amount);
    q.bindValue(QStringLiteral(":id"), customerId);
    if (!q.exec()) {
        m_db.rollback();
        return false;
    }

    if (paymentMode == "Cash") {
        QSqlQuery crQ(m_db);
        crQ.prepare(QStringLiteral(
            "UPDATE cash_register_sessions SET cash_in = cash_in + :amt, "
            "expected_cash = opening_cash + cash_sales + cash_in - cash_out "
            "WHERE status = 'Open' ORDER BY id DESC LIMIT 1;"
        ));
        crQ.bindValue(QStringLiteral(":amt"), amount);
        crQ.exec();
    }

    return m_db.commit();
}

QList<RetailNext::SupplierRecord> DatabaseManager::searchSuppliers(const QString& query) {
    QList<RetailNext::SupplierRecord> list;
    QString sql = QStringLiteral("SELECT id, code, name, contact_person, phone, email, address, gst_no, payment_terms, credit_limit, outstanding_balance, is_active FROM suppliers WHERE 1=1 ");

    if (!query.isEmpty()) {
        sql += QStringLiteral(" AND (name LIKE '%%1%' OR contact_person LIKE '%%1%' OR code LIKE '%%1%') ").arg(query);
    }
    sql += QStringLiteral(" ORDER BY name ASC;");

    QSqlQuery q(m_db);
    if (q.exec(sql)) {
        while (q.next()) {
            RetailNext::SupplierRecord s;
            s.id = q.value(0).toInt();
            s.code = q.value(1).toString();
            s.name = q.value(2).toString();
            s.contactPerson = q.value(3).toString();
            s.phone = q.value(4).toString();
            s.email = q.value(5).toString();
            s.address = q.value(6).toString();
            s.gstNo = q.value(7).toString();
            s.paymentTerms = q.value(8).toString();
            s.creditLimit = q.value(9).toDouble();
            s.outstandingBalance = q.value(10).toDouble();
            s.isActive = q.value(11).toInt() == 1;
            list.append(s);
        }
    }
    return list;
}

RetailNext::SupplierRecord DatabaseManager::getSupplierById(int id) {
    RetailNext::SupplierRecord s;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT id, code, name, contact_person, phone, email, address, gst_no, payment_terms, credit_limit, outstanding_balance, is_active FROM suppliers WHERE id = :id LIMIT 1;"));
    q.bindValue(QStringLiteral(":id"), id);
    if (q.exec() && q.next()) {
        s.id = q.value(0).toInt();
        s.code = q.value(1).toString();
        s.name = q.value(2).toString();
        s.contactPerson = q.value(3).toString();
        s.phone = q.value(4).toString();
        s.email = q.value(5).toString();
        s.address = q.value(6).toString();
        s.gstNo = q.value(7).toString();
        s.paymentTerms = q.value(8).toString();
        s.creditLimit = q.value(9).toDouble();
        s.outstandingBalance = q.value(10).toDouble();
        s.isActive = q.value(11).toInt() == 1;
    }
    return s;
}

bool DatabaseManager::saveSupplier(const RetailNext::SupplierRecord& supp, bool isNew) {
    QSqlQuery q(m_db);
    if (isNew) {
        q.prepare(QStringLiteral(
            "INSERT INTO suppliers (code, name, contact_person, phone, email, address, gst_no, payment_terms, credit_limit, outstanding_balance, is_active) "
            "VALUES (:code, :name, :cp, :phone, :email, :addr, :gst, :pt, :clim, :bal, :act);"
        ));
    } else {
        q.prepare(QStringLiteral(
            "UPDATE suppliers SET code = :code, name = :name, contact_person = :cp, phone = :phone, email = :email, "
            "address = :addr, gst_no = :gst, payment_terms = :pt, credit_limit = :clim, outstanding_balance = :bal, is_active = :act "
            "WHERE id = :id;"
        ));
        q.bindValue(QStringLiteral(":id"), supp.id);
    }

    q.bindValue(QStringLiteral(":code"), supp.code);
    q.bindValue(QStringLiteral(":name"), supp.name);
    q.bindValue(QStringLiteral(":cp"), supp.contactPerson);
    q.bindValue(QStringLiteral(":phone"), supp.phone);
    q.bindValue(QStringLiteral(":email"), supp.email);
    q.bindValue(QStringLiteral(":addr"), supp.address);
    q.bindValue(QStringLiteral(":gst"), supp.gstNo);
    q.bindValue(QStringLiteral(":pt"), supp.paymentTerms);
    q.bindValue(QStringLiteral(":clim"), supp.creditLimit);
    q.bindValue(QStringLiteral(":bal"), supp.outstandingBalance);
    q.bindValue(QStringLiteral(":act"), supp.isActive ? 1 : 0);

    return q.exec();
}

bool DatabaseManager::recordPurchaseReceipt(const QString& poNumber, int supplierId, const QString& supplierInvNo, const QList<RetailNext::CartItem>& items, double totalCost, const QString& notes) {
    if (items.isEmpty()) return false;
    if (!m_db.transaction()) return false;

    QString invNo = QStringLiteral("PINV-%1-%2").arg(QDateTime::currentDateTime().toString("yyyyMMdd")).arg(supplierId);

    QSqlQuery piQ(m_db);
    piQ.prepare(QStringLiteral(
        "INSERT INTO purchase_invoices (invoice_no, supplier_invoice_no, supplier_id, subtotal, tax_amount, total_amount, payment_status, notes) "
        "VALUES (:inv, :sinv, :sid, :sub, 0.0, :tot, 'Unpaid', :notes);"
    ));
    piQ.bindValue(QStringLiteral(":inv"), invNo);
    piQ.bindValue(QStringLiteral(":sinv"), supplierInvNo);
    piQ.bindValue(QStringLiteral(":sid"), supplierId);
    piQ.bindValue(QStringLiteral(":sub"), totalCost);
    piQ.bindValue(QStringLiteral(":tot"), totalCost);
    piQ.bindValue(QStringLiteral(":notes"), notes);

    if (!piQ.exec()) {
        m_db.rollback();
        return false;
    }

    for (const auto& item : items) {
        QSqlQuery updStock(m_db);
        updStock.prepare(QStringLiteral("UPDATE products SET current_stock = current_stock + :qty WHERE id = :pid;"));
        updStock.bindValue(QStringLiteral(":qty"), item.qty);
        updStock.bindValue(QStringLiteral(":pid"), item.productId);
        updStock.exec();

        QSqlQuery sm(m_db);
        sm.prepare(QStringLiteral(
            "INSERT INTO stock_movements (product_id, reference_no, movement_type, quantity, unit_cost, balance_after, user_name, notes) "
            "VALUES (:pid, :ref, 'Purchase Receipt', :qty, :cost, (SELECT current_stock FROM products WHERE id = :pid), :user, :notes);"
        ));
        sm.bindValue(QStringLiteral(":pid"), item.productId);
        sm.bindValue(QStringLiteral(":ref"), invNo);
        sm.bindValue(QStringLiteral(":qty"), item.qty);
        sm.bindValue(QStringLiteral(":cost"), item.unitPrice);
        sm.bindValue(QStringLiteral(":user"), m_currentUser);
        sm.bindValue(QStringLiteral(":notes"), QStringLiteral("Purchase from Supplier ID %1").arg(supplierId));
        sm.exec();
    }

    QSqlQuery suppBal(m_db);
    suppBal.prepare(QStringLiteral("UPDATE suppliers SET outstanding_balance = outstanding_balance + :tot WHERE id = :sid;"));
    suppBal.bindValue(QStringLiteral(":tot"), totalCost);
    suppBal.bindValue(QStringLiteral(":sid"), supplierId);
    suppBal.exec();

    return m_db.commit();
}

bool DatabaseManager::recordSupplierPayment(int supplierId, double amount, const QString& paymentMode, const QString& notes) {
    if (!m_db.transaction()) return false;

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("UPDATE suppliers SET outstanding_balance = MAX(0.0, outstanding_balance - :amt) WHERE id = :id;"));
    q.bindValue(QStringLiteral(":amt"), amount);
    q.bindValue(QStringLiteral(":id"), supplierId);
    if (!q.exec()) {
        m_db.rollback();
        return false;
    }

    if (paymentMode == "Cash") {
        QSqlQuery crQ(m_db);
        crQ.prepare(QStringLiteral(
            "UPDATE cash_register_sessions SET cash_out = cash_out + :amt, "
            "expected_cash = opening_cash + cash_sales + cash_in - cash_out "
            "WHERE status = 'Open' ORDER BY id DESC LIMIT 1;"
        ));
        crQ.bindValue(QStringLiteral(":amt"), amount);
        crQ.exec();
    }

    return m_db.commit();
}

QList<RetailNext::PurchaseInvoiceRecord> DatabaseManager::getPurchaseInvoices() {
    QList<RetailNext::PurchaseInvoiceRecord> list;
    QSqlQuery q(QStringLiteral(
        "SELECT pi.id, pi.invoice_no, pi.supplier_invoice_no, s.name, pi.invoice_date, "
        "pi.subtotal, pi.tax_amount, pi.total_amount, pi.payment_status "
        "FROM purchase_invoices pi "
        "LEFT JOIN suppliers s ON s.id = pi.supplier_id "
        "ORDER BY pi.id DESC;"
    ), m_db);

    while (q.next()) {
        RetailNext::PurchaseInvoiceRecord r;
        r.id = q.value(0).toInt();
        r.invoiceNo = q.value(1).toString();
        r.supplierInvoiceNo = q.value(2).toString();
        r.supplierName = q.value(3).toString();
        r.invoiceDate = q.value(4).toString();
        r.subtotal = q.value(5).toDouble();
        r.taxAmount = q.value(6).toDouble();
        r.totalAmount = q.value(7).toDouble();
        r.paymentStatus = q.value(8).toString();
        list.append(r);
    }
    return list;
}

RetailNext::CashSessionRecord DatabaseManager::getActiveCashSession() {
    RetailNext::CashSessionRecord r;
    QSqlQuery q(QStringLiteral(
        "SELECT id, register_id, cashier_name, opened_at, closed_at, "
        "opening_cash, cash_sales, cash_in, cash_out, expected_cash, counted_cash, variance, status "
        "FROM cash_register_sessions WHERE status = 'Open' ORDER BY id DESC LIMIT 1;"
    ), m_db);

    if (q.next()) {
        r.id = q.value(0).toInt();
        r.registerId = q.value(1).toInt();
        r.cashierName = q.value(2).toString();
        r.openedAt = q.value(3).toString();
        r.closedAt = q.value(4).toString();
        r.openingCash = q.value(5).toDouble();
        r.cashSales = q.value(6).toDouble();
        r.cashIn = q.value(7).toDouble();
        r.cashOut = q.value(8).toDouble();
        r.expectedCash = r.openingCash + r.cashSales + r.cashIn - r.cashOut;
        r.countedCash = q.value(10).toDouble();
        r.variance = q.value(11).toDouble();
        r.status = q.value(12).toString();
    }
    return r;
}

bool DatabaseManager::recordCashInOut(double amount, const QString& type, const QString& reason, const QString& reference) {
    auto session = getActiveCashSession();
    if (session.id == 0) return false;

    if (!m_db.transaction()) return false;

    QSqlQuery ct(m_db);
    ct.prepare(QStringLiteral(
        "INSERT INTO cash_transactions (session_id, trans_type, amount, reason, reference, user_name) "
        "VALUES (:sid, :type, :amt, :reason, :ref, :user);"
    ));
    ct.bindValue(QStringLiteral(":sid"), session.id);
    ct.bindValue(QStringLiteral(":type"), type);
    ct.bindValue(QStringLiteral(":amt"), amount);
    ct.bindValue(QStringLiteral(":reason"), reason);
    ct.bindValue(QStringLiteral(":ref"), reference);
    ct.bindValue(QStringLiteral(":user"), m_currentUser);
    if (!ct.exec()) {
        m_db.rollback();
        return false;
    }

    QSqlQuery upd(m_db);
    if (type == "Cash In") {
        upd.prepare(QStringLiteral(
            "UPDATE cash_register_sessions SET cash_in = cash_in + :amt, "
            "expected_cash = opening_cash + cash_sales + cash_in + :amt - cash_out WHERE id = :id;"
        ));
    } else {
        upd.prepare(QStringLiteral(
            "UPDATE cash_register_sessions SET cash_out = cash_out + :amt, "
            "expected_cash = opening_cash + cash_sales + cash_in - (cash_out + :amt) WHERE id = :id;"
        ));
    }
    upd.bindValue(QStringLiteral(":amt"), amount);
    upd.bindValue(QStringLiteral(":id"), session.id);
    if (!upd.exec()) {
        m_db.rollback();
        return false;
    }

    return m_db.commit();
}

bool DatabaseManager::closeCashRegisterSession(int sessionId, double countedCash, const QString& notes) {
    auto session = getActiveCashSession();
    if (session.id != sessionId) return false;

    double expected = session.openingCash + session.cashSales + session.cashIn - session.cashOut;
    double variance = countedCash - expected;

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "UPDATE cash_register_sessions SET closed_at = CURRENT_TIMESTAMP, "
        "counted_cash = :counted, variance = :var, status = 'Closed' WHERE id = :id;"
    ));
    q.bindValue(QStringLiteral(":counted"), countedCash);
    q.bindValue(QStringLiteral(":var"), variance);
    q.bindValue(QStringLiteral(":id"), sessionId);

    bool ok = q.exec();
    if (ok) {
        QSqlQuery newS(m_db);
        newS.prepare(QStringLiteral(
            "INSERT INTO cash_register_sessions (register_id, cashier_name, opened_at, opening_cash, cash_sales, cash_in, cash_out, expected_cash, status) "
            "VALUES (1, :user, CURRENT_TIMESTAMP, :op, 0.0, 0.0, 0.0, :op, 'Open');"
        ));
        newS.bindValue(QStringLiteral(":user"), m_currentUser);
        newS.bindValue(QStringLiteral(":op"), countedCash > 0 ? countedCash : 200.0);
        newS.exec();
    }
    return ok;
}

QList<RetailNext::ExpenseRecord> DatabaseManager::getExpenses(const QString& period) {
    QList<RetailNext::ExpenseRecord> list;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT id, expense_no, category_name, payee, amount, tax_amount, payment_method, branch, notes, created_by, created_at FROM expenses ORDER BY id DESC;"));

    if (q.exec()) {
        while (q.next()) {
            RetailNext::ExpenseRecord r;
            r.id = q.value(0).toInt();
            r.expenseNo = q.value(1).toString();
            r.categoryName = q.value(2).toString();
            r.payee = q.value(3).toString();
            r.amount = q.value(4).toDouble();
            r.taxAmount = q.value(5).toDouble();
            r.paymentMethod = q.value(6).toString();
            r.branch = q.value(7).toString();
            r.notes = q.value(8).toString();
            r.createdBy = q.value(9).toString();
            r.createdAt = q.value(10).toString();
            list.append(r);
        }
    }
    return list;
}

bool DatabaseManager::recordExpense(const QString& category, const QString& payee, double amount, double tax, const QString& method, const QString& notes) {
    QString expNo = QStringLiteral("EXP-%1-%2").arg(QDateTime::currentDateTime().toString("yyyyMMdd")).arg(QDateTime::currentMSecsSinceEpoch() % 10000);

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "INSERT INTO expenses (expense_no, category_name, payee, amount, tax_amount, payment_method, branch, notes, created_by) "
        "VALUES (:eno, :cat, :payee, :amt, :tax, :pm, :br, :notes, :user);"
    ));
    q.bindValue(QStringLiteral(":eno"), expNo);
    q.bindValue(QStringLiteral(":cat"), category);
    q.bindValue(QStringLiteral(":payee"), payee);
    q.bindValue(QStringLiteral(":amt"), amount);
    q.bindValue(QStringLiteral(":tax"), tax);
    q.bindValue(QStringLiteral(":pm"), method);
    q.bindValue(QStringLiteral(":br"), branchName());
    q.bindValue(QStringLiteral(":notes"), notes);
    q.bindValue(QStringLiteral(":user"), m_currentUser);

    bool ok = q.exec();
    if (ok && method == "Cash") {
        recordCashInOut(amount, QStringLiteral("Cash Out"), QStringLiteral("Expense: %1 - %2").arg(category, payee), expNo);
    }
    return ok;
}

bool DatabaseManager::backupDatabase(const QString& destPath) {
    if (!isConnected()) return false;
    QSqlQuery q(m_db);
    QString safeDest = destPath;
    safeDest.replace(QStringLiteral("\\"), QStringLiteral("/"));
    return q.exec(QStringLiteral("VACUUM INTO '%1';").arg(safeDest));
}

bool DatabaseManager::restoreDatabase(const QString& srcPath) {
    if (!QFile::exists(srcPath)) return false;

    QString currentPath = m_dbPath;
    m_db.close();
    QSqlDatabase::removeDatabase(m_db.connectionName());

    QFile::remove(currentPath);
    bool copied = QFile::copy(srcPath, currentPath);

    return initialize(currentPath);
}

QString DatabaseManager::runIntegrityCheck() {
    if (!isConnected()) return QStringLiteral("Database is not connected.");

    QSqlQuery q(m_db);
    if (q.exec(QStringLiteral("PRAGMA integrity_check;")) && q.next()) {
        QString result = q.value(0).toString();
        q.exec(QStringLiteral("PRAGMA wal_checkpoint(TRUNCATE);"));
        return result;
    }
    return QStringLiteral("Integrity check failed: ") + q.lastError().text();
}

// =========================================================================
// Access Module: Stores CRUD
// =========================================================================

QList<RetailNext::StoreRecord> DatabaseManager::getStoresList(const QString& query) {
    QList<RetailNext::StoreRecord> list;
    if (!isConnected()) return list;

    QSqlQuery q(m_db);
    QString sql = QStringLiteral("SELECT id, code, name, phone, email, city, address, gst_number, is_active, created_at FROM stores ");
    if (!query.trimmed().isEmpty()) {
        sql += QStringLiteral("WHERE name LIKE :q OR code LIKE :q OR city LIKE :q OR phone LIKE :q ");
    }
    sql += QStringLiteral("ORDER BY id ASC;");

    q.prepare(sql);
    if (!query.trimmed().isEmpty()) {
        QString pattern = QStringLiteral("%%1%").arg(query.trimmed());
        q.bindValue(QStringLiteral(":q"), pattern);
    }

    if (q.exec()) {
        while (q.next()) {
            RetailNext::StoreRecord r;
            r.id = q.value(0).toInt();
            r.code = q.value(1).toString();
            r.name = q.value(2).toString();
            r.phone = q.value(3).toString();
            r.email = q.value(4).toString();
            r.city = q.value(5).toString();
            r.address = q.value(6).toString();
            r.gstNo = q.value(7).toString();
            r.isActive = q.value(8).toInt() != 0;
            r.createdAt = q.value(9).toString();
            list.append(r);
        }
    }
    return list;
}

RetailNext::StoreRecord DatabaseManager::getStoreById(int id) {
    RetailNext::StoreRecord r;
    if (!isConnected()) return r;

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT id, code, name, phone, email, city, address, gst_number, is_active, created_at FROM stores WHERE id = :id;"));
    q.bindValue(QStringLiteral(":id"), id);
    if (q.exec() && q.next()) {
        r.id = q.value(0).toInt();
        r.code = q.value(1).toString();
        r.name = q.value(2).toString();
        r.phone = q.value(3).toString();
        r.email = q.value(4).toString();
        r.city = q.value(5).toString();
        r.address = q.value(6).toString();
        r.gstNo = q.value(7).toString();
        r.isActive = q.value(8).toInt() != 0;
        r.createdAt = q.value(9).toString();
    }
    return r;
}

bool DatabaseManager::saveStore(const RetailNext::StoreRecord& store, bool isNew) {
    if (!isConnected()) return false;
    QSqlQuery q(m_db);

    if (isNew) {
        QString code = store.code.trimmed();
        if (code.isEmpty()) {
            QSqlQuery seqQ(m_db);
            seqQ.exec(QStringLiteral("SELECT COUNT(*) FROM stores;"));
            int count = (seqQ.next()) ? seqQ.value(0).toInt() + 1 : 1;
            code = QStringLiteral("STR-%1").arg(count, 3, 10, QChar('0'));
        }

        q.prepare(QStringLiteral(
            "INSERT INTO stores (code, name, phone, email, city, address, gst_number, is_active) "
            "VALUES (:code, :name, :phone, :email, :city, :address, :gst, :act);"
        ));
        q.bindValue(QStringLiteral(":code"), code);
        q.bindValue(QStringLiteral(":name"), store.name);
        q.bindValue(QStringLiteral(":phone"), store.phone);
        q.bindValue(QStringLiteral(":email"), store.email);
        q.bindValue(QStringLiteral(":city"), store.city);
        q.bindValue(QStringLiteral(":address"), store.address);
        q.bindValue(QStringLiteral(":gst"), store.gstNo);
        q.bindValue(QStringLiteral(":act"), store.isActive ? 1 : 0);
    } else {
        q.prepare(QStringLiteral(
            "UPDATE stores SET name = :name, phone = :phone, email = :email, city = :city, "
            "address = :address, gst_number = :gst, is_active = :act WHERE id = :id;"
        ));
        q.bindValue(QStringLiteral(":name"), store.name);
        q.bindValue(QStringLiteral(":phone"), store.phone);
        q.bindValue(QStringLiteral(":email"), store.email);
        q.bindValue(QStringLiteral(":city"), store.city);
        q.bindValue(QStringLiteral(":address"), store.address);
        q.bindValue(QStringLiteral(":gst"), store.gstNo);
        q.bindValue(QStringLiteral(":act"), store.isActive ? 1 : 0);
        q.bindValue(QStringLiteral(":id"), store.id);
    }

    bool ok = q.exec();
    if (!ok) {
        m_lastError = q.lastError().text();
    }
    return ok;
}

bool DatabaseManager::deleteStore(int id) {
    if (!isConnected()) return false;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("DELETE FROM stores WHERE id = :id;"));
    q.bindValue(QStringLiteral(":id"), id);
    return q.exec();
}

// =========================================================================
// Access Module: Warehouses CRUD
// =========================================================================

QList<RetailNext::WarehouseRecord> DatabaseManager::getWarehousesList(const QString& query) {
    QList<RetailNext::WarehouseRecord> list;
    if (!isConnected()) return list;

    QSqlQuery q(m_db);
    QString sql = QStringLiteral("SELECT id, code, name, phone, email, city, address, is_active, created_at FROM warehouses ");
    if (!query.trimmed().isEmpty()) {
        sql += QStringLiteral("WHERE name LIKE :q OR code LIKE :q OR city LIKE :q OR phone LIKE :q ");
    }
    sql += QStringLiteral("ORDER BY id ASC;");

    q.prepare(sql);
    if (!query.trimmed().isEmpty()) {
        QString pattern = QStringLiteral("%%1%").arg(query.trimmed());
        q.bindValue(QStringLiteral(":q"), pattern);
    }

    if (q.exec()) {
        while (q.next()) {
            RetailNext::WarehouseRecord r;
            r.id = q.value(0).toInt();
            r.code = q.value(1).toString();
            r.name = q.value(2).toString();
            r.phone = q.value(3).toString();
            r.email = q.value(4).toString();
            r.city = q.value(5).toString();
            r.address = q.value(6).toString();
            r.isActive = q.value(7).toInt() != 0;
            r.createdAt = q.value(8).toString();
            list.append(r);
        }
    }
    return list;
}

RetailNext::WarehouseRecord DatabaseManager::getWarehouseById(int id) {
    RetailNext::WarehouseRecord r;
    if (!isConnected()) return r;

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT id, code, name, phone, email, city, address, is_active, created_at FROM warehouses WHERE id = :id;"));
    q.bindValue(QStringLiteral(":id"), id);
    if (q.exec() && q.next()) {
        r.id = q.value(0).toInt();
        r.code = q.value(1).toString();
        r.name = q.value(2).toString();
        r.phone = q.value(3).toString();
        r.email = q.value(4).toString();
        r.city = q.value(5).toString();
        r.address = q.value(6).toString();
        r.isActive = q.value(7).toInt() != 0;
        r.createdAt = q.value(8).toString();
    }
    return r;
}

bool DatabaseManager::saveWarehouse(const RetailNext::WarehouseRecord& wh, bool isNew) {
    if (!isConnected()) return false;
    QSqlQuery q(m_db);

    if (isNew) {
        QString code = wh.code.trimmed();
        if (code.isEmpty()) {
            QSqlQuery seqQ(m_db);
            seqQ.exec(QStringLiteral("SELECT COUNT(*) FROM warehouses;"));
            int count = (seqQ.next()) ? seqQ.value(0).toInt() + 1 : 1;
            code = QStringLiteral("WH-%1").arg(count, 3, 10, QChar('0'));
        }

        q.prepare(QStringLiteral(
            "INSERT INTO warehouses (code, name, phone, email, city, address, is_active) "
            "VALUES (:code, :name, :phone, :email, :city, :address, :act);"
        ));
        q.bindValue(QStringLiteral(":code"), code);
        q.bindValue(QStringLiteral(":name"), wh.name);
        q.bindValue(QStringLiteral(":phone"), wh.phone);
        q.bindValue(QStringLiteral(":email"), wh.email);
        q.bindValue(QStringLiteral(":city"), wh.city);
        q.bindValue(QStringLiteral(":address"), wh.address);
        q.bindValue(QStringLiteral(":act"), wh.isActive ? 1 : 0);
    } else {
        q.prepare(QStringLiteral(
            "UPDATE warehouses SET name = :name, phone = :phone, email = :email, city = :city, "
            "address = :address, is_active = :act WHERE id = :id;"
        ));
        q.bindValue(QStringLiteral(":name"), wh.name);
        q.bindValue(QStringLiteral(":phone"), wh.phone);
        q.bindValue(QStringLiteral(":email"), wh.email);
        q.bindValue(QStringLiteral(":city"), wh.city);
        q.bindValue(QStringLiteral(":address"), wh.address);
        q.bindValue(QStringLiteral(":act"), wh.isActive ? 1 : 0);
        q.bindValue(QStringLiteral(":id"), wh.id);
    }

    bool ok = q.exec();
    if (!ok) {
        m_lastError = q.lastError().text();
    }
    return ok;
}

bool DatabaseManager::deleteWarehouse(int id) {
    if (!isConnected()) return false;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("DELETE FROM warehouses WHERE id = :id;"));
    q.bindValue(QStringLiteral(":id"), id);
    return q.exec();
}

// =========================================================================
// Access Module: Users CRUD
// =========================================================================

QList<RetailNext::UserRecord> DatabaseManager::getUsersList(const QString& query) {
    QList<RetailNext::UserRecord> list;
    if (!isConnected()) return list;

    QSqlQuery q(m_db);
    QString sql = QStringLiteral("SELECT id, username, display_name, phone, password, role, permissions, is_active, created_at FROM users ");
    if (!query.trimmed().isEmpty()) {
        sql += QStringLiteral("WHERE display_name LIKE :q OR username LIKE :q OR phone LIKE :q OR role LIKE :q ");
    }
    sql += QStringLiteral("ORDER BY id ASC;");

    q.prepare(sql);
    if (!query.trimmed().isEmpty()) {
        QString pattern = QStringLiteral("%%1%").arg(query.trimmed());
        q.bindValue(QStringLiteral(":q"), pattern);
    }

    if (q.exec()) {
        while (q.next()) {
            RetailNext::UserRecord r;
            r.id = q.value(0).toInt();
            r.username = q.value(1).toString();
            r.displayName = q.value(2).toString();
            r.phone = q.value(3).toString();
            r.password = q.value(4).toString();
            r.role = q.value(5).toString();
            r.permissions = q.value(6).toString();
            r.isActive = q.value(7).toInt() != 0;
            r.createdAt = q.value(8).toString();
            list.append(r);
        }
    }
    return list;
}

RetailNext::UserRecord DatabaseManager::getUserById(int id) {
    RetailNext::UserRecord r;
    if (!isConnected()) return r;

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT id, username, display_name, phone, password, role, permissions, is_active, created_at FROM users WHERE id = :id;"));
    q.bindValue(QStringLiteral(":id"), id);
    if (q.exec() && q.next()) {
        r.id = q.value(0).toInt();
        r.username = q.value(1).toString();
        r.displayName = q.value(2).toString();
        r.phone = q.value(3).toString();
        r.password = q.value(4).toString();
        r.role = q.value(5).toString();
        r.permissions = q.value(6).toString();
        r.isActive = q.value(7).toInt() != 0;
        r.createdAt = q.value(8).toString();
    }
    return r;
}

bool DatabaseManager::saveUser(const RetailNext::UserRecord& user, bool isNew) {
    if (!isConnected()) return false;
    QSqlQuery q(m_db);

    if (isNew) {
        q.prepare(QStringLiteral(
            "INSERT INTO users (username, display_name, phone, password, role, permissions, is_active) "
            "VALUES (:username, :display_name, :phone, :password, :role, :permissions, :act);"
        ));
        q.bindValue(QStringLiteral(":username"), user.username);
        q.bindValue(QStringLiteral(":display_name"), user.displayName);
        q.bindValue(QStringLiteral(":phone"), user.phone);
        q.bindValue(QStringLiteral(":password"), user.password);
        q.bindValue(QStringLiteral(":role"), user.role);
        q.bindValue(QStringLiteral(":permissions"), user.permissions);
        q.bindValue(QStringLiteral(":act"), user.isActive ? 1 : 0);
    } else {
        q.prepare(QStringLiteral(
            "UPDATE users SET username = :username, display_name = :display_name, phone = :phone, "
            "password = :password, role = :role, permissions = :permissions, is_active = :act WHERE id = :id;"
        ));
        q.bindValue(QStringLiteral(":username"), user.username);
        q.bindValue(QStringLiteral(":display_name"), user.displayName);
        q.bindValue(QStringLiteral(":phone"), user.phone);
        q.bindValue(QStringLiteral(":password"), user.password);
        q.bindValue(QStringLiteral(":role"), user.role);
        q.bindValue(QStringLiteral(":permissions"), user.permissions);
        q.bindValue(QStringLiteral(":act"), user.isActive ? 1 : 0);
        q.bindValue(QStringLiteral(":id"), user.id);
    }

    bool ok = q.exec();
    if (!ok) {
        m_lastError = q.lastError().text();
    }
    return ok;
}

bool DatabaseManager::deleteUser(int id) {
    if (!isConnected()) return false;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("DELETE FROM users WHERE id = :id;"));
    q.bindValue(QStringLiteral(":id"), id);
    return q.exec();
}
