# RetailNext

A configurable, offline-first native Windows desktop application for Point of Sale (POS) and Enterprise Retail Management, built with **Qt 6 Widgets**, **C++17**, **CMake**, and **SQLite**.

RetailNext is engineered as a unified, configurable retail engine designed to adapt to multiple retail businesses (Clothing & Fashion, Supermarkets, Groceries & Provisions, Footwear, Electronics, Bakery, Stationery, Hardware, and General Retail) without duplicating code.

---

## 🏛️ Comprehensive Architecture & Hierarchy

```text
RetailNext/
├── CMakeLists.txt                 # Project build configuration
├── README.md                      # Developer setup and architectural blueprint
├── build.ps1                      # Automated build script (MinGW 64-bit + windeployqt)
├── run.ps1                        # One-click launch script
├── env.ps1                        # Terminal environment setup (. .\env.ps1)
├── src/
│   ├── app/                       # Application startup, main window, multi-tab workspace
│   │   ├── main.cpp
│   │   ├── MainWindow.h
│   │   ├── MainWindow.cpp
│   │   └── MainWindow.ui
│   ├── core/                      # Settings, session, permissions, business profile, utilities
│   │   ├── Constants.h
│   │   └── BusinessType.h
│   ├── database/                  # SQLite connection, ACID transactions, stock ledger
│   │   ├── DatabaseManager.h
│   │   └── DatabaseManager.cpp
│   ├── modules/                   # Enterprise feature modules
│   │   ├── dashboard/             # Real-time operational KPIs, alerts, trends
│   │   │   ├── DashboardPage.h
│   │   │   ├── DashboardPage.cpp
│   │   │   └── DashboardPage.ui
│   │   ├── pos/                   # High-speed barcode POS billing & thermal receipting
│   │   │   ├── POSBillingPage.h
│   │   │   └── POSBillingPage.cpp
│   │   ├── products/              # Tabbed Item Master, catalog, bulk price updates
│   │   │   ├── ProductsPage.h
│   │   │   ├── ProductsPage.cpp
│   │   │   ├── ProductDialog.h
│   │   │   └── ProductDialog.cpp
│   │   ├── inventory/             # Stock overview, movement ledger, adjustments, transfers
│   │   │   ├── InventoryPage.h
│   │   │   └── InventoryPage.cpp
│   │   ├── purchases/             # Vendor directory, POs, Goods Receipt Notes (GRN)
│   │   │   ├── PurchasesPage.h
│   │   │   └── PurchasesPage.cpp
│   │   ├── sales/                 # Invoice history, sales returns, held bills manager
│   │   │   ├── SalesPage.h
│   │   │   └── SalesPage.cpp
│   │   ├── customers/             # CRM, loyalty reward points, receivables ledger
│   │   │   ├── CustomersPage.h
│   │   │   └── CustomersPage.cpp
│   │   ├── finance/               # Cash drawer sessions, day closing, expenses
│   │   │   ├── FinancePage.h
│   │   │   └── FinancePage.cpp
│   │   ├── reports/               # Sales, gross profit margin, inventory valuation, tax
│   │   │   ├── ReportsPage.h
│   │   │   └── ReportsPage.cpp
│   │   └── settings/              # Business profile, industry switcher, backup/restore
│   │       ├── SettingsPage.h
│   │       └── SettingsPage.cpp
│   ├── ui/                        # Reusable dialogs and peripheral widgets
│   │   ├── ReceiptDialog.h        # 58mm/80mm Thermal Receipt Preview & Printing
│   │   ├── BarcodeLabelDialog.h   # Shelf / Garment Barcode Sticker Generator
│   │   ├── QuickCustomerDialog.h  # In-POS Quick Customer Registration
│   │   ├── GlobalSearchDialog.h   # Cross-entity unified search (Ctrl+F)
│   │   └── PlaceholderPage.h
│   └── hardware/                  # Barcode scanners, thermal receipt printers, cash drawers
└── tests/
    └── BusinessLogicTest.cpp      # 9-point automated test suite for ACID retail workflows
```

---

## 🗄️ Desktop Command Menus (Section 3 Hierarchy)

The top menu bar implements the full desktop command hierarchy:

| Menu | Submenus and Commands | Type |
| :--- | :--- | :--- |
| **File** | New transaction (F2) · Open recent · Save (Ctrl+S) · Save As / Export · Print (Ctrl+P) · Database backup (Ctrl+B) · Restore backup · Switch business / branch · Lock application · Exit | Actions & Dialogs |
| **Masters** | Products · Product categories · Brands · Units of measurement · Barcodes · Variants · Price lists · Tax rates · Customers · Suppliers · Employees · Payment methods | Master Pages & Dialogs |
| **Inventory** | Stock overview · Stock by location · Stock movement ledger · Stock adjustment · Stock transfer · Stock count / physical verification · Reorder list · Batch and expiry · Serial numbers · Barcode labels · Opening stock | Pages & Operations |
| **Purchases** | Purchase orders · Goods received (GRN) · Purchase invoices · Purchase returns · Supplier debit notes · Supplier payments · Purchase history | Procurement Ledger |
| **Sales** | POS billing (F2) · Sales invoices · Held bills · Quotations · Sales orders · Sales returns · Credit notes · Customer payments · Sales history | Sales Documents |
| **Finance** | Cash register · Cash in / out · Expenses · Income · Accounts receivable · Accounts payable · Day closing · Payment reconciliation | Accounting & Cash |
| **Customers** | Customer list · Customer groups · Loyalty points · Store credit · Customer statements · Birthday / anniversary offers | CRM & Loyalty |
| **Reports** | Dashboard analytics · Sales reports · Profit reports · Inventory reports · Purchase reports · Customer reports · Supplier reports · Cash and payment reports · Tax reports (GST/VAT) · Audit reports | BI & Analytics |
| **Tools** | Import data · Export data · Bulk price update · Bulk barcode generation · Data validation · Database maintenance · Backup manager · Printer diagnostics · System logs | System Utilities |
| **Settings** | Business profile · Branch management · Tax and currency · POS settings · Receipt design · Printers and scanners · Users and roles · Hardware settings · Industry configuration · Backup and synchronization | Configuration |
| **Window** | Dashboard (Alt+0) · Open pages · Next tab (Ctrl+Tab) · Previous tab (Ctrl+Shift+Tab) · Close current tab (Ctrl+W) · Close all tabs · Reset workspace | Tabbed Workspace |
| **Help** | Keyboard shortcuts (F1) · User manual · Getting started · Troubleshooting · System information · About RetailNext | Documentation |

---

## 🏬 Configurable Retail Profiles (Section 18)

RetailNext uses a single core engine configurable for different shop types:
- **Clothing & Fashion:** Sizes, colors, styles, SKU matrix, variants, apparel returns.
- **Supermarket:** Rapid barcode billing, pack sizes, bulk import, offers, cashier controls.
- **Groceries & Provisions:** Decimal quantities (kg, grams, litres, ml), loose-weight pricing.
- **General Retail:** Standard SKUs, stock ledger, barcodes.
- **Electronics:** Serial numbers, IMEI tracking, warranty periods.
- **Footwear:** Size, color, style, pair-level quantities.

---

## 🛠️ Build and Run Instructions

### 1. Automated Build

In PowerShell, run:

```powershell
.\build.ps1
```

This script:
1. Safely terminates any background instance of `RetailNext.exe`.
2. Configures CMake with the MinGW 64-bit toolchain.
3. Compiles `RetailNext` and the `BusinessLogicTest` suite.
4. Executes `windeployqt` to deploy all Qt 6 DLLs and platform plugins.

### 2. Launch Application

```powershell
.\run.ps1
```

### 3. Run Automated Business Logic Tests

```powershell
. .\env.ps1
.\build-mingw\BusinessLogicTest.exe
```

---

## 🗄️ Database Architecture (Section 23)

- **Engine:** SQLite 3 (via Qt `QSQLITE` driver).
- **Default Database Location:** `%APPDATA%\RetailNext\RetailNext.db`.
- **Concurrency & Reliability:** `PRAGMA journal_mode = WAL;` and `PRAGMA foreign_keys = ON;`.
- **ACID Transactions:** Transactions cover invoice generation, stock ledger movements, customer points, and cash drawer reconciliations.
- **Audit Ledger:** Every sale, receipt, return, adjustment, and transfer records an immutable row in `stock_movements`.
#   r e t a i l - n e x t - d e s k t o p  
 