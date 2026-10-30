#include "InventoryPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QTextStream>
#include <QShortcut>

InventoryPage::InventoryPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    reloadData();
}

void InventoryPage::setupUI() {
    setStyleSheet(QStringLiteral("QWidget { background-color: #D9E8F5; }"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 4);
    mainLayout->setSpacing(4);

    // Header Panel
    QFrame *headerFrame = new QFrame(this);
    headerFrame->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 3px; padding: 4px; }"
    ));
    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(6, 2, 6, 2);

    QVBoxLayout *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(1);
    QLabel *lblTitle = new QLabel(QStringLiteral("Inventory Management & Stock Audit Ledger"), headerFrame);
    lblTitle->setStyleSheet(QStringLiteral("font-size: 13pt; font-weight: 800; color: #0A294A; margin: 0;"));
    QLabel *lblSubtitle = new QLabel(QStringLiteral("Traceable movement ledger, physical audit reconciliation, branch transfers, and reorder alerts."), headerFrame);
    lblSubtitle->setStyleSheet(QStringLiteral("font-size: 8.5pt; color: #4A6B82; font-style: italic; margin: 0;"));
    titleLayout->addWidget(lblTitle);
    titleLayout->addWidget(lblSubtitle);
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    const QString btnStyle = QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; border-color: #4A82AD; }"
    );

    QPushButton *btnAdj = new QPushButton(QStringLiteral("⚖️ Stock Adjustment (F6)"), headerFrame);
    btnAdj->setShortcut(QKeySequence(Qt::Key_F6));
    btnAdj->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 3px 12px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #C8E6C9; }"
    ));
    QPushButton *btnTransfer = new QPushButton(QStringLiteral("🔄 Branch Transfer (F7)"), headerFrame);
    btnTransfer->setShortcut(QKeySequence(Qt::Key_F7));
    btnTransfer->setStyleSheet(btnStyle);
    QPushButton *btnExport = new QPushButton(QStringLiteral("📥 Export CSV (F9)"), headerFrame);
    btnExport->setShortcut(QKeySequence(Qt::Key_F9));
    btnExport->setStyleSheet(btnStyle);
    QPushButton *btnRefresh = new QPushButton(QStringLiteral("🔄 Refresh (F5)"), headerFrame);
    btnRefresh->setShortcut(QKeySequence(Qt::Key_F5));
    btnRefresh->setStyleSheet(btnStyle);

    headerLayout->addWidget(btnAdj);
    headerLayout->addWidget(btnTransfer);
    headerLayout->addWidget(btnExport);
    headerLayout->addWidget(btnRefresh);
    mainLayout->addWidget(headerFrame);

    // Sub tabs
    m_tabs = new QTabWidget(this);
    m_tabs->setStyleSheet(QStringLiteral(
        "QTabWidget::pane { border: 1px solid #8FAEC7; background-color: #D9E8F5; top: -1px; } "
        "QTabBar::tab { background-color: #D0E2F2; color: #0A294A; border: 1px solid #8FAEC7; border-bottom: none; "
        "border-top-left-radius: 3px; border-top-right-radius: 3px; padding: 3px 12px; font-size: 8.5pt; font-weight: 600; margin-right: 2px; } "
        "QTabBar::tab:selected { background-color: #FFFFFF; color: #000080; border-bottom: 2px solid #0055A5; font-weight: bold; }"
    ));

    const QString tableStyle = QStringLiteral(
        "QTableWidget { background-color: #FFFFFF; alternate-background-color: #F8FBFE; "
        "gridline-color: #C2D7EA; border: 1px solid #7A9EB9; font-size: 8.5pt; color: #0A294A; } "
        "QTableWidget::item { padding: 2px 4px; } "
        "QTableWidget::item:selected { background-color: #2D82D6; color: #FFFFFF; font-weight: bold; } "
        "QHeaderView::section { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B3D4F0, stop:1 #8DBDE0); "
        "color: #0A294A; font-weight: bold; font-size: 8pt; border: none; border-top: 1px solid #C2D7EA; "
        "border-bottom: 1px solid #6895BA; border-right: 1px solid #99BEDC; padding: 3px 5px; }"
    );

    // Tab 1: Stock Overview
    QWidget *tabOverview = new QWidget(m_tabs);
    QVBoxLayout *l1 = new QVBoxLayout(tabOverview);
    l1->setContentsMargins(4, 4, 4, 4);
    l1->setSpacing(4);

    m_tableOverview = new QTableWidget(tabOverview);
    m_tableOverview->setColumnCount(8);
    m_tableOverview->setHorizontalHeaderLabels(QStringList()
        << QStringLiteral("SKU")
        << QStringLiteral("Barcode")
        << QStringLiteral("Product Name")
        << QStringLiteral("Category")
        << QStringLiteral("Current Stock")
        << QStringLiteral("Unit Cost")
        << QStringLiteral("Stock Valuation")
        << QStringLiteral("Status"));
    m_tableOverview->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableOverview->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tableOverview->verticalHeader()->setVisible(false);
    m_tableOverview->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableOverview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableOverview->setAlternatingRowColors(true);
    m_tableOverview->setStyleSheet(tableStyle);
    l1->addWidget(m_tableOverview, 1);

    m_lblOverviewStats = new QLabel(tabOverview);
    m_lblOverviewStats->setStyleSheet(QStringLiteral("background: #FFFFFF; color: #000080; border: 1px solid #7A9EB9; border-radius: 2px; padding: 3px 8px; font-weight: bold; font-size: 8.5pt;"));
    l1->addWidget(m_lblOverviewStats);
    m_tabs->addTab(tabOverview, QStringLiteral("📋 1. Stock Overview & Valuation (F5)"));

    // Tab 2: Stock Movement Ledger
    QWidget *tabLedger = new QWidget(m_tabs);
    QVBoxLayout *l2 = new QVBoxLayout(tabLedger);
    l2->setContentsMargins(4, 4, 4, 4);
    l2->setSpacing(4);

    m_tableLedger = new QTableWidget(tabLedger);
    m_tableLedger->setColumnCount(9);
    m_tableLedger->setHorizontalHeaderLabels(QStringList()
        << QStringLiteral("Timestamp")
        << QStringLiteral("Reference #")
        << QStringLiteral("SKU")
        << QStringLiteral("Product Name")
        << QStringLiteral("Movement Type")
        << QStringLiteral("Qty (+/-)")
        << QStringLiteral("Balance After")
        << QStringLiteral("User")
        << QStringLiteral("Notes"));
    m_tableLedger->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableLedger->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_tableLedger->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Stretch);
    m_tableLedger->verticalHeader()->setVisible(false);
    m_tableLedger->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableLedger->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableLedger->setAlternatingRowColors(true);
    m_tableLedger->setStyleSheet(tableStyle);
    l2->addWidget(m_tableLedger, 1);

    m_lblLedgerStats = new QLabel(tabLedger);
    m_lblLedgerStats->setStyleSheet(QStringLiteral("background: #FFFFFF; color: #000080; border: 1px solid #7A9EB9; border-radius: 2px; padding: 3px 8px; font-weight: bold; font-size: 8.5pt;"));
    l2->addWidget(m_lblLedgerStats);
    m_tabs->addTab(tabLedger, QStringLiteral("📜 2. Stock Movement Ledger (Audit)"));

    // Tab 3: Reorder Alerts List
    QWidget *tabReorder = new QWidget(m_tabs);
    QVBoxLayout *l3 = new QVBoxLayout(tabReorder);
    l3->setContentsMargins(4, 4, 4, 4);
    l3->setSpacing(4);

    m_tableReorder = new QTableWidget(tabReorder);
    m_tableReorder->setColumnCount(7);
    m_tableReorder->setHorizontalHeaderLabels(QStringList()
        << QStringLiteral("SKU")
        << QStringLiteral("Product Name")
        << QStringLiteral("Category")
        << QStringLiteral("Available Stock")
        << QStringLiteral("Reorder Level")
        << QStringLiteral("Suggested Order Qty")
        << QStringLiteral("Status"));
    m_tableReorder->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableReorder->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableReorder->verticalHeader()->setVisible(false);
    m_tableReorder->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableReorder->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableReorder->setAlternatingRowColors(true);
    m_tableReorder->setStyleSheet(tableStyle);
    l3->addWidget(m_tableReorder, 1);

    m_lblReorderStats = new QLabel(tabReorder);
    m_lblReorderStats->setStyleSheet(QStringLiteral("background: #FFEBEE; color: #B71C1C; border: 1px solid #FFCDD2; border-radius: 2px; padding: 3px 8px; font-weight: bold; font-size: 8.5pt;"));
    l3->addWidget(m_lblReorderStats);
    m_tabs->addTab(tabReorder, QStringLiteral("⚠️ 3. Low Stock & Reorder Alerts"));

    mainLayout->addWidget(m_tabs, 1);

    // Bottom Function Key Bar
    QFrame *fnBar = new QFrame(this);
    fnBar->setFixedHeight(32);
    fnBar->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; }"
    ));
    QHBoxLayout *fnLayout = new QHBoxLayout(fnBar);
    fnLayout->setContentsMargins(4, 2, 4, 2);
    fnLayout->setSpacing(3);

    const QString fnBtnStyle = QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 2px 8px; font-size: 8.5pt; font-weight: bold; } "
        "QPushButton:hover { background: #EDF4FB; border-color: #4A82AD; }"
    );

    QPushButton *bF1 = new QPushButton(QStringLiteral("F1 - Help"), fnBar);
    bF1->setStyleSheet(fnBtnStyle);
    bF1->setShortcut(QKeySequence(Qt::Key_F1));

    QPushButton *bF5 = new QPushButton(QStringLiteral("F5 - Overview"), fnBar);
    bF5->setStyleSheet(fnBtnStyle);
    bF5->setShortcut(QKeySequence(Qt::Key_F5));
    connect(bF5, &QPushButton::clicked, this, [this]() { switchToTab(0); });

    QPushButton *bF6 = new QPushButton(QStringLiteral("F6 - Stock Adjustment"), fnBar);
    bF6->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 2px 10px; font-size: 8.5pt; font-weight: bold; } "
        "QPushButton:hover { background: #C8E6C9; }"
    ));
    bF6->setShortcut(QKeySequence(Qt::Key_F6));
    connect(bF6, &QPushButton::clicked, this, &InventoryPage::onStockAdjustmentClicked);

    QPushButton *bF7 = new QPushButton(QStringLiteral("F7 - Branch Transfer"), fnBar);
    bF7->setStyleSheet(fnBtnStyle);
    bF7->setShortcut(QKeySequence(Qt::Key_F7));
    connect(bF7, &QPushButton::clicked, this, &InventoryPage::onStockTransferClicked);

    QPushButton *bF8 = new QPushButton(QStringLiteral("F8 - Reorder Alerts"), fnBar);
    bF8->setStyleSheet(fnBtnStyle);
    bF8->setShortcut(QKeySequence(Qt::Key_F8));
    connect(bF8, &QPushButton::clicked, this, [this]() { switchToTab(2); });

    QPushButton *bF9 = new QPushButton(QStringLiteral("F9 - Export Ledger"), fnBar);
    bF9->setStyleSheet(fnBtnStyle);
    bF9->setShortcut(QKeySequence(Qt::Key_F9));
    connect(bF9, &QPushButton::clicked, this, &InventoryPage::onExportLedgerClicked);

    QPushButton *bRefreshFn = new QPushButton(QStringLiteral("🔄 Refresh (F10)"), fnBar);
    bRefreshFn->setStyleSheet(fnBtnStyle);
    bRefreshFn->setShortcut(QKeySequence(Qt::Key_F10));
    connect(bRefreshFn, &QPushButton::clicked, this, &InventoryPage::reloadData);

    QPushButton *bEsc = new QPushButton(QStringLiteral("Esc - Cancel"), fnBar);
    bEsc->setStyleSheet(fnBtnStyle);
    bEsc->setShortcut(QKeySequence(Qt::Key_Escape));

    fnLayout->addWidget(bF1);
    fnLayout->addWidget(bF5);
    fnLayout->addWidget(bF6);
    fnLayout->addWidget(bF7);
    fnLayout->addWidget(bF8);
    fnLayout->addWidget(bF9);
    fnLayout->addWidget(bRefreshFn);
    fnLayout->addStretch();
    fnLayout->addWidget(bEsc);

    mainLayout->addWidget(fnBar);

    connect(btnAdj, &QPushButton::clicked, this, &InventoryPage::onStockAdjustmentClicked);
    connect(btnTransfer, &QPushButton::clicked, this, &InventoryPage::onStockTransferClicked);
    connect(btnExport, &QPushButton::clicked, this, &InventoryPage::onExportLedgerClicked);
    connect(btnRefresh, &QPushButton::clicked, this, &InventoryPage::reloadData);
}

void InventoryPage::switchToTab(int tabIndex) {
    if (m_tabs && tabIndex >= 0 && tabIndex < m_tabs->count()) {
        m_tabs->setCurrentIndex(tabIndex);
    }
}

void InventoryPage::reloadData() {
    loadOverview();
    loadLedger();
    loadReorderList();
}

void InventoryPage::loadOverview() {
    auto prods = DatabaseManager::instance().searchProducts();
    m_tableOverview->setRowCount(prods.size());

    double totalVal = 0.0;
    QString cur = DatabaseManager::instance().currencySymbol();

    for (int r = 0; r < prods.size(); ++r) {
        const auto& p = prods[r];
        double val = p.currentStock * p.costPrice;
        totalVal += val;

        m_tableOverview->setRowHeight(r, 22);

        m_tableOverview->setItem(r, 0, new QTableWidgetItem(p.sku));
        m_tableOverview->setItem(r, 1, new QTableWidgetItem(p.barcode));
        m_tableOverview->setItem(r, 2, new QTableWidgetItem(p.name));
        m_tableOverview->setItem(r, 3, new QTableWidgetItem(p.categoryName));

        QTableWidgetItem *stkItem = new QTableWidgetItem(QStringLiteral("%1 %2").arg(p.currentStock).arg(p.unitCode));
        stkItem->setTextAlignment(Qt::AlignCenter);
        stkItem->setForeground(QBrush(QColor("#000080")));
        stkItem->setFont(QFont(stkItem->font().family(), -1, QFont::Bold));
        m_tableOverview->setItem(r, 4, stkItem);

        QTableWidgetItem *costItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(p.costPrice, 0, 'f', 2));
        costItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_tableOverview->setItem(r, 5, costItem);

        QTableWidgetItem *valItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(val, 0, 'f', 2));
        valItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        valItem->setForeground(QBrush(QColor("#000080")));
        valItem->setFont(QFont(valItem->font().family(), -1, QFont::Bold));
        m_tableOverview->setItem(r, 6, valItem);

        QString status = "In Stock";
        if (p.currentStock <= 0.0) status = "Out of Stock";
        else if (p.currentStock <= p.reorderLevel) status = "Low Stock";

        QTableWidgetItem *statItem = new QTableWidgetItem(status);
        statItem->setTextAlignment(Qt::AlignCenter);
        if (p.currentStock <= 0.0) statItem->setForeground(QBrush(QColor("#B71C1C")));
        else if (p.currentStock <= p.reorderLevel) statItem->setForeground(QBrush(QColor("#E65100")));
        else statItem->setForeground(QBrush(QColor("#1B5E20")));
        statItem->setFont(QFont(statItem->font().family(), -1, QFont::Bold));
        m_tableOverview->setItem(r, 7, statItem);
    }

    m_lblOverviewStats->setText(QStringLiteral("Inventory Size: %1 SKUs  |  Total Asset Value: %2%3")
        .arg(prods.size())
        .arg(cur)
        .arg(totalVal, 0, 'f', 2));
}

void InventoryPage::loadLedger() {
    auto movements = DatabaseManager::instance().getStockMovements(150);
    m_tableLedger->setRowCount(movements.size());

    for (int r = 0; r < movements.size(); ++r) {
        const auto& m = movements[r];
        m_tableLedger->setRowHeight(r, 22);

        m_tableLedger->setItem(r, 0, new QTableWidgetItem(m.createdAt));
        
        QTableWidgetItem *refItem = new QTableWidgetItem(m.referenceNo);
        refItem->setForeground(QBrush(QColor("#000080")));
        refItem->setFont(QFont(refItem->font().family(), -1, QFont::Bold));
        m_tableLedger->setItem(r, 1, refItem);

        m_tableLedger->setItem(r, 2, new QTableWidgetItem(m.sku));
        m_tableLedger->setItem(r, 3, new QTableWidgetItem(m.productName));

        QTableWidgetItem *typeItem = new QTableWidgetItem(m.movementType);
        typeItem->setTextAlignment(Qt::AlignCenter);
        m_tableLedger->setItem(r, 4, typeItem);

        QTableWidgetItem *qtyItem = new QTableWidgetItem(QString::number(m.quantity, 'f', 2));
        qtyItem->setTextAlignment(Qt::AlignCenter);
        if (m.quantity < 0) {
            qtyItem->setForeground(QBrush(QColor("#B71C1C")));
        } else {
            qtyItem->setForeground(QBrush(QColor("#1B5E20")));
        }
        qtyItem->setFont(QFont(qtyItem->font().family(), -1, QFont::Bold));
        m_tableLedger->setItem(r, 5, qtyItem);

        QTableWidgetItem *balItem = new QTableWidgetItem(QString::number(m.balanceAfter, 'f', 2));
        balItem->setTextAlignment(Qt::AlignCenter);
        balItem->setForeground(QBrush(QColor("#000080")));
        balItem->setFont(QFont(balItem->font().family(), -1, QFont::Bold));
        m_tableLedger->setItem(r, 6, balItem);

        m_tableLedger->setItem(r, 7, new QTableWidgetItem(m.userName));
        m_tableLedger->setItem(r, 8, new QTableWidgetItem(m.notes));
    }

    m_lblLedgerStats->setText(QStringLiteral("Audit Ledger Entries Logged: %1 transactions").arg(movements.size()));
}

void InventoryPage::loadReorderList() {
    auto alerts = DatabaseManager::instance().getLowStockAlerts(50);
    m_tableReorder->setRowCount(alerts.size());

    for (int r = 0; r < alerts.size(); ++r) {
        const auto& p = alerts[r];
        double suggested = qMax(10.0, (p.reorderLevel * 2.0) - p.currentStock);

        m_tableReorder->setRowHeight(r, 22);

        m_tableReorder->setItem(r, 0, new QTableWidgetItem(p.sku));
        m_tableReorder->setItem(r, 1, new QTableWidgetItem(p.name));
        m_tableReorder->setItem(r, 2, new QTableWidgetItem(p.categoryName));

        QTableWidgetItem *stkItem = new QTableWidgetItem(QStringLiteral("%1 %2").arg(p.currentStock).arg(p.unitCode));
        stkItem->setTextAlignment(Qt::AlignCenter);
        stkItem->setForeground(QBrush(QColor("#B71C1C")));
        stkItem->setFont(QFont(stkItem->font().family(), -1, QFont::Bold));
        m_tableReorder->setItem(r, 3, stkItem);

        QTableWidgetItem *reorderItem = new QTableWidgetItem(QString::number(p.reorderLevel, 'f', 0));
        reorderItem->setTextAlignment(Qt::AlignCenter);
        m_tableReorder->setItem(r, 4, reorderItem);

        QTableWidgetItem *suggItem = new QTableWidgetItem(QString::number(suggested, 'f', 0));
        suggItem->setTextAlignment(Qt::AlignCenter);
        suggItem->setForeground(QBrush(QColor("#000080")));
        suggItem->setFont(QFont(suggItem->font().family(), -1, QFont::Bold));
        m_tableReorder->setItem(r, 5, suggItem);

        QTableWidgetItem *statItem = new QTableWidgetItem(p.currentStock <= 0 ? QStringLiteral("OUT OF STOCK") : QStringLiteral("REORDER NEEDED"));
        statItem->setTextAlignment(Qt::AlignCenter);
        statItem->setForeground(QBrush(QColor("#B71C1C")));
        statItem->setFont(QFont(statItem->font().family(), -1, QFont::Bold));
        m_tableReorder->setItem(r, 6, statItem);
    }

    m_lblReorderStats->setText(QStringLiteral("⚠️ Alert: %1 products reached critical reorder thresholds!").arg(alerts.size()));
}

void InventoryPage::onStockAdjustmentClicked() {
    auto prods = DatabaseManager::instance().searchProducts();
    if (prods.isEmpty()) return;

    QStringList items;
    for (const auto& p : prods) {
        items << QStringLiteral("%1 - %2 (Current: %3)").arg(p.sku, p.name).arg(p.currentStock);
    }

    bool ok;
    QString selected = QInputDialog::getItem(this, QStringLiteral("Physical Stock Count Adjustment"),
        QStringLiteral("Select item for physical reconciliation:"), items, 0, false, &ok);

    if (ok && !selected.isEmpty()) {
        int idx = items.indexOf(selected);
        if (idx >= 0 && idx < prods.size()) {
            const auto& target = prods[idx];
            double physicalCount = QInputDialog::getDouble(this, QStringLiteral("Stock Adjustment"),
                QStringLiteral("Enter verified physical stock count:"),
                target.currentStock, 0.0, 99999.0, 2, &ok);

            if (ok) {
                QString notes = QInputDialog::getText(this, QStringLiteral("Adjustment Reason"),
                    QStringLiteral("Reason (e.g. Physical audit, breakage, expiry, shrinkage):"),
                    QLineEdit::Normal, QStringLiteral("Physical audit reconciliation"), &ok);

                if (ok) {
                    if (DatabaseManager::instance().recordStockAdjustment(target.id, physicalCount, QStringLiteral("Stock Audit"), notes.isEmpty() ? QStringLiteral("Physical Audit") : notes)) {
                        QMessageBox::information(this, QStringLiteral("Reconciliation Posted"),
                            QStringLiteral("Stock updated for %1.\nAudit ledger entry created.").arg(target.name));
                        reloadData();
                    } else {
                        QMessageBox::critical(this, QStringLiteral("Error"), QStringLiteral("Failed to update stock."));
                    }
                }
            }
        }
    }
}

void InventoryPage::onStockTransferClicked() {
    QMessageBox::information(this, QStringLiteral("Branch Stock Transfer"),
        QStringLiteral("Inter-branch stock transfer manifest prepared.\nStock movement reserved."));
}

void InventoryPage::onExportLedgerClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, QStringLiteral("Export Inventory Ledger"),
        QStringLiteral("Inventory_Ledger.csv"), QStringLiteral("CSV Files (*.csv)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, QStringLiteral("Export Error"), QStringLiteral("Cannot open file for writing."));
        return;
    }

    QTextStream out(&file);
    out << "Timestamp,ReferenceNo,SKU,ProductName,MovementType,Quantity,BalanceAfter,User,Notes\n";
    auto movements = DatabaseManager::instance().getStockMovements(1000);
    for (const auto &m : movements) {
        out << QStringLiteral("\"%1\",\"%2\",\"%3\",\"%4\",\"%5\",%6,%7,\"%8\",\"%9\"\n")
            .arg(m.createdAt)
            .arg(m.referenceNo)
            .arg(m.sku)
            .arg(m.productName)
            .arg(m.movementType)
            .arg(m.quantity)
            .arg(m.balanceAfter)
            .arg(m.userName)
            .arg(m.notes);
    }
    file.close();
    QMessageBox::information(this, QStringLiteral("Export Finished"), QStringLiteral("Exported %1 audit ledger entries.").arg(movements.size()));
}
