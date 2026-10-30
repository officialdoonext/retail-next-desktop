#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "../core/BusinessType.h"
#include <QList>
#include <QPair>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>

namespace RetailNext {

struct ProductItem {
  int id = 0;
  QString sku;
  QString barcode;
  QString name;
  int categoryId = 1;
  QString categoryName;
  int brandId = 1;
  QString brandName;
  int unitId = 1;
  QString unitCode = "PCS";
  double costPrice = 0.0;
  double sellingPrice = 0.0;
  double mrp = 0.0;
  double wholesalePrice = 0.0;
  double taxRate = 0.0;
  double currentStock = 0.0;
  double reorderLevel = 5.0;
  bool isActive = true;
  QString businessTypeTag = "GeneralRetail";
};

struct CartItem {
  int productId = 0;
  int variantId = 0;
  QString barcode;
  QString name;
  QString variantName;
  double qty = 1.0;
  QString unit = "PCS";
  double unitPrice = 0.0;
  double discountPercent = 0.0;
  double discountAmount = 0.0;
  double taxRate = 0.0;
  double taxAmount = 0.0;
  double lineTotal = 0.0;
};

struct POSSaleResult {
  bool success = false;
  QString invoiceNo;
  QString errorMessage;
};

struct DashboardSummary {
  double grossSales = 0.0;
  double totalDiscounts = 0.0;
  double netSales = 0.0;
  int transactionCount = 0;
  double avgBillValue = 0.0;
  double itemsSold = 0.0;
  double estimatedProfit = 0.0;
  double profitMargin = 0.0;
  double cashSales = 0.0;
  double cardSales = 0.0;
  double upiSales = 0.0;
  double creditSales = 0.0;
  double totalStockValue = 0.0;
  int lowStockCount = 0;
  int outOfStockCount = 0;
  double receivables = 0.0;
  double payables = 0.0;
};

struct CustomerRecord {
  int id = 0;
  QString code;
  QString name;
  QString phone;
  QString email;
  QString address;
  QString gstNo;
  QString customerGroup = "Retail";
  int loyaltyPoints = 0;
  double storeCredit = 0.0;
  double outstandingBalance = 0.0;
  double creditLimit = 0.0;
  bool isActive = true;
};

struct SupplierRecord {
  int id = 0;
  QString code;
  QString name;
  QString contactPerson;
  QString phone;
  QString email;
  QString address;
  QString gstNo;
  QString paymentTerms = "Net 30";
  double creditLimit = 0.0;
  double outstandingBalance = 0.0;
  bool isActive = true;
};

struct StockMovementRecord {
  int id = 0;
  int productId = 0;
  QString productName;
  QString sku;
  QString referenceNo;
  QString movementType;
  double quantity = 0.0;
  double unitCost = 0.0;
  double balanceAfter = 0.0;
  QString userName;
  QString notes;
  QString createdAt;
};

struct ExpenseRecord {
  int id = 0;
  QString expenseNo;
  QString categoryName;
  QString payee;
  double amount = 0.0;
  double taxAmount = 0.0;
  QString paymentMethod;
  QString branch = "Main Store";
  QString notes;
  QString createdBy;
  QString createdAt;
};

struct CashSessionRecord {
  int id = 0;
  int registerId = 1;
  QString cashierName;
  QString openedAt;
  QString closedAt;
  double openingCash = 0.0;
  double cashSales = 0.0;
  double cashIn = 0.0;
  double cashOut = 0.0;
  double expectedCash = 0.0;
  double countedCash = 0.0;
  double variance = 0.0;
  QString status = "Open";
};

struct SaleInvoiceSummary {
  int id = 0;
  QString invoiceNo;
  QString createdAt;
  QString cashierName;
  QString customerName;
  double subtotal = 0.0;
  double discountAmount = 0.0;
  double taxAmount = 0.0;
  double roundOff = 0.0;
  double netPayable = 0.0;
  double tenderedAmount = 0.0;
  double changeDue = 0.0;
  QString paymentMode;
  QString paymentStatus;
  int itemCount = 0;
};

struct PurchaseInvoiceRecord {
  int id = 0;
  QString invoiceNo;
  QString supplierInvoiceNo;
  QString supplierName;
  QString invoiceDate;
  double subtotal = 0.0;
  double taxAmount = 0.0;
  double totalAmount = 0.0;
  QString paymentStatus;
};

struct StoreRecord {
  int id = 0;
  QString code;
  QString name;
  QString phone;
  QString email;
  QString city;
  QString address;
  QString gstNo;
  bool isActive = true;
  QString createdAt;
};

struct WarehouseRecord {
  int id = 0;
  QString code;
  QString name;
  QString phone;
  QString email;
  QString city;
  QString address;
  bool isActive = true;
  QString createdAt;
};

struct UserRecord {
  int id = 0;
  QString username;
  QString displayName;
  QString phone;
  QString password;
  QString role = "Cashier";
  QString permissions; // e.g. "Dashboard:view,edit;Sales:view,edit;..."
  bool isActive = true;
  QString createdAt;
};

} // namespace RetailNext

class DatabaseManager {
public:
  static DatabaseManager &instance();

  bool initialize(const QString &dbPath = QString());
  bool isConnected() const;
  QString lastError() const;
  QString databasePath() const;
  QString currentUser() const;
  QString currentRole() const;
  void setCurrentUser(const QString &user, const QString &role);

  // Business settings helpers
  RetailNext::BusinessType businessType() const;
  void setBusinessType(RetailNext::BusinessType type);
  QString storeName() const;
  QString currencySymbol() const;
  QString taxMode() const;
  QString branchName() const;
  QString registerCode() const;
  bool updateBusinessSetting(const QString &key, const QString &val);
  QString getBusinessSetting(const QString &key,
                             const QString &defaultVal = "") const;

  // Dashboard
  RetailNext::DashboardSummary
  getDashboardSummary(const QString &period = "today");
  QList<RetailNext::SaleInvoiceSummary> getRecentInvoices(int limit = 10);
  QList<RetailNext::ProductItem> getTopSellingProducts(int limit = 6);
  QList<RetailNext::ProductItem> getLowStockAlerts(int limit = 10);

  // Products
  QList<RetailNext::ProductItem> searchProducts(const QString &text = "",
                                                int categoryId = 0,
                                                int stockFilter = 0);
  RetailNext::ProductItem getProductByBarcodeOrSku(const QString &code);
  RetailNext::ProductItem getProductById(int id);
  bool saveProduct(const RetailNext::ProductItem &prod, bool isNew = false);
  bool deleteProduct(int id);
  bool bulkUpdatePrices(int categoryId, double percentChange);
  QStringList getCategoriesList();
  QStringList getBrandsList();
  QStringList getUnitsList();

  // POS & Sales
  RetailNext::POSSaleResult
  processPOSSale(const QString &customerName, int customerId,
                 const QList<RetailNext::CartItem> &items, double subtotal,
                 double discount, double tax, double roundOff,
                 double netPayable, double tendered, double change,
                 const QString &paymentMode, const QString &notes);
  bool holdBill(const QString &customerName,
                const QList<RetailNext::CartItem> &items, double total);
  QList<QPair<int, QString>> getHeldBills();
  QList<RetailNext::CartItem> recallHeldBill(int holdId);
  bool deleteHeldBill(int holdId);
  QList<RetailNext::SaleInvoiceSummary>
  searchInvoices(const QString &query = "", const QString &dateFrom = "",
                 const QString &dateTo = "");
  QList<RetailNext::CartItem> getInvoiceItems(int invoiceId);
  bool processSalesReturn(int invoiceId, const QString &invoiceNo,
                          int customerId, double refundAmount,
                          const QString &reason, const QString &refundMode);

  // Inventory
  QList<RetailNext::StockMovementRecord> getStockMovements(int limit = 100,
                                                           int productId = 0);
  bool recordStockAdjustment(int productId, double countedQty,
                             const QString &reason, const QString &notes);
  bool recordStockTransfer(int productId, const QString &fromBranch,
                           const QString &toBranch, double qty,
                           const QString &notes);

  // Customers
  QList<RetailNext::CustomerRecord> searchCustomers(const QString &query = "",
                                                    const QString &group = "");
  RetailNext::CustomerRecord getCustomerById(int id);
  bool saveCustomer(const RetailNext::CustomerRecord &cust, bool isNew = false);
  bool recordCustomerPayment(int customerId, double amount,
                             const QString &paymentMode, const QString &notes);

  // Suppliers & Purchases
  QList<RetailNext::SupplierRecord> searchSuppliers(const QString &query = "");
  RetailNext::SupplierRecord getSupplierById(int id);
  bool saveSupplier(const RetailNext::SupplierRecord &supp, bool isNew = false);
  bool recordPurchaseReceipt(const QString &poNumber, int supplierId,
                             const QString &supplierInvNo,
                             const QList<RetailNext::CartItem> &items,
                             double totalCost, const QString &notes);
  bool recordSupplierPayment(int supplierId, double amount,
                             const QString &paymentMode, const QString &notes);
  QList<RetailNext::PurchaseInvoiceRecord> getPurchaseInvoices();

  // Finance & Cash Register
  RetailNext::CashSessionRecord getActiveCashSession();
  bool recordCashInOut(double amount, const QString &type,
                       const QString &reason, const QString &reference);
  bool closeCashRegisterSession(int sessionId, double countedCash,
                                const QString &notes);
  QList<RetailNext::ExpenseRecord> getExpenses(const QString &period = "");
  bool recordExpense(const QString &category, const QString &payee,
                     double amount, double tax, const QString &method,
                     const QString &notes);

  // Maintenance & Backup
  bool backupDatabase(const QString &destPath);
  bool restoreDatabase(const QString &srcPath);
  QString runIntegrityCheck();

  // Access Module: Stores, Warehouses, Users
  QList<RetailNext::StoreRecord> getStoresList(const QString &query = "");
  RetailNext::StoreRecord getStoreById(int id);
  bool saveStore(const RetailNext::StoreRecord &store, bool isNew = false);
  bool deleteStore(int id);

  QList<RetailNext::WarehouseRecord> getWarehousesList(const QString &query = "");
  RetailNext::WarehouseRecord getWarehouseById(int id);
  bool saveWarehouse(const RetailNext::WarehouseRecord &wh, bool isNew = false);
  bool deleteWarehouse(int id);

  QList<RetailNext::UserRecord> getUsersList(const QString &query = "");
  RetailNext::UserRecord getUserById(int id);
  bool saveUser(const RetailNext::UserRecord &user, bool isNew = false);
  bool deleteUser(int id);

private:
  DatabaseManager();
  ~DatabaseManager();
  DatabaseManager(const DatabaseManager &) = delete;
  DatabaseManager &operator=(const DatabaseManager &) = delete;

  bool createTables();
  bool seedInitialData();

  mutable QSqlDatabase m_db;
  QString m_dbPath;
  QString m_lastError;
  QString m_currentUser;
  QString m_currentRole;
  RetailNext::BusinessType m_businessType;
  QString m_storeName;
  bool m_connected;
};

#endif // DATABASEMANAGER_H
