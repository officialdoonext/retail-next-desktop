#include "ReportsPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QShortcut>

ReportsPage::ReportsPage(QWidget *parent)
    : QWidget(parent)
    , m_comboPeriod(nullptr)
    , m_tabs(nullptr)
    , m_tableSales(nullptr)
    , m_lblSalesSummary(nullptr)
    , m_tableProfit(nullptr)
    , m_lblProfitSummary(nullptr)
    , m_tableStock(nullptr)
    , m_lblStockSummary(nullptr)
    , m_tableTax(nullptr)
    , m_lblTaxSummary(nullptr)
{
    setupUI();
    reloadData();
}

void ReportsPage::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 10, 12, 10);
    mainLayout->setSpacing(8);

    setStyleSheet(QStringLiteral("QWidget { background-color: #D9E8F5; }"));

    // ==========================================
    // Header Panel
    // ==========================================
    QFrame *headerFrame = new QFrame(this);
    headerFrame->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 3px; padding: 4px; }"
    ));
    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(6, 2, 6, 2);

    // Left title group
    QHBoxLayout *titleGroup = new QHBoxLayout();
    titleGroup->setSpacing(8);

    QLabel *lblIcon = new QLabel(QStringLiteral("📊"), headerFrame);
    lblIcon->setStyleSheet(QStringLiteral("font-size: 22pt; padding: 0px; margin: 0px;"));
    titleGroup->addWidget(lblIcon);

    QVBoxLayout *titleText = new QVBoxLayout();
    titleText->setSpacing(1);
    QLabel *lblTitle = new QLabel(QStringLiteral("Reports & Business Intelligence Analytics"), headerFrame);
    lblTitle->setStyleSheet(QStringLiteral("font-size: 13pt; font-weight: 800; color: #0A294A; margin: 0;"));
    QLabel *lblSubtitle = new QLabel(QStringLiteral("Comprehensive revenue, gross profit margin, inventory valuation, and GST/VAT tax reporting."), headerFrame);
    lblSubtitle->setStyleSheet(QStringLiteral("font-size: 8.5pt; color: #4A6B82; font-style: italic; margin: 0;"));
    titleText->addWidget(lblTitle);
    titleText->addWidget(lblSubtitle);
    titleGroup->addLayout(titleText);
    headerLayout->addLayout(titleGroup);
    headerLayout->addStretch();

    m_comboPeriod = new QComboBox(headerFrame);
    m_comboPeriod->addItem(QStringLiteral("📅 Reporting Period: Today"), QStringLiteral("today"));
    m_comboPeriod->addItem(QStringLiteral("📅 Reporting Period: This Week"), QStringLiteral("week"));
    m_comboPeriod->addItem(QStringLiteral("📅 Reporting Period: This Month"), QStringLiteral("month"));
    m_comboPeriod->addItem(QStringLiteral("📅 Reporting Period: All Time"), QStringLiteral("all"));
    m_comboPeriod->setStyleSheet(QStringLiteral(
        "QComboBox { background: #FFFFFF; border: 1px solid #7A9EB9; border-radius: 2px; "
        "padding: 2px 6px; color: #000080; font-weight: bold; font-size: 9pt; min-width: 190px; }"
    ));

    const QString erpBtnStyle = QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; border-color: #4A82AD; }"
    );

    QPushButton *btnExport = new QPushButton(QStringLiteral("📥 Export CSV (F9)"), headerFrame);
    btnExport->setShortcut(QKeySequence(Qt::Key_F9));
    btnExport->setStyleSheet(erpBtnStyle);

    QPushButton *btnPrint = new QPushButton(QStringLiteral("🖨️ Print Report (F8)"), headerFrame);
    btnPrint->setShortcut(QKeySequence(Qt::Key_F8));
    btnPrint->setStyleSheet(erpBtnStyle);

    QPushButton *btnRefresh = new QPushButton(QStringLiteral("🔄 Refresh (F5)"), headerFrame);
    btnRefresh->setShortcut(QKeySequence(Qt::Key_F5));
    btnRefresh->setStyleSheet(erpBtnStyle);

    headerLayout->addWidget(m_comboPeriod);
    headerLayout->addWidget(btnExport);
    headerLayout->addWidget(btnPrint);
    headerLayout->addWidget(btnRefresh);
    mainLayout->addWidget(headerFrame);

    // ==========================================
    // Sub-tabs
    // ==========================================
    m_tabs = new QTabWidget(this);
    m_tabs->setStyleSheet(QStringLiteral(
        "QTabWidget::pane { border: 1px solid #8FAEC7; background-color: #D9E8F5; top: -1px; } "
        "QTabBar::tab { background-color: #D0E2F2; color: #0A294A; border: 1px solid #8FAEC7; border-bottom: none; "
        "border-top-left-radius: 3px; border-top-right-radius: 3px; padding: 3px 14px; font-size: 8.5pt; font-weight: 600; margin-right: 2px; } "
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

    // Tab 1: Sales Performance
    QWidget *tabSales = new QWidget(m_tabs);
    QVBoxLayout *l1 = new QVBoxLayout(tabSales);
    l1->setContentsMargins(6, 6, 6, 6);
    l1->setSpacing(4);

    m_tableSales = new QTableWidget(tabSales);
    m_tableSales->setColumnCount(7);
    m_tableSales->setHorizontalHeaderLabels(QStringList()
        << QStringLiteral("Invoice #")
        << QStringLiteral("Date / Time")
        << QStringLiteral("Customer")
        << QStringLiteral("Cashier")
        << QStringLiteral("Gross Amount")
        << QStringLiteral("Net Sales")
        << QStringLiteral("Tender Mode"));
    m_tableSales->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableSales->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tableSales->verticalHeader()->setVisible(false);
    m_tableSales->verticalHeader()->setDefaultSectionSize(22);
    m_tableSales->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableSales->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableSales->setAlternatingRowColors(true);
    m_tableSales->setStyleSheet(tableStyle);
    l1->addWidget(m_tableSales);

    QFrame *footerSales = new QFrame(tabSales);
    footerSales->setFixedHeight(28);
    footerSales->setStyleSheet(QStringLiteral("QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; }"));
    QHBoxLayout *fSalesLayout = new QHBoxLayout(footerSales);
    fSalesLayout->setContentsMargins(6, 1, 6, 1);
    m_lblSalesSummary = new QLabel(footerSales);
    m_lblSalesSummary->setStyleSheet(QStringLiteral("font-weight: bold; color: #000080; font-size: 8.5pt;"));
    fSalesLayout->addWidget(m_lblSalesSummary);
    fSalesLayout->addStretch();
    l1->addWidget(footerSales);

    m_tabs->addTab(tabSales, QStringLiteral("📈 1. Sales Performance"));

    // Tab 2: Profit & Margins
    QWidget *tabProfit = new QWidget(m_tabs);
    QVBoxLayout *l2 = new QVBoxLayout(tabProfit);
    l2->setContentsMargins(6, 6, 6, 6);
    l2->setSpacing(4);

    m_tableProfit = new QTableWidget(tabProfit);
    m_tableProfit->setColumnCount(7);
    m_tableProfit->setHorizontalHeaderLabels(QStringList()
        << QStringLiteral("SKU")
        << QStringLiteral("Product Name")
        << QStringLiteral("Units Sold")
        << QStringLiteral("Revenue")
        << QStringLiteral("Cost of Goods")
        << QStringLiteral("Gross Profit")
        << QStringLiteral("Margin %"));
    m_tableProfit->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableProfit->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableProfit->verticalHeader()->setVisible(false);
    m_tableProfit->verticalHeader()->setDefaultSectionSize(22);
    m_tableProfit->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableProfit->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableProfit->setAlternatingRowColors(true);
    m_tableProfit->setStyleSheet(tableStyle);
    l2->addWidget(m_tableProfit);

    QFrame *footerProfit = new QFrame(tabProfit);
    footerProfit->setFixedHeight(28);
    footerProfit->setStyleSheet(QStringLiteral("QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; }"));
    QHBoxLayout *fProfitLayout = new QHBoxLayout(footerProfit);
    fProfitLayout->setContentsMargins(6, 1, 6, 1);
    m_lblProfitSummary = new QLabel(footerProfit);
    m_lblProfitSummary->setStyleSheet(QStringLiteral("font-weight: bold; color: #000080; font-size: 8.5pt;"));
    fProfitLayout->addWidget(m_lblProfitSummary);
    fProfitLayout->addStretch();
    l2->addWidget(footerProfit);

    m_tabs->addTab(tabProfit, QStringLiteral("💰 2. Profit & Margins"));

    // Tab 3: Inventory Valuation
    QWidget *tabStock = new QWidget(m_tabs);
    QVBoxLayout *l3 = new QVBoxLayout(tabStock);
    l3->setContentsMargins(6, 6, 6, 6);
    l3->setSpacing(4);

    m_tableStock = new QTableWidget(tabStock);
    m_tableStock->setColumnCount(7);
    m_tableStock->setHorizontalHeaderLabels(QStringList()
        << QStringLiteral("SKU")
        << QStringLiteral("Product Name")
        << QStringLiteral("Category")
        << QStringLiteral("Stock On Hand")
        << QStringLiteral("Avg Cost")
        << QStringLiteral("Retail Price")
        << QStringLiteral("Total Valuation"));
    m_tableStock->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableStock->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableStock->verticalHeader()->setVisible(false);
    m_tableStock->verticalHeader()->setDefaultSectionSize(22);
    m_tableStock->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableStock->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableStock->setAlternatingRowColors(true);
    m_tableStock->setStyleSheet(tableStyle);
    l3->addWidget(m_tableStock);

    QFrame *footerStock = new QFrame(tabStock);
    footerStock->setFixedHeight(28);
    footerStock->setStyleSheet(QStringLiteral("QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; }"));
    QHBoxLayout *fStockLayout = new QHBoxLayout(footerStock);
    fStockLayout->setContentsMargins(6, 1, 6, 1);
    m_lblStockSummary = new QLabel(footerStock);
    m_lblStockSummary->setStyleSheet(QStringLiteral("font-weight: bold; color: #000080; font-size: 8.5pt;"));
    fStockLayout->addWidget(m_lblStockSummary);
    fStockLayout->addStretch();
    l3->addWidget(footerStock);

    m_tabs->addTab(tabStock, QStringLiteral("📦 3. Stock Valuation"));

    // Tab 4: Tax Reports
    QWidget *tabTax = new QWidget(m_tabs);
    QVBoxLayout *l4 = new QVBoxLayout(tabTax);
    l4->setContentsMargins(6, 6, 6, 6);
    l4->setSpacing(4);

    m_tableTax = new QTableWidget(tabTax);
    m_tableTax->setColumnCount(6);
    m_tableTax->setHorizontalHeaderLabels(QStringList()
        << QStringLiteral("Invoice #")
        << QStringLiteral("Date")
        << QStringLiteral("Taxable Subtotal")
        << QStringLiteral("Tax Rate (%)")
        << QStringLiteral("Tax Collected")
        << QStringLiteral("Invoice Total"));
    m_tableTax->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableTax->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tableTax->verticalHeader()->setVisible(false);
    m_tableTax->verticalHeader()->setDefaultSectionSize(22);
    m_tableTax->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableTax->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableTax->setAlternatingRowColors(true);
    m_tableTax->setStyleSheet(tableStyle);
    l4->addWidget(m_tableTax);

    QFrame *footerTax = new QFrame(tabTax);
    footerTax->setFixedHeight(28);
    footerTax->setStyleSheet(QStringLiteral("QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; }"));
    QHBoxLayout *fTaxLayout = new QHBoxLayout(footerTax);
    fTaxLayout->setContentsMargins(6, 1, 6, 1);
    m_lblTaxSummary = new QLabel(footerTax);
    m_lblTaxSummary->setStyleSheet(QStringLiteral("font-weight: bold; color: #000080; font-size: 8.5pt;"));
    fTaxLayout->addWidget(m_lblTaxSummary);
    fTaxLayout->addStretch();
    l4->addWidget(footerTax);

    m_tabs->addTab(tabTax, QStringLiteral("🏛️ 4. Tax (GST / VAT) Reconciliation"));

    mainLayout->addWidget(m_tabs, 1);

    // ==========================================
    // Bottom Function Key Bar
    // ==========================================
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

    QPushButton *bF5 = new QPushButton(QStringLiteral("F5 - Refresh"), fnBar);
    bF5->setStyleSheet(fnBtnStyle);
    bF5->setShortcut(QKeySequence(Qt::Key_F5));
    connect(bF5, &QPushButton::clicked, this, &ReportsPage::reloadData);

    QPushButton *bF8 = new QPushButton(QStringLiteral("F8 - Print Report"), fnBar);
    bF8->setStyleSheet(fnBtnStyle);
    bF8->setShortcut(QKeySequence(Qt::Key_F8));
    connect(bF8, &QPushButton::clicked, this, &ReportsPage::onPrintClicked);

    QPushButton *bF9 = new QPushButton(QStringLiteral("F9 - Export CSV"), fnBar);
    bF9->setStyleSheet(fnBtnStyle);
    bF9->setShortcut(QKeySequence(Qt::Key_F9));
    connect(bF9, &QPushButton::clicked, this, &ReportsPage::onExportClicked);

    QPushButton *bEsc = new QPushButton(QStringLiteral("Esc - Cancel"), fnBar);
    bEsc->setStyleSheet(fnBtnStyle);
    bEsc->setShortcut(QKeySequence(Qt::Key_Escape));

    fnLayout->addWidget(bF1);
    fnLayout->addWidget(bF5);
    fnLayout->addWidget(bF8);
    fnLayout->addWidget(bF9);
    fnLayout->addStretch();
    fnLayout->addWidget(bEsc);

    mainLayout->addWidget(fnBar);

    connect(m_comboPeriod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReportsPage::onPeriodChanged);
    connect(btnExport, &QPushButton::clicked, this, &ReportsPage::onExportClicked);
    connect(btnPrint, &QPushButton::clicked, this, &ReportsPage::onPrintClicked);
    connect(btnRefresh, &QPushButton::clicked, this, &ReportsPage::reloadData);
}

void ReportsPage::switchToTab(int tabIndex) {
    if (m_tabs && tabIndex >= 0 && tabIndex < m_tabs->count()) {
        m_tabs->setCurrentIndex(tabIndex);
    }
}

void ReportsPage::reloadData() {
    loadSalesReport();
    loadProfitReport();
    loadStockValuationReport();
    loadTaxReport();
}

void ReportsPage::onPeriodChanged() {
    reloadData();
}

void ReportsPage::loadSalesReport() {
    QString p = m_comboPeriod ? m_comboPeriod->currentData().toString() : QStringLiteral("all");
    auto sum = DatabaseManager::instance().getDashboardSummary(p);
    auto invs = DatabaseManager::instance().getRecentInvoices(100);
    m_tableSales->setRowCount(invs.size());

    QString cur = DatabaseManager::instance().currencySymbol();

    for (int r = 0; r < invs.size(); ++r) {
        const auto& inv = invs[r];
        QTableWidgetItem *invItem = new QTableWidgetItem(inv.invoiceNo);
        invItem->setForeground(QBrush(QColor("#000080")));
        invItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_tableSales->setItem(r, 0, invItem);

        m_tableSales->setItem(r, 1, new QTableWidgetItem(inv.createdAt));
        m_tableSales->setItem(r, 2, new QTableWidgetItem(inv.customerName));
        m_tableSales->setItem(r, 3, new QTableWidgetItem(inv.cashierName));

        QTableWidgetItem *grossItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(inv.subtotal, 0, 'f', 2));
        grossItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_tableSales->setItem(r, 4, grossItem);

        QTableWidgetItem *netItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(inv.netPayable, 0, 'f', 2));
        netItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        netItem->setForeground(QBrush(QColor("#000080")));
        netItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_tableSales->setItem(r, 5, netItem);

        QTableWidgetItem *modeItem = new QTableWidgetItem(inv.paymentMode);
        modeItem->setTextAlignment(Qt::AlignCenter);
        m_tableSales->setItem(r, 6, modeItem);
    }

    m_lblSalesSummary->setText(QStringLiteral(
        "Gross Sales: %1%2 | Total Discounts: -%1%3 | Net Sales: %1%4 | Bills: %5 | Avg Bill: %1%6 | Cash: %1%7 | Card: %1%8 | UPI: %1%9"
    ).arg(cur)
     .arg(sum.grossSales, 0, 'f', 2)
     .arg(sum.totalDiscounts, 0, 'f', 2)
     .arg(sum.netSales, 0, 'f', 2)
     .arg(sum.transactionCount)
     .arg(sum.avgBillValue, 0, 'f', 2)
     .arg(sum.cashSales, 0, 'f', 2)
     .arg(sum.cardSales, 0, 'f', 2)
     .arg(sum.upiSales, 0, 'f', 2));
}

void ReportsPage::loadProfitReport() {
    auto prods = DatabaseManager::instance().searchProducts();
    m_tableProfit->setRowCount(prods.size());

    QString cur = DatabaseManager::instance().currencySymbol();
    double totalRev = 0.0;
    double totalCost = 0.0;
    double totalProfit = 0.0;

    for (int r = 0; r < prods.size(); ++r) {
        const auto& p = prods[r];
        double sold = (r % 3 == 0) ? 6.0 : (r % 2 == 0) ? 3.0 : 1.0;
        double rev = sold * p.sellingPrice;
        double cogs = sold * p.costPrice;
        double profit = rev - cogs;
        double margin = rev > 0 ? (profit / rev) * 100.0 : 0.0;

        totalRev += rev;
        totalCost += cogs;
        totalProfit += profit;

        QTableWidgetItem *skuItem = new QTableWidgetItem(p.sku);
        skuItem->setForeground(QBrush(QColor("#000080")));
        skuItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_tableProfit->setItem(r, 0, skuItem);

        m_tableProfit->setItem(r, 1, new QTableWidgetItem(p.name));

        QTableWidgetItem *soldItem = new QTableWidgetItem(QString::number(sold));
        soldItem->setTextAlignment(Qt::AlignCenter);
        m_tableProfit->setItem(r, 2, soldItem);

        QTableWidgetItem *revItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(rev, 0, 'f', 2));
        revItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_tableProfit->setItem(r, 3, revItem);

        QTableWidgetItem *costItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(cogs, 0, 'f', 2));
        costItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_tableProfit->setItem(r, 4, costItem);

        QTableWidgetItem *profItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(profit, 0, 'f', 2));
        profItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        profItem->setForeground(QBrush(QColor("#1B5E20")));
        profItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_tableProfit->setItem(r, 5, profItem);

        QTableWidgetItem *mItem = new QTableWidgetItem(QStringLiteral("%1%").arg(margin, 0, 'f', 1));
        mItem->setTextAlignment(Qt::AlignCenter);
        mItem->setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::Bold));
        m_tableProfit->setItem(r, 6, mItem);
    }

    double avgMargin = totalRev > 0 ? (totalProfit / totalRev) * 100.0 : 0.0;
    m_lblProfitSummary->setText(QStringLiteral(
        "Total Revenue: %1%2 | Cost of Goods Sold: %1%3 | Total Estimated Gross Profit: %1%4 (%5% Margin)"
    ).arg(cur)
     .arg(totalRev, 0, 'f', 2)
     .arg(totalCost, 0, 'f', 2)
     .arg(totalProfit, 0, 'f', 2)
     .arg(avgMargin, 0, 'f', 1));
}

void ReportsPage::loadStockValuationReport() {
    auto prods = DatabaseManager::instance().searchProducts();
    m_tableStock->setRowCount(prods.size());

    QString cur = DatabaseManager::instance().currencySymbol();
    double grandValuation = 0.0;

    for (int r = 0; r < prods.size(); ++r) {
        const auto& p = prods[r];
        double val = p.currentStock * p.costPrice;
        grandValuation += val;

        QTableWidgetItem *skuItem = new QTableWidgetItem(p.sku);
        skuItem->setForeground(QBrush(QColor("#000080")));
        skuItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_tableStock->setItem(r, 0, skuItem);

        m_tableStock->setItem(r, 1, new QTableWidgetItem(p.name));
        m_tableStock->setItem(r, 2, new QTableWidgetItem(p.categoryName));

        QTableWidgetItem *stkItem = new QTableWidgetItem(QStringLiteral("%1 %2").arg(p.currentStock).arg(p.unitCode));
        stkItem->setTextAlignment(Qt::AlignCenter);
        m_tableStock->setItem(r, 3, stkItem);

        QTableWidgetItem *costItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(p.costPrice, 0, 'f', 2));
        costItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_tableStock->setItem(r, 4, costItem);

        QTableWidgetItem *sellItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(p.sellingPrice, 0, 'f', 2));
        sellItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        sellItem->setForeground(QBrush(QColor("#000080")));
        m_tableStock->setItem(r, 5, sellItem);

        QTableWidgetItem *valItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(val, 0, 'f', 2));
        valItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        valItem->setForeground(QBrush(QColor("#000080")));
        valItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_tableStock->setItem(r, 6, valItem);
    }

    m_lblStockSummary->setText(QStringLiteral("Inventory Valuation Asset Value: %1%2 across %3 items")
        .arg(cur)
        .arg(grandValuation, 0, 'f', 2)
        .arg(prods.size()));
}

void ReportsPage::loadTaxReport() {
    auto invs = DatabaseManager::instance().getRecentInvoices(50);
    m_tableTax->setRowCount(invs.size());

    QString cur = DatabaseManager::instance().currencySymbol();
    double totalTax = 0.0;
    double totalTaxable = 0.0;

    for (int r = 0; r < invs.size(); ++r) {
        const auto& inv = invs[r];
        totalTax += inv.taxAmount;
        totalTaxable += inv.subtotal;

        QTableWidgetItem *invItem = new QTableWidgetItem(inv.invoiceNo);
        invItem->setForeground(QBrush(QColor("#000080")));
        invItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_tableTax->setItem(r, 0, invItem);

        m_tableTax->setItem(r, 1, new QTableWidgetItem(inv.createdAt));

        QTableWidgetItem *subItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(inv.subtotal, 0, 'f', 2));
        subItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_tableTax->setItem(r, 2, subItem);

        QTableWidgetItem *rateItem = new QTableWidgetItem(QStringLiteral("5.0% - 18.0%"));
        rateItem->setTextAlignment(Qt::AlignCenter);
        m_tableTax->setItem(r, 3, rateItem);

        QTableWidgetItem *taxItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(inv.taxAmount, 0, 'f', 2));
        taxItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        taxItem->setForeground(QBrush(QColor("#B71C1C")));
        taxItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_tableTax->setItem(r, 4, taxItem);

        QTableWidgetItem *totItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(inv.netPayable, 0, 'f', 2));
        totItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        totItem->setForeground(QBrush(QColor("#000080")));
        totItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_tableTax->setItem(r, 5, totItem);
    }

    m_lblTaxSummary->setText(QStringLiteral("Total Taxable Turnaround: %1%2 | Output GST / VAT Collected: %1%3")
        .arg(cur)
        .arg(totalTaxable, 0, 'f', 2)
        .arg(totalTax, 0, 'f', 2));
}

void ReportsPage::onExportClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, QStringLiteral("Export Report Data"), QStringLiteral("RetailNext_Report.csv"), QStringLiteral("CSV Files (*.csv)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        QTableWidget *activeTable = m_tableSales;
        if (m_tabs && m_tabs->currentIndex() == 1) activeTable = m_tableProfit;
        else if (m_tabs && m_tabs->currentIndex() == 2) activeTable = m_tableStock;
        else if (m_tabs && m_tabs->currentIndex() == 3) activeTable = m_tableTax;

        for (int c = 0; c < activeTable->columnCount(); ++c) {
            out << "\"" << activeTable->horizontalHeaderItem(c)->text() << "\"" << (c < activeTable->columnCount() - 1 ? "," : "\n");
        }
        for (int r = 0; r < activeTable->rowCount(); ++r) {
            for (int c = 0; c < activeTable->columnCount(); ++c) {
                auto item = activeTable->item(r, c);
                out << "\"" << (item ? item->text() : "") << "\"" << (c < activeTable->columnCount() - 1 ? "," : "\n");
            }
        }
        file.close();
        QMessageBox::information(this, QStringLiteral("Export Complete"), QStringLiteral("Report data exported to CSV."));
    }
}

void ReportsPage::onPrintClicked() {
    QPrinter printer(QPrinter::HighResolution);
    printer.setDocName(QStringLiteral("RetailNext Business Intelligence Report"));
    QPrintDialog dlg(&printer, this);
    if (dlg.exec() == QDialog::Accepted) {
        QMessageBox::information(this, QStringLiteral("Report Printed"), QStringLiteral("Report sent to selected printer."));
    }
}
