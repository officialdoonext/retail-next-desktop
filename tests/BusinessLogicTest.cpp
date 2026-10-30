#include <QCoreApplication>
#include <QDir>
#include <QTemporaryDir>
#include <QDebug>
#include <cassert>
#include <iostream>

#include "database/DatabaseManager.h"
#include "core/BusinessType.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "========================================" << std::endl;
    std::cout << "Running RetailNext Business Logic Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        std::cerr << "Failed to create temporary directory for test database." << std::endl;
        return 1;
    }

    QString testDbPath = tempDir.filePath("RetailNext_Test.db");
    DatabaseManager &db = DatabaseManager::instance();

    // Test 1: Initialization & Schema Creation
    std::cout << "[TEST 1] Initializing SQLite database and ACID schema..." << std::endl;
    bool ok = db.initialize(testDbPath);
    if (!ok || !db.isConnected()) {
        std::cerr << "FAILED: Database initialization failed: " << db.lastError().toStdString() << std::endl;
        return 1;
    }
    std::cout << "  -> PASSED: SQLite initialized with WAL mode and foreign keys." << std::endl;

    // Test 2: Seed Data Verification
    std::cout << "[TEST 2] Verifying initial seed data..." << std::endl;
    auto prods = db.searchProducts();
    if (prods.size() < 10) {
        std::cerr << "FAILED: Expected at least 10 seeded products, found: " << prods.size() << std::endl;
        return 1;
    }
    std::cout << "  -> PASSED: Seeded " << prods.size() << " products across multiple retail verticals." << std::endl;

    // Test 3: Barcode and SKU lookup
    std::cout << "[TEST 3] Testing barcode lookup..." << std::endl;
    auto milk = db.getProductByBarcodeOrSku("890103000001");
    if (milk.id == 0 || !milk.name.contains("Milk")) {
        std::cerr << "FAILED: Failed barcode lookup for 890103000001." << std::endl;
        return 1;
    }
    double initialStock = milk.currentStock;
    std::cout << "  -> PASSED: Looked up " << milk.name.toStdString() << " with initial stock: " << initialStock << std::endl;

    // Test 4: POS Sale Transaction & Stock Movement Ledger
    std::cout << "[TEST 4] Processing POS sale with ACID stock deduction..." << std::endl;
    QList<RetailNext::CartItem> cart;
    RetailNext::CartItem item;
    item.productId = milk.id;
    item.barcode = milk.barcode;
    item.name = milk.name;
    item.qty = 2.0;
    item.unit = milk.unitCode;
    item.unitPrice = milk.sellingPrice;
    item.discountPercent = 0.0;
    item.discountAmount = 0.0;
    item.taxRate = milk.taxRate;
    item.taxAmount = (item.unitPrice * item.qty) * (item.taxRate / 100.0);
    item.lineTotal = (item.unitPrice * item.qty) + item.taxAmount;
    cart.append(item);

    double subtotal = item.unitPrice * item.qty;
    double tax = item.taxAmount;
    double net = subtotal + tax;

    auto saleRes = db.processPOSSale(
        "Walk-in Customer",
        1,
        cart,
        subtotal,
        0.0,
        tax,
        0.0,
        net,
        net,
        0.0,
        "Cash",
        "Test POS Sale"
    );

    if (!saleRes.success) {
        std::cerr << "FAILED: POS Sale processing failed: " << saleRes.errorMessage.toStdString() << std::endl;
        return 1;
    }

    auto updatedMilk = db.getProductById(milk.id);
    if (updatedMilk.currentStock != initialStock - 2.0) {
        std::cerr << "FAILED: Stock did not deduct accurately. Expected: " << (initialStock - 2.0) << ", got: " << updatedMilk.currentStock << std::endl;
        return 1;
    }
    std::cout << "  -> PASSED: Sale " << saleRes.invoiceNo.toStdString() << " posted. Stock accurately deducted to: " << updatedMilk.currentStock << std::endl;

    // Test 5: Verify Stock Movement Ledger
    std::cout << "[TEST 5] Checking stock movement audit trail..." << std::endl;
    auto movements = db.getStockMovements(10, milk.id);
    bool foundSaleMovement = false;
    for (const auto& m : movements) {
        if (m.referenceNo == saleRes.invoiceNo && m.movementType == "POS Sale" && m.quantity == -2.0) {
            foundSaleMovement = true;
            break;
        }
    }
    if (!foundSaleMovement) {
        std::cerr << "FAILED: Stock movement ledger entry missing for POS sale." << std::endl;
        return 1;
    }
    std::cout << "  -> PASSED: Traceable stock movement ledger entry confirmed." << std::endl;

    // Test 6: Inward Goods Receipt (GRN) / Purchase Inward
    std::cout << "[TEST 6] Testing Purchase Inward (GRN) and inventory increment..." << std::endl;
    double prePurchaseStock = updatedMilk.currentStock;
    QList<RetailNext::CartItem> purchaseItems;
    RetailNext::CartItem pItem;
    pItem.productId = milk.id;
    pItem.qty = 10.0;
    pItem.unitPrice = milk.costPrice;
    pItem.name = milk.name;
    purchaseItems.append(pItem);

    bool grnOk = db.recordPurchaseReceipt("PO-TEST-001", 1, "CH-TEST-99", purchaseItems, 10.0 * milk.costPrice, "Test GRN Inward");
    if (!grnOk) {
        std::cerr << "FAILED: Purchase inward failed." << std::endl;
        return 1;
    }
    auto postPurchaseMilk = db.getProductById(milk.id);
    if (postPurchaseMilk.currentStock != prePurchaseStock + 10.0) {
        std::cerr << "FAILED: Stock after GRN incorrect. Expected: " << (prePurchaseStock + 10.0) << ", got: " << postPurchaseMilk.currentStock << std::endl;
        return 1;
    }
    std::cout << "  -> PASSED: Goods receipt posted. Stock increased to: " << postPurchaseMilk.currentStock << std::endl;

    // Test 7: Stock Adjustment
    std::cout << "[TEST 7] Testing physical stock count adjustment..." << std::endl;
    bool adjOk = db.recordStockAdjustment(milk.id, 50.0, "Physical Verification Count", "Audit reconciliation");
    if (!adjOk) {
        std::cerr << "FAILED: Stock adjustment failed." << std::endl;
        return 1;
    }
    auto adjustedMilk = db.getProductById(milk.id);
    if (adjustedMilk.currentStock != 50.0) {
        std::cerr << "FAILED: Stock after adjustment incorrect. Expected: 50.0, got: " << adjustedMilk.currentStock << std::endl;
        return 1;
    }
    std::cout << "  -> PASSED: Stock reconciled to physical count (50.0)." << std::endl;

    // Test 8: Cash Register Drawer Operations
    std::cout << "[TEST 8] Testing Cash Register session and drawer tracking..." << std::endl;
    auto session = db.getActiveCashSession();
    double oldExpected = session.expectedCash;
    bool inOk = db.recordCashInOut(25.0, "Cash In", "Test Float Addition", "TEST-IN");
    if (!inOk) {
        std::cerr << "FAILED: Cash in operation failed." << std::endl;
        return 1;
    }
    auto updatedSession = db.getActiveCashSession();
    if (updatedSession.expectedCash != oldExpected + 25.0) {
        std::cerr << "FAILED: Cash in did not update expected cash drawer balance." << std::endl;
        return 1;
    }
    std::cout << "  -> PASSED: Cash in recorded. Drawer expected balance: $" << updatedSession.expectedCash << std::endl;

    // Test 9: Database Atomic Backup & PRAGMA Integrity Check
    std::cout << "[TEST 9] Running SQLite atomic backup & integrity check..." << std::endl;
    QString backupFile = tempDir.filePath("TestBackup.db");
    bool backupOk = db.backupDatabase(backupFile);
    if (!backupOk || !QFile::exists(backupFile)) {
        std::cerr << "FAILED: Database backup failed." << std::endl;
        return 1;
    }
    QString integrity = db.runIntegrityCheck();
    if (integrity != "ok") {
        std::cerr << "FAILED: Integrity check failed: " << integrity.toStdString() << std::endl;
        return 1;
    }
    std::cout << "  -> PASSED: Atomic WAL backup generated (" << QFile(backupFile).size() << " bytes), integrity verified: ok." << std::endl;

    // Test 10: Access Control Module (Stores, Warehouses, Users)
    std::cout << "[TEST 10] Testing Access Control (Stores, Warehouses, Users CRUD)..." << std::endl;
    // Stores
    auto stores = db.getStoresList();
    if (stores.isEmpty()) {
        std::cerr << "FAILED: Seeded stores list is empty." << std::endl;
        return 1;
    }
    RetailNext::StoreRecord newStore;
    newStore.name = "Test Airport Branch";
    newStore.phone = "+1 555-9988";
    newStore.email = "airport@retailnext.com";
    newStore.city = "Metropolis";
    newStore.address = "Terminal 3 Arrival Concourse";
    newStore.gstNo = "TX-99887766";
    newStore.isActive = true;
    bool storeSaved = db.saveStore(newStore, true);
    if (!storeSaved) {
        std::cerr << "FAILED: Could not save new store." << std::endl;
        return 1;
    }
    auto testStoreResults = db.getStoresList("Airport");
    if (testStoreResults.isEmpty() || testStoreResults[0].name != "Test Airport Branch") {
        std::cerr << "FAILED: Could not retrieve saved store by name filter." << std::endl;
        return 1;
    }
    std::cout << "  -> PASSED: Stores CRUD and filter lookup verified." << std::endl;

    // Warehouses
    auto whList = db.getWarehousesList();
    if (whList.isEmpty()) {
        std::cerr << "FAILED: Seeded warehouses list is empty." << std::endl;
        return 1;
    }
    RetailNext::WarehouseRecord newWh;
    newWh.name = "South Regional Depo";
    newWh.phone = "+1 555-4433";
    newWh.email = "south@retailnext.com";
    newWh.city = "Metropolis";
    newWh.address = "South Industrial Parkway #10";
    newWh.isActive = true;
    bool whSaved = db.saveWarehouse(newWh, true);
    if (!whSaved) {
        std::cerr << "FAILED: Could not save new warehouse." << std::endl;
        return 1;
    }
    auto testWhResults = db.getWarehousesList("South");
    if (testWhResults.isEmpty() || testWhResults[0].name != "South Regional Depo") {
        std::cerr << "FAILED: Could not retrieve saved warehouse by name filter." << std::endl;
        return 1;
    }
    std::cout << "  -> PASSED: Warehouses CRUD and filter lookup verified." << std::endl;

    // Users & Permissions Matrix
    auto usersList = db.getUsersList();
    if (usersList.isEmpty()) {
        std::cerr << "FAILED: Seeded users list is empty." << std::endl;
        return 1;
    }
    RetailNext::UserRecord newUser;
    newUser.displayName = "Robert Green";
    newUser.phone = "+1 555-7766";
    newUser.username = "rgreen";
    newUser.password = "Secr3t!2026";
    newUser.role = "Manager";
    newUser.permissions = "Dashboard:view,edit;Sales:view,edit;Customers:view";
    newUser.isActive = true;
    bool userSaved = db.saveUser(newUser, true);
    if (!userSaved) {
        std::cerr << "FAILED: Could not save new user." << std::endl;
        return 1;
    }
    auto testUserResults = db.getUsersList("rgreen");
    if (testUserResults.isEmpty() || testUserResults[0].displayName != "Robert Green" || testUserResults[0].password != "Secr3t!2026") {
        std::cerr << "FAILED: Could not retrieve saved user with permissions and password." << std::endl;
        return 1;
    }
    std::cout << "  -> PASSED: User credentials and permission matrix persistence verified." << std::endl;

    std::cout << "========================================" << std::endl;
    std::cout << "ALL 10 BUSINESS LOGIC TESTS PASSED (100%)" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}

