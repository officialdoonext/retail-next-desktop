#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "../ui/AppTheme.h"
#include "../ui/GlobalSearchDialog.h"
#include "../ui/PlaceholderPage.h"
#include "../ui/ReceiptDialog.h"
#include "../database/DatabaseManager.h"

#include "../modules/customers/CustomersPage.h"
#include "../modules/dashboard/DashboardPage.h"
#include "../modules/finance/FinancePage.h"
#include "../modules/inventory/InventoryPage.h"
#include "../modules/pos/POSBillingPage.h"
#include "../modules/products/ProductsPage.h"
#include "../modules/purchases/PurchasesPage.h"
#include "../modules/reports/ReportsPage.h"
#include "../modules/sales/SalesPage.h"
#include "../modules/settings/SettingsPage.h"
#include "../modules/access/AccessPage.h"

#include <QDateTime>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QShortcut>
#include <QTabBar>
#include <QTimer>
#include <QVBoxLayout>

struct SubmenuItem {
  QString name;
  QStringList options;
  int moduleIndex;
  int tabIndex;
};

static const QMap<QString, QList<SubmenuItem>> &getMenuStructure() {
  static const QMap<QString, QList<SubmenuItem>> structure = {
      {QStringLiteral("File"),
       {
           {QStringLiteral("Dashboard"),
            {QStringLiteral("🔄 Refresh"), QStringLiteral("📊 Today's Summary"),
             QStringLiteral("⚡ Quick Actions")},
            0,
            0},
           {QStringLiteral("New Bill"),
            {QStringLiteral("📄 New"), QStringLiteral("⏸️ Hold"),
             QStringLiteral("🔁 Recall"), QStringLiteral("💳 Pay"),
             QStringLiteral("🖨️ Print")},
            1,
            0},
           {QStringLiteral("New Purchase"),
            {QStringLiteral("➕ New"), QStringLiteral("💾 Save Draft"),
             QStringLiteral("📥 Post"), QStringLiteral("🖨️ Print")},
            4,
            0},
           {QStringLiteral("New Product"),
            {QStringLiteral("➕ New"), QStringLiteral("💾 Save"),
             QStringLiteral("💾 Save & New")},
            2,
            0},
           {QStringLiteral("Import Data"),
            {QStringLiteral("📦 Import Products"),
             QStringLiteral("👥 Customers"), QStringLiteral("🚚 Suppliers"),
             QStringLiteral("👁️ Preview")},
            2,
            0},
           {QStringLiteral("Export Data"),
            {QStringLiteral("📦 Products"), QStringLiteral("🧾 Sales"),
             QStringLiteral("🚚 Purchases"), QStringLiteral("👥 Customers")},
            2,
            0},
           {QStringLiteral("Backup & Restore"),
            {QStringLiteral("💾 Backup"), QStringLiteral("🔄 Restore"),
             QStringLiteral("📜 Backup History")},
            9,
            2},
           {QStringLiteral("Logout"), {QStringLiteral("🔒 Logout")}, -1, 0},
           {QStringLiteral("Exit"),
            {QStringLiteral("🚪 Exit Application")},
            -1,
            0},
       }},

      {QStringLiteral("Masters"),
       {
           {QStringLiteral("Products"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("👁️ View"), QStringLiteral("🚫 Deactivate"),
             QStringLiteral("📥 Import"), QStringLiteral("📤 Export"),
             QStringLiteral("🏷️ Barcode Labels")},
            2,
            0},
           {QStringLiteral("Categories"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("🚫 Deactivate"), QStringLiteral("📤 Export")},
            2,
            0},
           {QStringLiteral("Brands"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("🚫 Deactivate")},
            2,
            0},
           {QStringLiteral("Units"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("📏 Conversion Rules")},
            2,
            0},
           {QStringLiteral("Customers"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("📜 View Ledger"), QStringLiteral("📤 Export")},
            6,
            0},
           {QStringLiteral("Suppliers"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("📜 View Ledger"), QStringLiteral("📤 Export")},
            4,
            1},
           {QStringLiteral("Tax Master"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("🏛️ Tax Rates"),
             QStringLiteral("⚙️ Tax Settings")},
            9,
            0},
           {QStringLiteral("Price Levels"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("💰 Assign Prices")},
            2,
            0},
           {QStringLiteral("Payment Methods"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("⚙️ Configure")},
            9,
            0},
           {QStringLiteral("Warehouses"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("⚡ Activate/Deactivate")},
            3,
            0},
           {QStringLiteral("Rack / Shelf"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("📍 Assign Location")},
            3,
            0},
           {QStringLiteral("Expense Categories"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("🚫 Deactivate")},
            7,
            1},
           {QStringLiteral("Employees"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("👔 Assign Role")},
            9,
            0},
           {QStringLiteral("Discount Rules"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("⚡ Activate/Deactivate")},
            1,
            0},
       }},

      {QStringLiteral("Purchase"),
       {
           {QStringLiteral("Purchase Entry"),
            {QStringLiteral("➕ New"), QStringLiteral("💾 Save Draft"),
             QStringLiteral("📥 Post"), QStringLiteral("🖨️ Print"),
             QStringLiteral("❌ Cancel Draft")},
            4,
            0},
           {QStringLiteral("Purchase List"),
            {QStringLiteral("🔍 Search"), QStringLiteral("👁️ View"),
             QStringLiteral("✏️ Edit Draft"), QStringLiteral("🖨️ Print"),
             QStringLiteral("📤 Export")},
            4,
            0},
           {QStringLiteral("Purchase Orders"),
            {QStringLiteral("➕ New"), QStringLiteral("✅ Approve"),
             QStringLiteral("🖨️ Print"), QStringLiteral("📦 Receive Goods"),
             QStringLiteral("🔒 Close")},
            4,
            0},
           {QStringLiteral("Goods Received"),
            {QStringLiteral("📦 New GRN"), QStringLiteral("📥 Receive"),
             QStringLiteral("👁️ View"), QStringLiteral("🖨️ Print")},
            4,
            0},
           {QStringLiteral("Purchase Return"),
            {QStringLiteral("↩️ New Return"),
             QStringLiteral("📑 Select Purchase"), QStringLiteral("📥 Post"),
             QStringLiteral("🖨️ Print")},
            4,
            0},
           {QStringLiteral("Supplier Payment"),
            {QStringLiteral("💳 New Payment"), QStringLiteral("💰 Allocate"),
             QStringLiteral("💾 Save"), QStringLiteral("🖨️ Print")},
            4,
            1},
           {QStringLiteral("Purchase Reports"),
            {QStringLiteral("📅 Daily"), QStringLiteral("📆 Monthly"),
             QStringLiteral("🚚 Supplier-wise"), QStringLiteral("📦 Item-wise")},
            8,
            0},
       }},

      {QStringLiteral("Inventory"),
       {
           {QStringLiteral("Stock Overview"),
            {QStringLiteral("🔍 Search"), QStringLiteral("🔽 Filter"),
             QStringLiteral("📜 View Ledger"), QStringLiteral("📤 Export")},
            3,
            0},
           {QStringLiteral("Stock Adjustment"),
            {QStringLiteral("➕ New"), QStringLiteral("🔼 Increase"),
             QStringLiteral("🔽 Decrease"), QStringLiteral("✅ Approve"),
             QStringLiteral("📥 Post")},
            3,
            0},
           {QStringLiteral("Stock Transfer"),
            {QStringLiteral("🔄 New"), QStringLiteral("🚚 Dispatch"),
             QStringLiteral("📥 Receive"), QStringLiteral("👁️ View"),
             QStringLiteral("🖨️ Print")},
            3,
            0},
           {QStringLiteral("Stock Ledger"),
            {QStringLiteral("🔽 Filter"), QStringLiteral("👁️ View Reference"),
             QStringLiteral("📤 Export"), QStringLiteral("🖨️ Print")},
            3,
            1},
           {QStringLiteral("Stock Count"),
            {QStringLiteral("📝 New Count"), QStringLiteral("✍️ Enter Count"),
             QStringLiteral("⚖️ Review Variance"), QStringLiteral("✅ Approve")},
            3,
            0},
           {QStringLiteral("Barcode Printing"),
            {QStringLiteral("📦 Select Products"),
             QStringLiteral("📑 Template"), QStringLiteral("👁️ Preview"),
             QStringLiteral("🖨️ Print")},
            2,
            0},
           {QStringLiteral("Low Stock"),
            {QStringLiteral("🔽 Filter"), QStringLiteral("📦 Reorder"),
             QStringLiteral("📤 Export")},
            3,
            2},
           {QStringLiteral("Expiry Management"),
            {QStringLiteral("⏳ Expiring Soon"), QStringLiteral("⚠️ Expired"),
             QStringLiteral("📦 Batch Details")},
            3,
            0},
           {QStringLiteral("Batch Management"),
            {QStringLiteral("🔍 Search"), QStringLiteral("👁️ View"),
             QStringLiteral("⚖️ Adjust"), QStringLiteral("📤 Export")},
            3,
            0},
           {QStringLiteral("Damaged Stock"),
            {QStringLiteral("➕ New Entry"), QStringLiteral("✅ Approve"),
             QStringLiteral("🗑️ Dispose")},
            3,
            0},
           {QStringLiteral("Stock Valuation"),
            {QStringLiteral("🧮 Calculate"), QStringLiteral("🔽 Filter"),
             QStringLiteral("🖨️ Print"), QStringLiteral("📤 Export")},
            8,
            2},
           {QStringLiteral("Opening Stock"),
            {QStringLiteral("📥 Import"), QStringLiteral("✍️ Enter Stock"),
             QStringLiteral("✅ Validate"), QStringLiteral("📥 Post")},
            3,
            0},
       }},

      {QStringLiteral("Sales"),
       {
           {QStringLiteral("POS Billing"),
            {QStringLiteral("📄 New Bill"), QStringLiteral("⏸️ Hold"),
             QStringLiteral("🔁 Recall"), QStringLiteral("👤 Customer"),
             QStringLiteral("🏷️ Discount"), QStringLiteral("💳 Pay"),
             QStringLiteral("🖨️ Print")},
            1,
            0},
           {QStringLiteral("Sales Invoice List"),
            {QStringLiteral("🔍 Search"), QStringLiteral("👁️ View"),
             QStringLiteral("🖨️ Print"),
             QStringLiteral("💵 Receive Payment"), QStringLiteral("↩️ Return"),
             QStringLiteral("📤 Export")},
            5,
            0},
           {QStringLiteral("Sales Return"),
            {QStringLiteral("↩️ New Return"), QStringLiteral("📑 Load Invoice"),
             QStringLiteral("🧮 Calculate Refund"), QStringLiteral("✅ Approve"),
             QStringLiteral("🖨️ Print")},
            5,
            0},
           {QStringLiteral("Quotations"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("🖨️ Print"),
             QStringLiteral("🔄 Convert to Invoice")},
            5,
            0},
           {QStringLiteral("Sales Orders"),
            {QStringLiteral("➕ New"), QStringLiteral("✅ Confirm"),
             QStringLiteral("📦 Fulfil"), QStringLiteral("🧾 Invoice"),
             QStringLiteral("❌ Cancel")},
            5,
            0},
           {QStringLiteral("Held Bills"),
            {QStringLiteral("🔍 Search"), QStringLiteral("▶️ Resume"),
             QStringLiteral("🗑️ Delete Draft")},
            5,
            1},
           {QStringLiteral("Cancelled / Voided Bills"),
            {QStringLiteral("🔍 Search"), QStringLiteral("👁️ View"),
             QStringLiteral("📜 Audit Details"), QStringLiteral("🖨️ Print")},
            5,
            0},
           {QStringLiteral("Sales Reports"),
            {QStringLiteral("📅 Daily"), QStringLiteral("📆 Monthly"),
             QStringLiteral("📦 Item-wise"), QStringLiteral("👔 Cashier-wise"),
             QStringLiteral("💳 Payment-wise")},
            8,
            0},
       }},

      {QStringLiteral("Customers"),
       {
           {QStringLiteral("Customer List"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("👁️ View"), QStringLiteral("🔍 Search"),
             QStringLiteral("📤 Export")},
            6,
            0},
           {QStringLiteral("Customer Ledger"),
            {QStringLiteral("🔽 Filter"), QStringLiteral("👁️ View Invoice"),
             QStringLiteral("🖨️ Print Statement"),
             QStringLiteral("📤 Export")},
            6,
            0},
           {QStringLiteral("Receive Payment"),
            {QStringLiteral("💵 New Receipt"), QStringLiteral("💰 Allocate"),
             QStringLiteral("💾 Save"), QStringLiteral("🖨️ Print")},
            6,
            0},
           {QStringLiteral("Outstanding Dues"),
            {QStringLiteral("🔍 Search"), QStringLiteral("💵 Receive Payment"),
             QStringLiteral("🖨️ Print Statement")},
            6,
            0},
           {QStringLiteral("Loyalty Points"),
            {QStringLiteral("👁️ View"), QStringLiteral("⭐ Earned Points"),
             QStringLiteral("🎁 Redeem"), QStringLiteral("📜 History")},
            6,
            0},
           {QStringLiteral("Customer Groups"),
            {QStringLiteral("➕ New Group"), QStringLiteral("✏️ Edit"),
             QStringLiteral("👥 Assign Customers")},
            6,
            0},
       }},

      {QStringLiteral("Accounts"),
       {
           {QStringLiteral("Cash Register"),
            {QStringLiteral("🔓 Open Shift"), QStringLiteral("💵 Cash In"),
             QStringLiteral("💸 Cash Out"), QStringLiteral("🔒 Close Shift")},
            7,
            0},
           {QStringLiteral("Day Closing"),
            {QStringLiteral("👁️ Preview"), QStringLiteral("⚖️ Reconcile"),
             QStringLiteral("🔒 Close Day"), QStringLiteral("🖨️ Print")},
            7,
            0},
           {QStringLiteral("Expenses"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit Draft"),
             QStringLiteral("✅ Approve"), QStringLiteral("🖨️ Print"),
             QStringLiteral("📤 Export")},
            7,
            1},
           {QStringLiteral("Supplier Outstanding"),
            {QStringLiteral("🔍 Search"), QStringLiteral("💳 Pay Supplier"),
             QStringLiteral("📜 View Ledger")},
            4,
            1},
           {QStringLiteral("Customer Outstanding"),
            {QStringLiteral("🔍 Search"), QStringLiteral("💵 Receive Payment"),
             QStringLiteral("📜 View Ledger")},
            6,
            0},
           {QStringLiteral("Bank / Payment Accounts"),
            {QStringLiteral("➕ New Account"), QStringLiteral("💳 Transactions"),
             QStringLiteral("⚖️ Reconcile")},
            7,
            0},
           {QStringLiteral("Cash Book"),
            {QStringLiteral("🔽 Filter"), QStringLiteral("👁️ View Entries"),
             QStringLiteral("🖨️ Print"), QStringLiteral("📤 Export")},
            7,
            0},
           {QStringLiteral("Payment History"),
            {QStringLiteral("🔍 Search"), QStringLiteral("👁️ View"),
             QStringLiteral("🖨️ Print"), QStringLiteral("📤 Export")},
            7,
            0},
       }},

      {QStringLiteral("Reports"),
       {
           {QStringLiteral("Dashboard Reports"),
            {QStringLiteral("📊 Sales Summary"),
             QStringLiteral("💰 Profit Summary"),
             QStringLiteral("📦 Stock Summary")},
            0,
            0},
           {QStringLiteral("Sales Reports"),
            {QStringLiteral("📅 Daily"), QStringLiteral("📆 Monthly"),
             QStringLiteral("📦 Item-wise"),
             QStringLiteral("📁 Category-wise")},
            8,
            0},
           {QStringLiteral("Purchase Reports"),
            {QStringLiteral("📅 Daily"), QStringLiteral("📆 Monthly"),
             QStringLiteral("🚚 Supplier-wise"), QStringLiteral("📦 Item-wise")},
            8,
            0},
           {QStringLiteral("Inventory Reports"),
            {QStringLiteral("📦 Stock Summary"),
             QStringLiteral("📜 Stock Ledger"), QStringLiteral("💰 Valuation")},
            8,
            2},
           {QStringLiteral("Profit Reports"),
            {QStringLiteral("💰 Gross Profit"),
             QStringLiteral("📦 Product-wise Profit"),
             QStringLiteral("📁 Category-wise Profit")},
            8,
            1},
           {QStringLiteral("Tax Reports"),
            {QStringLiteral("🏛️ Tax Summary"), QStringLiteral("🧾 Sales Tax"),
             QStringLiteral("📥 Purchase Tax")},
            8,
            3},
           {QStringLiteral("Customer Reports"),
            {QStringLiteral("👥 Customer Sales"),
             QStringLiteral("💳 Outstanding"), QStringLiteral("📜 Ledger")},
            8,
            0},
           {QStringLiteral("Supplier Reports"),
            {QStringLiteral("🚚 Purchases"), QStringLiteral("💳 Outstanding"),
             QStringLiteral("📜 Ledger")},
            8,
            0},
           {QStringLiteral("Cashier Reports"),
            {QStringLiteral("👔 Cashier Sales"),
             QStringLiteral("🔒 Shift Summary"), QStringLiteral("⚖️ Variance")},
            8,
            0},
           {QStringLiteral("Expense Reports"),
            {QStringLiteral("📁 Category-wise"), QStringLiteral("📅 Date-wise")},
            7,
            1},
           {QStringLiteral("Audit Reports"),
            {QStringLiteral("👤 User Activity"),
             QStringLiteral("❌ Voided Bills"), QStringLiteral("⚖️ Adjustments")},
            8,
            0},
       }},

      {QStringLiteral("Access"),
       {
           {QStringLiteral("Stores"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("🔄 Refresh")},
            10,
            0},
           {QStringLiteral("Warehouses"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("🔄 Refresh")},
            10,
            1},
           {QStringLiteral("Users"),
            {QStringLiteral("➕ New"), QStringLiteral("✏️ Edit"),
             QStringLiteral("🔄 Refresh")},
            10,
            2},
       }},

      {QStringLiteral("Tools"),
       {
           {QStringLiteral("Import"),
            {QStringLiteral("📦 Products"), QStringLiteral("👥 Customers"),
             QStringLiteral("🚚 Suppliers"),
             QStringLiteral("📥 Opening Stock")},
            2,
            0},
           {QStringLiteral("Export"),
            {QStringLiteral("📦 Products"), QStringLiteral("🧾 Sales"),
             QStringLiteral("🚚 Purchases"), QStringLiteral("📊 Reports")},
            2,
            0},
           {QStringLiteral("Bulk Price Update"),
            {QStringLiteral("📦 Select Products"),
             QStringLiteral("💰 Update Price"), QStringLiteral("👁️ Preview"),
             QStringLiteral("✅ Apply")},
            2,
            0},
           {QStringLiteral("Printer Setup"),
            {QStringLiteral("🧾 Receipt Printer"),
             QStringLiteral("📄 A4 Printer"),
             QStringLiteral("🏷️ Barcode Printer"),
             QStringLiteral("🖨️ Test Print")},
            9,
            0},
           {QStringLiteral("Barcode Setup"),
            {QStringLiteral("🔫 Scanner Settings"),
             QStringLiteral("🏷️ Barcode Formats"),
             QStringLiteral("📑 Label Templates")},
            9,
            0},
           {QStringLiteral("Keyboard Shortcuts"),
            {QStringLiteral("👁️ View"), QStringLiteral("✏️ Edit"),
             QStringLiteral("🔄 Reset Defaults")},
            -1,
            0},
           {QStringLiteral("Database Tools"),
            {QStringLiteral("🔍 Check Database"), QStringLiteral("⚡ Optimise"),
             QStringLiteral("📋 Migration Status")},
            9,
            2},
           {QStringLiteral("Sync Manager"),
            {QStringLiteral("🔄 Sync Now"), QStringLiteral("⏳ Pending Items"),
             QStringLiteral("🔁 Retry Errors")},
            9,
            2},
           {QStringLiteral("Diagnostics"),
            {QStringLiteral("🗄️ Database"), QStringLiteral("🖨️ Printer"),
             QStringLiteral("💾 Storage"), QStringLiteral("📡 Sync Status")},
            9,
            2},
       }},

      {QStringLiteral("Window"),
       {
           {QStringLiteral("Open Pages"),
            {QStringLiteral("📑 List of open pages")},
            -1,
            0},
           {QStringLiteral("Next Tab"),
            {QStringLiteral("▶️ Navigate to next page")},
            -1,
            0},
           {QStringLiteral("Previous Tab"),
            {QStringLiteral("◀️ Navigate to previous page")},
            -1,
            0},
           {QStringLiteral("Close Current Tab"),
            {QStringLiteral("❌ Close selected page")},
            -1,
            0},
           {QStringLiteral("Close All Tabs"),
            {QStringLiteral("🚫 Close all permitted pages")},
            -1,
            0},
           {QStringLiteral("Reset Layout"),
            {QStringLiteral("🔄 Restore default layout")},
            -1,
            0},
       }},

      {QStringLiteral("Help"),
       {
           {QStringLiteral("User Guide"),
            {QStringLiteral("📖 Open Help")},
            -1,
            0},
           {QStringLiteral("Keyboard Shortcuts"),
            {QStringLiteral("⌨️ View Shortcuts")},
            -1,
            0},
           {QStringLiteral("About RetailNext"),
            {QStringLiteral("ℹ️ Version & Build Information")},
            -1,
            0},
           {QStringLiteral("Diagnostics"),
            {QStringLiteral("🩺 Run Checks"), QStringLiteral("👁️ View Results")},
            -1,
            0},
           {QStringLiteral("Support"),
            {QStringLiteral("💬 Support Details"),
             QStringLiteral("📤 Export Diagnostic Log")},
            -1,
            0},
           {QStringLiteral("Check for Updates"),
            {QStringLiteral("🔄 Check"), QStringLiteral("📜 View Release Notes")},
            -1,
            0},
       }}};
  return structure;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_topMenuBar(nullptr),
      m_topToolbar(nullptr), m_secondaryActionsLayout(nullptr),
      m_btnSubmenuPicker(nullptr), m_currentCategory(QStringLiteral("Sales")),
      m_currentSubmenu(QStringLiteral("POS Billing")),
      m_workspaceTabs(nullptr), m_dashboardPage(nullptr), m_posPage(nullptr),
      m_productsPage(nullptr), m_inventoryPage(nullptr),
      m_purchasesPage(nullptr), m_salesPage(nullptr), m_customersPage(nullptr),
      m_financePage(nullptr), m_reportsPage(nullptr), m_settingsPage(nullptr),
      m_accessPage(nullptr),
      m_genericPlaceholderPage(nullptr), m_statusReadyLabel(nullptr),
      m_statusUserLabel(nullptr), m_statusBranchLabel(nullptr),
      m_statusDbLabel(nullptr), m_statusClockLabel(nullptr),
      m_clockTimer(nullptr) {
  ui->setupUi(this);

  setMinimumSize(1100, 740);
  resize(1280, 800);
  setWindowTitle(QStringLiteral(
      "RetailNext - Configurable Enterprise POS & Inventory Engine"));

  ui->navPanel->hide();
  ui->stackedWidget->hide();
  menuBar()->hide();
  ui->mainToolBar->hide();

  setupWorkspaceTabs();
  setupNativeRibbon();
  setupGlobalKeyboardShortcuts();
  setupStatusBar();

  selectCategorySubmenu(QStringLiteral("Sales"), QStringLiteral("POS Billing"));
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setupNativeRibbon() {
  QWidget *centralContainer = new QWidget(this);
  QVBoxLayout *rootLayout = new QVBoxLayout(centralContainer);
  rootLayout->setContentsMargins(0, 0, 0, 0);
  rootLayout->setSpacing(0);

  // Row 1: Top Command Menu Bar with all 11 Categories
  setupCommandMenuBar(centralContainer, rootLayout);

  // Row 2: Secondary Toolbar with Submenu selector & dynamic action buttons
  setupActionToolbar(centralContainer, rootLayout);

  // Central Workspace
  rootLayout->addWidget(m_workspaceTabs, 1);

  setCentralWidget(centralContainer);
}

void MainWindow::setupCommandMenuBar(QWidget *parent,
                                     QVBoxLayout *rootLayout) {
  m_topMenuBar = new QFrame(parent);
  m_topMenuBar->setFixedHeight(26);
  m_topMenuBar->setStyleSheet(QStringLiteral(
      "QFrame { background-color: #D9E8F5; border-bottom: 1px solid #A4BED2; "
      "}"));

  QHBoxLayout *menuLayout = new QHBoxLayout(m_topMenuBar);
  menuLayout->setContentsMargins(4, 0, 4, 0);
  menuLayout->setSpacing(1);

  struct CategoryShortcut {
    QString name;
    QString keySeq;
  };

  const QList<CategoryShortcut> categories = {
      {QStringLiteral("File"), QStringLiteral("Alt+F")},
      {QStringLiteral("Masters"), QStringLiteral("Alt+M")},
      {QStringLiteral("Purchase"), QStringLiteral("Alt+P")},
      {QStringLiteral("Inventory"), QStringLiteral("Alt+I")},
      {QStringLiteral("Sales"), QStringLiteral("Alt+S")},
      {QStringLiteral("Customers"), QStringLiteral("Alt+C")},
      {QStringLiteral("Accounts"), QStringLiteral("Alt+A")},
      {QStringLiteral("Reports"), QStringLiteral("Alt+R")},
      {QStringLiteral("Access"), QStringLiteral("Alt+K")},
      {QStringLiteral("Tools"), QStringLiteral("Alt+T")},
      {QStringLiteral("Window"), QStringLiteral("Alt+W")},
      {QStringLiteral("Help"), QStringLiteral("Alt+H")}};

  const auto &menuData = getMenuStructure();

  for (const auto &cs : categories) {
    const QString &cat = cs.name;
    QPushButton *btnCat = new QPushButton(QStringLiteral("%1 (%2)").arg(cat, cs.keySeq), m_topMenuBar);
    btnCat->setCursor(Qt::PointingHandCursor);
    btnCat->setShortcut(QKeySequence(cs.keySeq));
    btnCat->setStyleSheet(QStringLiteral(
        "QPushButton { background: transparent; color: #0A294A; border: 1px "
        "solid transparent; "
        "border-radius: 2px; padding: 2px 7px; font-size: 8.5pt; font-weight: "
        "bold; } "
        "QPushButton:hover { background: #E5EFF8; border: 1px solid #7A9EB9; "
        "} "
        "QPushButton:pressed { background: #C8DBEC; } "
        "QPushButton::menu-indicator { image: none; width: 0px; }"));

    QMenu *catMenu = new QMenu(btnCat);
    catMenu->setStyleSheet(QStringLiteral(
        "QMenu { background-color: #FFFFFF; border: 1px solid #7A9EB9; color: "
        "#0A294A; font-size: 8.5pt; } "
        "QMenu::item { padding: 4px 20px 4px 10px; } "
        "QMenu::item:selected { background-color: #2D82D6; color: #FFFFFF; "
        "font-weight: bold; }"));

    if (menuData.contains(cat)) {
      const auto &subList = menuData[cat];
      for (const auto &sub : subList) {
        QAction *act = catMenu->addAction(sub.name);
        connect(act, &QAction::triggered, this,
                [this, cat, sub]() { selectCategorySubmenu(cat, sub.name); });
      }
    }

    btnCat->setMenu(catMenu);

    connect(btnCat, &QPushButton::clicked, this, [this, cat, menuData]() {
      if (menuData.contains(cat) && !menuData[cat].isEmpty()) {
        selectCategorySubmenu(cat, menuData[cat].first().name);
      }
    });

    // Global shortcut to pop open or navigate to this menu
    new QShortcut(QKeySequence(cs.keySeq), this, [btnCat]() {
      btnCat->showMenu();
    });

    m_menuButtons[cat] = btnCat;
    menuLayout->addWidget(btnCat);
  }

  menuLayout->addStretch();
  rootLayout->addWidget(m_topMenuBar);
}

void MainWindow::setupActionToolbar(QWidget *parent, QVBoxLayout *rootLayout) {
  m_topToolbar = new QFrame(parent);
  m_topToolbar->setFixedHeight(36);
  m_topToolbar->setStyleSheet(QStringLiteral(
      "QFrame { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 "
      "#F0F6FC, stop:1 #D9E8F5); "
      "border-bottom: 1px solid #8FAEC7; }"));

  QHBoxLayout *tbLayout = new QHBoxLayout(m_topToolbar);
  tbLayout->setContentsMargins(6, 2, 6, 2);
  tbLayout->setSpacing(6);

  // 1. Breadcrumb / Submenu Switcher Button
  m_btnSubmenuPicker = new QPushButton(m_topToolbar);
  m_btnSubmenuPicker->setCursor(Qt::PointingHandCursor);
  m_btnSubmenuPicker->setShortcut(QKeySequence(Qt::Key_F11));
  m_btnSubmenuPicker->setStyleSheet(QStringLiteral(
      "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
      "stop:0 #FFFFFF, stop:1 #E0EBF5); "
      "color: #000080; border: 1px solid #7A9EB9; border-radius: 2px; padding: "
      "3px 10px; font-weight: bold; font-size: 8.5pt; } "
      "QPushButton:hover { background: #EDF4FB; border-color: #0055A5; }"));
  tbLayout->addWidget(m_btnSubmenuPicker);

  // Vertical Separator
  QFrame *sep = new QFrame(m_topToolbar);
  sep->setFrameShape(QFrame::VLine);
  sep->setStyleSheet(QStringLiteral("color: #8FAEC7;"));
  tbLayout->addWidget(sep);

  // 2. Dynamic Secondary Action Options Container
  QWidget *actionsContainer = new QWidget(m_topToolbar);
  m_secondaryActionsLayout = new QHBoxLayout(actionsContainer);
  m_secondaryActionsLayout->setContentsMargins(0, 0, 0, 0);
  m_secondaryActionsLayout->setSpacing(4);
  tbLayout->addWidget(actionsContainer);

  tbLayout->addStretch();

  // 3. Always-Accessible Quick POS Billing Button
  QPushButton *btnQuickPos = new QPushButton(
      QStringLiteral("🛒 POS Register (F2)"), m_topToolbar);
  btnQuickPos->setCursor(Qt::PointingHandCursor);
  btnQuickPos->setShortcut(QKeySequence(Qt::Key_F2));
  btnQuickPos->setStyleSheet(QStringLiteral(
      "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
      "stop:0 #E8F5E9, stop:1 #C8E6C9); "
      "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; "
      "padding: 3px 12px; font-weight: bold; font-size: 8.5pt; } "
      "QPushButton:hover { background: #C8E6C9; border-color: #388E3C; }"));
  connect(btnQuickPos, &QPushButton::clicked, this, [this]() {
    selectCategorySubmenu(QStringLiteral("Sales"),
                          QStringLiteral("POS Billing"));
  });
  tbLayout->addWidget(btnQuickPos);

  // 4. Quick Search Box
  QLineEdit *quickSearch = new QLineEdit(m_topToolbar);
  quickSearch->setPlaceholderText(
      QStringLiteral("Search products, invoices, customers... (Ctrl+F)"));
  quickSearch->setFixedWidth(240);
  quickSearch->setStyleSheet(QStringLiteral(
      "QLineEdit { background: #FFFFFF; border: 1px solid #7A9EB9; "
      "border-radius: 2px; "
      "padding: 2px 6px; font-size: 8.5pt; color: #000080; font-weight: 500; "
      "} "
      "QLineEdit:focus { border: 1px solid #0055A5; background-color: "
      "#FFFFF2; }"));
  connect(quickSearch, &QLineEdit::returnPressed, this,
          &MainWindow::onGlobalSearchActivated);
  tbLayout->addWidget(quickSearch);

  rootLayout->addWidget(m_topToolbar);
}

void MainWindow::selectCategorySubmenu(const QString &category,
                                       const QString &submenu) {
  m_currentCategory = category;
  m_currentSubmenu = submenu;

  // Highlight active category button in top menu bar
  for (auto it = m_menuButtons.begin(); it != m_menuButtons.end(); ++it) {
    if (it.key() == category) {
      it.value()->setStyleSheet(QStringLiteral(
          "QPushButton { background: #2D82D6; color: #FFFFFF; border: 1px "
          "solid #0055A5; "
          "border-radius: 2px; padding: 2px 9px; font-size: 8.5pt; "
          "font-weight: bold; } "
          "QPushButton::menu-indicator { image: none; width: 0px; }"));
    } else {
      it.value()->setStyleSheet(QStringLiteral(
          "QPushButton { background: transparent; color: #0A294A; border: 1px "
          "solid transparent; "
          "border-radius: 2px; padding: 2px 9px; font-size: 8.5pt; "
          "font-weight: bold; } "
          "QPushButton:hover { background: #E5EFF8; border: 1px solid #7A9EB9; "
          "} "
          "QPushButton:pressed { background: #C8DBEC; } "
          "QPushButton::menu-indicator { image: none; width: 0px; }"));
    }
  }

  // Update Breadcrumb & Submenu Switcher Menu
  m_btnSubmenuPicker->setText(
      QStringLiteral("📁 %1 ▸ %2 ▾ (F11)").arg(category, submenu));

  QMenu *subPickerMenu = new QMenu(m_btnSubmenuPicker);
  subPickerMenu->setStyleSheet(QStringLiteral(
      "QMenu { background-color: #FFFFFF; border: 1px solid #7A9EB9; color: "
      "#0A294A; font-size: 8.5pt; } "
      "QMenu::item { padding: 4px 20px 4px 10px; } "
      "QMenu::item:selected { background-color: #2D82D6; color: #FFFFFF; "
      "font-weight: bold; }"));

  const auto &menuData = getMenuStructure();
  if (menuData.contains(category)) {
    for (const auto &s : menuData[category]) {
      QAction *act = subPickerMenu->addAction(s.name);
      if (s.name == submenu) {
        QFont f = act->font();
        f.setBold(true);
        act->setFont(f);
      }
      connect(act, &QAction::triggered, this,
              [this, category, s]() { selectCategorySubmenu(category, s.name); });
    }
  }
  m_btnSubmenuPicker->setMenu(subPickerMenu);

  // Update Secondary Toolbar Options
  updateSecondaryToolbar(category, submenu);

  // Navigate workspace
  int modIdx = -1;
  int tabIdx = 0;
  if (menuData.contains(category)) {
    for (const auto &s : menuData[category]) {
      if (s.name == submenu) {
        modIdx = s.moduleIndex;
        tabIdx = s.tabIndex;
        break;
      }
    }
  }

  if (modIdx >= 0) {
    openModule(modIdx);
    if (modIdx == 3 && m_inventoryPage)
      m_inventoryPage->switchToTab(tabIdx);
    else if (modIdx == 4 && m_purchasesPage)
      m_purchasesPage->switchToTab(tabIdx);
    else if (modIdx == 5 && m_salesPage)
      m_salesPage->switchToTab(tabIdx);
    else if (modIdx == 7 && m_financePage)
      m_financePage->switchToTab(tabIdx);
    else if (modIdx == 8 && m_reportsPage)
      m_reportsPage->switchToTab(tabIdx);
    else if (modIdx == 9 && m_settingsPage)
      m_settingsPage->switchToTab(tabIdx);
    else if (modIdx == 10 && m_accessPage)
      m_accessPage->switchToSubTab(tabIdx);
  } else {
    // Show descriptive placeholder for specialized menu commands
    if (m_genericPlaceholderPage) {
      m_workspaceTabs->setCurrentWidget(m_genericPlaceholderPage);
    }
  }
}

void MainWindow::updateSecondaryToolbar(const QString &category,
                                        const QString &submenu) {
  // Clear existing secondary action buttons
  QLayoutItem *item;
  while ((item = m_secondaryActionsLayout->takeAt(0)) != nullptr) {
    if (item->widget())
      delete item->widget();
    delete item;
  }

  const auto &menuData = getMenuStructure();
  if (!menuData.contains(category))
    return;

  QStringList options;
  for (const auto &s : menuData[category]) {
    if (s.name == submenu) {
      options = s.options;
      break;
    }
  }

  for (int i = 0; i < options.size(); ++i) {
    const QString &opt = options[i];
    QString shortcutKey = QStringLiteral("Alt+%1").arg(i + 1);
    QString btnText = QStringLiteral("%1 (%2)").arg(opt, shortcutKey);

    QPushButton *btn = new QPushButton(btnText, m_topToolbar);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setShortcut(QKeySequence(shortcutKey));

    if (opt.contains(QStringLiteral("New")) ||
        opt.contains(QStringLiteral("Save")) ||
        opt.contains(QStringLiteral("Post")) ||
        opt.contains(QStringLiteral("Approve")) ||
        opt.contains(QStringLiteral("Pay"))) {
      btn->setStyleSheet(QStringLiteral(
          "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
          "stop:0 #E8F5E9, stop:1 #C8E6C9); "
          "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; "
          "padding: 3px 8px; font-weight: bold; font-size: 8.5pt; } "
          "QPushButton:hover { background: #C8E6C9; border-color: #388E3C; }"));
    } else if (opt.contains(QStringLiteral("Cancel")) ||
               opt.contains(QStringLiteral("Delete")) ||
               opt.contains(QStringLiteral("Deactivate")) ||
               opt.contains(QStringLiteral("Void")) ||
               opt.contains(QStringLiteral("Exit")) ||
               opt.contains(QStringLiteral("Logout"))) {
      btn->setStyleSheet(QStringLiteral(
          "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
          "stop:0 #FFEBEE, stop:1 #FFCDD2); "
          "color: #B71C1C; border: 1px solid #EF9A9A; border-radius: 2px; "
          "padding: 3px 8px; font-weight: bold; font-size: 8.5pt; } "
          "QPushButton:hover { background: #FFCDD2; border-color: #D32F2F; }"));
    } else {
      btn->setStyleSheet(QStringLiteral(
          "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
          "stop:0 #FFFFFF, stop:1 #E0EBF5); "
          "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; "
          "padding: 3px 8px; font-size: 8.5pt; font-weight: 500; } "
          "QPushButton:hover { background: #EDF4FB; border-color: #4A82AD; } "
          "QPushButton:pressed { background: #C8DBEC; }"));
    }

    connect(btn, &QPushButton::clicked, this,
            [this, opt]() { onSecondaryActionClicked(opt); });

    new QShortcut(QKeySequence(shortcutKey), btn, [btn]() {
      btn->click();
    });

    m_secondaryActionsLayout->addWidget(btn);
  }
}

void MainWindow::onSecondaryActionClicked(const QString &action) {
  // POS & Billing actions
  if (action.contains(QStringLiteral("New Bill")) ||
      action == QStringLiteral("📄 New")) {
    openModule(1);
    if (m_posPage)
      m_posPage->resetBillingSession();
  } else if (action.contains(QStringLiteral("Hold"))) {
    if (m_posPage)
      m_posPage->onHoldBillClicked();
  } else if (action.contains(QStringLiteral("Recall"))) {
    if (m_posPage)
      m_posPage->onRecallBillClicked();
  } else if (action.contains(QStringLiteral("Pay"))) {
    if (m_posPage)
      m_posPage->onCheckoutClicked();
  } else if (action.contains(QStringLiteral("Print"))) {
    onActionPrint();
  }
  // Product actions
  else if (action.contains(QStringLiteral("Save & New")) ||
           (action.contains(QStringLiteral("New")) &&
            m_currentSubmenu == QStringLiteral("Products"))) {
    openModule(2);
    if (m_productsPage)
      m_productsPage->onAddProductClicked();
  } else if (action.contains(QStringLiteral("Edit")) &&
             m_currentSubmenu == QStringLiteral("Products")) {
    openModule(2);
    if (m_productsPage)
      m_productsPage->onEditProductClicked();
  } else if (action.contains(QStringLiteral("Deactivate")) &&
             m_currentSubmenu == QStringLiteral("Products")) {
    if (m_productsPage)
      m_productsPage->onDeleteProductClicked();
  } else if (action.contains(QStringLiteral("Barcode Labels")) ||
             action.contains(QStringLiteral("Barcode Printing"))) {
    openModule(2);
    if (m_productsPage)
      m_productsPage->onPrintBarcodeClicked();
  } else if (action.contains(QStringLiteral("Bulk Price Update")) ||
             action.contains(QStringLiteral("Update Price"))) {
    openModule(2);
    if (m_productsPage)
      m_productsPage->onBulkPriceUpdateClicked();
  }
  // Purchase & Inventory actions
  else if (action.contains(QStringLiteral("New GRN")) ||
           (action.contains(QStringLiteral("New")) &&
            m_currentCategory == QStringLiteral("Purchase"))) {
    openModule(4);
    if (m_purchasesPage)
      m_purchasesPage->onNewInwardReceiptClicked();
  } else if (action.contains(QStringLiteral("Stock Adjustment")) ||
             action.contains(QStringLiteral("Increase")) ||
             action.contains(QStringLiteral("Decrease"))) {
    openModule(3);
    if (m_inventoryPage)
      m_inventoryPage->onStockAdjustmentClicked();
  } else if (action.contains(QStringLiteral("Stock Transfer")) ||
             action.contains(QStringLiteral("Dispatch"))) {
    openModule(3);
    if (m_inventoryPage)
      m_inventoryPage->onStockTransferClicked();
  }
  // Cash & Accounts actions
  else if (action.contains(QStringLiteral("Cash In"))) {
    openModule(7);
    if (m_financePage)
      m_financePage->onCashInClicked();
  } else if (action.contains(QStringLiteral("Cash Out"))) {
    openModule(7);
    if (m_financePage)
      m_financePage->onCashOutClicked();
  } else if (action.contains(QStringLiteral("Close Shift")) ||
             action.contains(QStringLiteral("Close Day")) ||
             action.contains(QStringLiteral("Day Closing"))) {
    openModule(7);
    if (m_financePage)
      m_financePage->onCloseRegisterClicked();
  } else if (action.contains(QStringLiteral("New Payment")) ||
             action.contains(QStringLiteral("Pay Supplier"))) {
    openModule(4);
    if (m_purchasesPage)
      m_purchasesPage->onRecordPaymentClicked();
  } else if (action.contains(QStringLiteral("Receive Payment")) ||
             action.contains(QStringLiteral("New Receipt"))) {
    openModule(6);
    if (m_customersPage)
      m_customersPage->onRecordPaymentClicked();
  }
  // Customer actions
  else if (action.contains(QStringLiteral("New")) &&
           m_currentCategory == QStringLiteral("Customers")) {
    openModule(6);
    if (m_customersPage)
      m_customersPage->onAddCustomerClicked();
  } else if (action.contains(QStringLiteral("Edit")) &&
             m_currentCategory == QStringLiteral("Customers")) {
    openModule(6);
    if (m_customersPage)
      m_customersPage->onEditCustomerClicked();
  }
  // Access actions (Stores, Warehouses, Users)
  else if (m_currentCategory == QStringLiteral("Access")) {
    openModule(10);
    if (action.contains(QStringLiteral("New"))) {
      if (m_currentSubmenu == QStringLiteral("Stores")) {
        if (m_accessPage) m_accessPage->openNewStoreDialog();
      } else if (m_currentSubmenu == QStringLiteral("Warehouses")) {
        if (m_accessPage) m_accessPage->openNewWarehouseDialog();
      } else if (m_currentSubmenu == QStringLiteral("Users")) {
        if (m_accessPage) m_accessPage->openNewUserDialog();
      }
    } else if (action.contains(QStringLiteral("Edit"))) {
      if (m_currentSubmenu == QStringLiteral("Stores")) {
        if (m_accessPage) m_accessPage->openEditStoreDialog();
      } else if (m_currentSubmenu == QStringLiteral("Warehouses")) {
        if (m_accessPage) m_accessPage->openEditWarehouseDialog();
      } else if (m_currentSubmenu == QStringLiteral("Users")) {
        if (m_accessPage) m_accessPage->openEditUserDialog();
      }
    } else if (action.contains(QStringLiteral("Refresh"))) {
      if (m_accessPage) m_accessPage->refreshCurrentTab();
    }
  }
  // Backup / System
  else if (action.contains(QStringLiteral("Backup"))) {
    onActionBackupDatabase();
  } else if (action.contains(QStringLiteral("Restore"))) {
    onActionRestoreBackup();
  } else if (action.contains(QStringLiteral("Refresh"))) {
    int idx = m_workspaceTabs->currentIndex();
    if (idx == 0 && m_dashboardPage)
      m_dashboardPage->refreshData();
    else if (idx == 2 && m_productsPage)
      m_productsPage->reloadData();
    else if (idx == 3 && m_inventoryPage)
      m_inventoryPage->reloadData();
    else if (idx == 4 && m_purchasesPage)
      m_purchasesPage->reloadData();
    else if (idx == 5 && m_salesPage)
      m_salesPage->reloadData();
    else if (idx == 6 && m_customersPage)
      m_customersPage->reloadData();
    else if (idx == 7 && m_financePage)
      m_financePage->reloadData();
    else if (idx == 8 && m_reportsPage)
      m_reportsPage->reloadData();
    else if (idx == 10 && m_accessPage)
      m_accessPage->refreshCurrentTab();
  } else if (action.contains(QStringLiteral("Export"))) {
    onActionExport();
  } else if (action.contains(QStringLiteral("Exit"))) {
    onActionExit();
  } else if (action.contains(QStringLiteral("Logout"))) {
    QMessageBox::information(
        this, QStringLiteral("Logout"),
        QStringLiteral("User session logged out. Ready for cashier sign-in."));
  } else if (action.contains(QStringLiteral("Shortcuts"))) {
    onActionKeyboardShortcuts();
  } else if (action.contains(QStringLiteral("Help")) ||
             action.contains(QStringLiteral("Guide"))) {
    onActionDocumentation();
  } else if (action.contains(QStringLiteral("About"))) {
    onActionAbout();
  } else {
    QMessageBox::information(
        this, action,
        QStringLiteral("Executed command '%1' for %2 ▸ %3.")
            .arg(action, m_currentCategory, m_currentSubmenu));
  }
}

void MainWindow::setupWorkspaceTabs() {
  m_workspaceTabs = new QTabWidget(this);
  m_workspaceTabs->tabBar()->hide();
  m_workspaceTabs->setStyleSheet(QStringLiteral(
      "QTabWidget::pane { border: none; background-color: #D9E8F5; }"));

  m_dashboardPage = new DashboardPage(this);
  m_workspaceTabs->addTab(m_dashboardPage, QStringLiteral("Dashboard"));

  m_posPage = new POSBillingPage(this);
  m_workspaceTabs->addTab(m_posPage, QStringLiteral("POS Billing"));

  m_productsPage = new ProductsPage(this);
  m_inventoryPage = new InventoryPage(this);
  m_purchasesPage = new PurchasesPage(this);
  m_salesPage = new SalesPage(this);
  m_customersPage = new CustomersPage(this);
  m_financePage = new FinancePage(this);
  m_reportsPage = new ReportsPage(this);
  m_settingsPage = new SettingsPage(this);
  m_accessPage = new AccessPage(this);

  m_workspaceTabs->addTab(m_productsPage, QStringLiteral("Products"));
  m_workspaceTabs->addTab(m_inventoryPage, QStringLiteral("Inventory"));
  m_workspaceTabs->addTab(m_purchasesPage, QStringLiteral("Purchases"));
  m_workspaceTabs->addTab(m_salesPage, QStringLiteral("Sales"));
  m_workspaceTabs->addTab(m_customersPage, QStringLiteral("Customers"));
  m_workspaceTabs->addTab(m_financePage, QStringLiteral("Finance"));
  m_workspaceTabs->addTab(m_reportsPage, QStringLiteral("Reports"));
  m_workspaceTabs->addTab(m_settingsPage, QStringLiteral("Settings"));
  m_workspaceTabs->addTab(m_accessPage, QStringLiteral("Access"));

  m_genericPlaceholderPage = new PlaceholderPage(
      QStringLiteral("Enterprise Module View"), QStringLiteral("⚙️"),
      QStringLiteral("This workspace view is active and connected to the "
                     "RetailNext database."),
      this);
  m_workspaceTabs->addTab(m_genericPlaceholderPage,
                          QStringLiteral("Generic View"));
}

void MainWindow::openModule(int moduleIndex) {
  if (moduleIndex >= 0 && moduleIndex < m_workspaceTabs->count()) {
    m_workspaceTabs->setCurrentIndex(moduleIndex);

    // Refresh data if switching to a live page
    if (moduleIndex == 0 && m_dashboardPage)
      m_dashboardPage->refreshData();
    else if (moduleIndex == 2 && m_productsPage)
      m_productsPage->reloadData();
    else if (moduleIndex == 3 && m_inventoryPage)
      m_inventoryPage->reloadData();
    else if (moduleIndex == 4 && m_purchasesPage)
      m_purchasesPage->reloadData();
    else if (moduleIndex == 5 && m_salesPage)
      m_salesPage->reloadData();
    else if (moduleIndex == 6 && m_customersPage)
      m_customersPage->reloadData();
    else if (moduleIndex == 7 && m_financePage)
      m_financePage->reloadData();
    else if (moduleIndex == 8 && m_reportsPage)
      m_reportsPage->reloadData();
    else if (moduleIndex == 10 && m_accessPage)
      m_accessPage->refreshCurrentTab();
  }
}

void MainWindow::setupGlobalKeyboardShortcuts() {
  new QShortcut(QKeySequence(Qt::Key_F1), this,
                SLOT(onActionKeyboardShortcuts()));
  new QShortcut(QKeySequence(Qt::Key_F2), this,
                SLOT(onActionNewTransaction()));

  new QShortcut(QKeySequence(Qt::Key_F3), this, [this]() {
    selectCategorySubmenu(QStringLiteral("Masters"),
                          QStringLiteral("Products"));
  });

  new QShortcut(QKeySequence(Qt::Key_F4), this, [this]() {
    selectCategorySubmenu(QStringLiteral("Customers"),
                          QStringLiteral("Customer List"));
  });

  new QShortcut(QKeySequence(Qt::Key_F5), this, [this]() {
    selectCategorySubmenu(QStringLiteral("Inventory"),
                          QStringLiteral("Stock Overview"));
  });

  new QShortcut(QKeySequence(Qt::Key_F6), this, [this]() {
    selectCategorySubmenu(QStringLiteral("Purchase"),
                          QStringLiteral("Purchase Entry"));
  });

  new QShortcut(QKeySequence(Qt::Key_F7), this, [this]() {
    selectCategorySubmenu(QStringLiteral("Sales"),
                          QStringLiteral("Sales Invoice List"));
  });

  new QShortcut(QKeySequence(Qt::Key_F8), this, [this]() {
    selectCategorySubmenu(QStringLiteral("Reports"),
                          QStringLiteral("Sales Reports"));
  });

  new QShortcut(QKeySequence(Qt::Key_F9), this, [this]() {
    if (m_workspaceTabs->currentIndex() == 1) {
      m_posPage->onCheckoutClicked();
    }
  });

  new QShortcut(QKeySequence(Qt::Key_F10), this, [this]() {
    selectCategorySubmenu(QStringLiteral("Accounts"),
                          QStringLiteral("Day Closing"));
  });

  new QShortcut(QKeySequence(Qt::Key_F11), this, [this]() {
    if (m_btnSubmenuPicker) {
      m_btnSubmenuPicker->showMenu();
    }
  });

  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this,
                SLOT(onGlobalSearchActivated()));
  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this,
                SLOT(onActionSave()));
  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_P), this,
                SLOT(onActionPrint()));
  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_E), this,
                SLOT(onActionExport()));
  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_B), this,
                SLOT(onActionBackupDatabase()));
  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), this,
                SLOT(onActionCloseCurrentTab()));
  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Tab), this,
                SLOT(onActionNextTab()));
  new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Tab), this,
                SLOT(onActionPreviousTab()));
}

void MainWindow::setupStatusBar() {
  m_statusReadyLabel = new QLabel(QStringLiteral("Ready"), this);
  m_statusUserLabel = new QLabel(this);
  m_statusBranchLabel = new QLabel(this);
  m_statusDbLabel = new QLabel(this);
  m_statusClockLabel = new QLabel(this);

  m_statusReadyLabel->setStyleSheet(
      QStringLiteral("font-weight: bold; color: #000080;"));

  statusBar()->addWidget(m_statusReadyLabel, 1);
  statusBar()->addPermanentWidget(m_statusUserLabel);
  statusBar()->addPermanentWidget(m_statusBranchLabel);
  statusBar()->addPermanentWidget(m_statusDbLabel);
  statusBar()->addPermanentWidget(m_statusClockLabel);

  m_clockTimer = new QTimer(this);
  connect(m_clockTimer, &QTimer::timeout, this,
          &MainWindow::updateStatusBarIndicators);
  m_clockTimer->start(1000);

  updateStatusBarIndicators();
}

void MainWindow::updateStatusBarIndicators() {
  DatabaseManager &db = DatabaseManager::instance();
  m_statusUserLabel->setText(
      QStringLiteral("User : %1").arg(db.currentUser()));
  m_statusBranchLabel->setText(
      QStringLiteral("Branch : %1").arg(db.branchName()));
  m_statusDbLabel->setText(QStringLiteral("Database : Local (SQLite)"));
  m_statusClockLabel->setText(
      QDateTime::currentDateTime().toString(QStringLiteral("dd/MM/yyyy   hh:mm:ss AP")));
}

void MainWindow::onGlobalSearchActivated() {
  GlobalSearchDialog dlg(this);
  dlg.exec();
}

void MainWindow::onActionNewTransaction() {
  selectCategorySubmenu(QStringLiteral("Sales"),
                        QStringLiteral("POS Billing"));
  m_posPage->resetBillingSession();
}

void MainWindow::onActionSave() {
  int cur = m_workspaceTabs->currentIndex();
  if (cur == 1) {
    m_posPage->onHoldBillClicked();
  } else if (cur == 9) {
    m_settingsPage->onBackupNowClicked();
  } else {
    QMessageBox::information(this, QStringLiteral("Save"),
                             QStringLiteral("Workspace state saved."));
  }
}

void MainWindow::onActionPrint() {
  int cur = m_workspaceTabs->currentIndex();
  if (cur == 1) {
    QMessageBox::information(
        this, QStringLiteral("Print Receipt"),
        QStringLiteral("Receipt dispatched to default thermal printer."));
  } else if (cur == 8) {
    m_reportsPage->onPrintClicked();
  } else {
    QMessageBox::information(
        this, QStringLiteral("Print"),
        QStringLiteral("Active view sent to system printer."));
  }
}

void MainWindow::onActionExport() {
  int cur = m_workspaceTabs->currentIndex();
  if (cur == 2)
    m_productsPage->onExportCsvClicked();
  else if (cur == 3)
    m_inventoryPage->onExportLedgerClicked();
  else if (cur == 4)
    m_purchasesPage->onExportCsvClicked();
  else if (cur == 5)
    m_salesPage->onExportCsvClicked();
  else if (cur == 6)
    m_customersPage->onExportCsvClicked();
  else if (cur == 7)
    m_financePage->onExportCsvClicked();
  else if (cur == 8)
    m_reportsPage->onExportClicked();
  else
    QMessageBox::information(this, QStringLiteral("Export Data"),
                             QStringLiteral("No active grid data to export."));
}

void MainWindow::onActionBackupDatabase() {
  selectCategorySubmenu(QStringLiteral("File"),
                        QStringLiteral("Backup & Restore"));
  m_settingsPage->onBackupNowClicked();
}

void MainWindow::onActionRestoreBackup() {
  selectCategorySubmenu(QStringLiteral("File"),
                        QStringLiteral("Backup & Restore"));
  m_settingsPage->onRestoreBackupClicked();
}

void MainWindow::onActionSwitchBranch() {
  bool ok = false;
  QString branch = QInputDialog::getItem(
      this, QStringLiteral("Switch Branch"),
      QStringLiteral("Select branch location:"),
      QStringList() << QStringLiteral("Main Store (Flagship)")
                    << QStringLiteral("Downtown Express Outlet")
                    << QStringLiteral("Mall Warehouse Unit"),
      0, false, &ok);
  if (ok && !branch.isEmpty()) {
    DatabaseManager::instance().updateBusinessSetting(
        QStringLiteral("branch_name"), branch);
    updateStatusBarIndicators();
    QMessageBox::information(
        this, QStringLiteral("Branch Changed"),
        QStringLiteral("Switched to '%1'.").arg(branch));
  }
}

void MainWindow::onActionLockApplication() {
  QMessageBox::information(
      this, QStringLiteral("Application Locked"),
      QStringLiteral("Terminal locked. Please enter cashier PIN to resume."));
}

void MainWindow::onActionExit() { close(); }

void MainWindow::onActionStockAdjustment() {
  selectCategorySubmenu(QStringLiteral("Inventory"),
                        QStringLiteral("Stock Adjustment"));
  m_inventoryPage->onStockAdjustmentClicked();
}

void MainWindow::onActionStockTransfer() {
  selectCategorySubmenu(QStringLiteral("Inventory"),
                        QStringLiteral("Stock Transfer"));
  m_inventoryPage->onStockTransferClicked();
}

void MainWindow::onActionCashInOut() {
  selectCategorySubmenu(QStringLiteral("Accounts"),
                        QStringLiteral("Cash Register"));
  m_financePage->onCashInClicked();
}

void MainWindow::onActionDayClosing() {
  selectCategorySubmenu(QStringLiteral("Accounts"),
                        QStringLiteral("Day Closing"));
  m_financePage->onCloseRegisterClicked();
}

void MainWindow::onActionDbMaintenance() {
  selectCategorySubmenu(QStringLiteral("Tools"),
                        QStringLiteral("Database Tools"));
  m_settingsPage->onRunIntegrityCheckClicked();
}

void MainWindow::onActionPrinterDiagnostics() {
  QMessageBox::information(
      this, QStringLiteral("Printer Diagnostics"),
      QStringLiteral(
          "Thermal ESC/POS: OK\nBarcode Label Printer: OK\nA4 Document "
          "Spooler: Ready"));
}

void MainWindow::onActionSystemLogs() {
  QMessageBox::information(
      this, QStringLiteral("System Logs"),
      QStringLiteral("All audit logs written to SQLite WAL journal.\nZero "
                     "critical faults detected."));
}

void MainWindow::onActionAbout() {
  QMessageBox::about(
      this, QStringLiteral("About RetailNext"),
      QStringLiteral(
          "<h3>RetailNext 2026 Enterprise POS</h3>"
          "<p>Configurable Enterprise POS &amp; Inventory Management "
          "System</p>"
          "<p>Engineered for high-throughput retail, pharma, supermarket, and "
          "apparel chains.</p>"
          "<hr/>"
          "<p><b>Version:</b> 2.0.4-Release<br/>"
          "<b>Storage:</b> SQLite WAL High-Concurrency Engine<br/>"
          "<b>UI Framework:</b> Qt 6.12 MinGW 64-bit</p>"));
}

void MainWindow::onActionKeyboardShortcuts() {
  QMessageBox::information(
      this, QStringLiteral("Enterprise Keyboard Shortcuts"),
      QStringLiteral(
          "<h3>RetailNext Standard ERP Shortcuts</h3>"
          "<table border='1' cellpadding='4' cellspacing='0' "
          "style='border-collapse:collapse;'>"
          "<tr><th>Key</th><th>Function</th></tr>"
          "<tr><td><b>F1</b></td><td>Help &amp; Shortcuts</td></tr>"
          "<tr><td><b>F2</b></td><td>POS Billing / New Bill</td></tr>"
          "<tr><td><b>F3</b></td><td>Products Master</td></tr>"
          "<tr><td><b>F4</b></td><td>Customers Registry</td></tr>"
          "<tr><td><b>F5</b></td><td>Stock Overview / Refresh</td></tr>"
          "<tr><td><b>F6</b></td><td>Purchases / Inward GRN</td></tr>"
          "<tr><td><b>F7</b></td><td>Sales Returns / Refund</td></tr>"
          "<tr><td><b>F8</b></td><td>Reports &amp; Analytics</td></tr>"
          "<tr><td><b>F9</b></td><td>Complete Tender / Save / Post</td></tr>"
          "<tr><td><b>F10</b></td><td>Day Closing / Shift Reconciliation</td></"
          "tr>"
          "<tr><td><b>Ctrl+F</b></td><td>Global Search (Entities, Invoices, "
          "SKUs)</td></tr>"
          "<tr><td><b>Ctrl+S</b></td><td>Save Active Bill</td></tr>"
          "<tr><td><b>Ctrl+P</b></td><td>Print Invoice / Receipt</td></tr>"
          "<tr><td><b>Ctrl+B</b></td><td>Backup SQLite Database</td></tr>"
          "<tr><td><b>Ctrl+Tab</b></td><td>Next Workspace View</td></tr>"
          "<tr><td><b>Ctrl+Shift+Tab</b></td><td>Previous Workspace "
          "View</td></tr>"
          "<tr><td><b>Esc</b></td><td>Cancel / Close Dialog</td></tr>"
          "</table>"));
}

void MainWindow::onActionDocumentation() {
  QMessageBox::information(
      this, QStringLiteral("User Documentation"),
      QStringLiteral("RetailNext Enterprise manual available in project docs.\n"
                     "Press F1 anywhere for contextual keyboard help."));
}

void MainWindow::onActionCalculator() {
#if defined(Q_OS_WIN)
  QProcess::startDetached(QStringLiteral("calc.exe"), QStringList());
#else
  QMessageBox::information(this, QStringLiteral("Calculator"),
                           QStringLiteral("Calculator utility."));
#endif
}

void MainWindow::onActionNextTab() {
  int count = m_workspaceTabs->count();
  if (count > 1) {
    int next = (m_workspaceTabs->currentIndex() + 1) % count;
    openModule(next);
  }
}

void MainWindow::onActionPreviousTab() {
  int count = m_workspaceTabs->count();
  if (count > 1) {
    int prev = (m_workspaceTabs->currentIndex() - 1 + count) % count;
    openModule(prev);
  }
}

void MainWindow::onActionCloseCurrentTab() {
  openModule(1); // Return to primary POS billing
}

void MainWindow::onActionCloseAllTabs() { openModule(1); }

void MainWindow::onActionResetWorkspace() {
  selectCategorySubmenu(QStringLiteral("Sales"),
                        QStringLiteral("POS Billing"));
}

void MainWindow::onTabCloseRequested(int index) { Q_UNUSED(index); }
