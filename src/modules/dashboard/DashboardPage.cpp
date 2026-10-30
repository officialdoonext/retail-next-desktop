#include "DashboardPage.h"
#include "../../database/DatabaseManager.h"
#include "ui_DashboardPage.h"
#include <QDateTime>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QShortcut>

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::DashboardPage) {
  ui->setupUi(this);

  // High-Density ERP Palette & Frame Styling
  setStyleSheet(QStringLiteral("QWidget { background-color: #D9E8F5; }"));

  ui->headerFrame->setStyleSheet(QStringLiteral(
      "QFrame#headerFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 3px; padding: 4px; }"
  ));
  ui->labelTitle->setStyleSheet(QStringLiteral("color: #0A294A; font-weight: 800; font-size: 13pt;"));
  ui->labelSubtitle->setStyleSheet(QStringLiteral("color: #4A6B82; font-size: 8.5pt;"));
  ui->labelStoreBadge->setStyleSheet(QStringLiteral(
      "background-color: #FFFFFF; color: #000080; border: 1px solid #7A9EB9; border-radius: 2px; padding: 4px 10px; font-weight: bold; font-size: 8.5pt;"
  ));

  const QString cardStyle = QStringLiteral(
      "QFrame { background-color: #FFFFFF; border: 1px solid #8FAEC7; border-radius: 3px; padding: 6px; } "
      "QLabel { background: transparent; } "
      "QLabel[objectName^=\"lblCard\"][objectName$=\"Title\"] { color: #0A294A; font-size: 8pt; font-weight: bold; } "
      "QLabel[objectName^=\"lblCard\"][objectName$=\"Value\"] { color: #000080; font-size: 15pt; font-weight: 800; } "
      "QLabel[objectName^=\"lblCard\"][objectName$=\"Desc\"] { color: #64748B; font-size: 8pt; }"
  );

  ui->cardRevenue->setStyleSheet(cardStyle);
  ui->cardTransactions->setStyleSheet(cardStyle);
  ui->cardStock->setStyleSheet(cardStyle);
  ui->cardCustomers->setStyleSheet(cardStyle);

  const QString btnStyle = QStringLiteral(
      "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
      "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 4px 10px; font-weight: bold; font-size: 8.5pt; } "
      "QPushButton:hover { background: #EDF4FB; border-color: #4A82AD; } "
      "QPushButton:pressed { background: #C8DBEC; }"
  );

  ui->btnQuickPOS->setText(QStringLiteral("🛒 Open POS Cash Register (F2)"));
  ui->btnQuickPOS->setShortcut(QKeySequence(Qt::Key_F2));
  ui->btnQuickPOS->setStyleSheet(QStringLiteral(
      "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
      "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 4px 12px; font-weight: 800; font-size: 8.5pt; } "
      "QPushButton:hover { background: #C8E6C9; border-color: #4CAF50; }"
  ));

  ui->btnAddProduct->setText(QStringLiteral("📦 New Item Master (F3)"));
  ui->btnAddProduct->setShortcut(QKeySequence(Qt::Key_F3));
  ui->btnAddProduct->setStyleSheet(btnStyle);

  ui->btnStockInward->setText(QStringLiteral("🚚 Goods Receipt Note (F6)"));
  ui->btnStockInward->setShortcut(QKeySequence(Qt::Key_F6));
  ui->btnStockInward->setStyleSheet(btnStyle);

  ui->btnDailyReport->setText(QStringLiteral("📊 Daily Analytics Report (F8)"));
  ui->btnDailyReport->setShortcut(QKeySequence(Qt::Key_F8));
  ui->btnDailyReport->setStyleSheet(btnStyle);

  setupRecentTable();
  refreshData();
}

DashboardPage::~DashboardPage() { delete ui; }

void DashboardPage::setupRecentTable() {
  ui->tableRecentSales->setColumnCount(7);
  QStringList headers;
  headers << QStringLiteral("Invoice #") << QStringLiteral("Time")
          << QStringLiteral("Customer") << QStringLiteral("Items")
          << QStringLiteral("Payment Method") << QStringLiteral("Total Amount")
          << QStringLiteral("Status");
  ui->tableRecentSales->setHorizontalHeaderLabels(headers);
  ui->tableRecentSales->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  ui->tableRecentSales->horizontalHeader()->setSectionResizeMode(
      0, QHeaderView::ResizeToContents);
  ui->tableRecentSales->horizontalHeader()->setSectionResizeMode(
      1, QHeaderView::ResizeToContents);
  ui->tableRecentSales->horizontalHeader()->setSectionResizeMode(
      3, QHeaderView::ResizeToContents);
  ui->tableRecentSales->horizontalHeader()->setSectionResizeMode(
      6, QHeaderView::ResizeToContents);
  ui->tableRecentSales->verticalHeader()->setVisible(false);
  ui->tableRecentSales->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableRecentSales->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->tableRecentSales->setAlternatingRowColors(true);
  ui->tableRecentSales->setStyleSheet(QStringLiteral(
      "QTableWidget { background-color: #FFFFFF; alternate-background-color: #F8FBFE; "
      "gridline-color: #C2D7EA; border: 1px solid #7A9EB9; font-size: 8.5pt; color: #0A294A; } "
      "QTableWidget::item { padding: 2px 4px; } "
      "QTableWidget::item:selected { background-color: #2D82D6; color: #FFFFFF; font-weight: bold; } "
      "QHeaderView::section { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B3D4F0, stop:1 #8DBDE0); "
      "color: #0A294A; font-weight: bold; font-size: 8pt; border: none; border-top: 1px solid #C2D7EA; "
      "border-bottom: 1px solid #6895BA; border-right: 1px solid #99BEDC; padding: 3px 5px; }"
  ));
}

void DashboardPage::refreshData() {
  updateKPICards();

  DatabaseManager &db = DatabaseManager::instance();
  auto invs = db.getRecentInvoices(8);
  ui->tableRecentSales->setRowCount(invs.size());

  QString cur = db.currencySymbol();

  for (int r = 0; r < invs.size(); ++r) {
    const auto &inv = invs[r];
    ui->tableRecentSales->setRowHeight(r, 22);

    QTableWidgetItem *invItem = new QTableWidgetItem(inv.invoiceNo);
    invItem->setForeground(QBrush(QColor("#000080")));
    invItem->setFont(QFont(invItem->font().family(), -1, QFont::Bold));
    ui->tableRecentSales->setItem(r, 0, invItem);

    ui->tableRecentSales->setItem(r, 1, new QTableWidgetItem(inv.createdAt));
    ui->tableRecentSales->setItem(r, 2, new QTableWidgetItem(inv.customerName));
    
    QTableWidgetItem *itemCount = new QTableWidgetItem(QString::number(inv.itemCount > 0 ? inv.itemCount : 1));
    itemCount->setTextAlignment(Qt::AlignCenter);
    ui->tableRecentSales->setItem(r, 3, itemCount);

    ui->tableRecentSales->setItem(r, 4, new QTableWidgetItem(inv.paymentMode));

    QTableWidgetItem *totItem = new QTableWidgetItem(
        QStringLiteral("%1%2").arg(cur).arg(inv.netPayable, 0, 'f', 2));
    totItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    totItem->setForeground(QBrush(QColor("#000080")));
    totItem->setFont(QFont(totItem->font().family(), -1, QFont::Bold));
    ui->tableRecentSales->setItem(r, 5, totItem);

    QTableWidgetItem *statusItem = new QTableWidgetItem(inv.paymentStatus);
    statusItem->setTextAlignment(Qt::AlignCenter);
    if (inv.paymentStatus == "Returned") {
      statusItem->setForeground(QBrush(QColor("#B71C1C")));
    } else {
      statusItem->setForeground(QBrush(QColor("#1B5E20")));
    }
    statusItem->setFont(QFont(statusItem->font().family(), -1, QFont::Bold));
    ui->tableRecentSales->setItem(r, 6, statusItem);
  }
}

void DashboardPage::updateKPICards() {
  DatabaseManager &db = DatabaseManager::instance();
  auto sum = db.getDashboardSummary("today");
  QString cur = db.currencySymbol();

  ui->labelStoreBadge->setText(
      QStringLiteral("🏪 %1 | %2 | Profile: %3")
          .arg(db.storeName())
          .arg(db.branchName())
          .arg(RetailNext::businessTypeToString(db.businessType())));

  ui->lblCard1Title->setText(QStringLiteral("TODAY'S NET SALES"));
  ui->lblCard1Value->setText(
      QStringLiteral("%1%2").arg(cur).arg(sum.netSales, 0, 'f', 2));
  ui->lblCard1Desc->setText(QStringLiteral("Gross: %1%2 | Disc: -%1%3")
                                .arg(cur)
                                .arg(sum.grossSales, 0, 'f', 2)
                                .arg(sum.totalDiscounts, 0, 'f', 2));

  ui->lblCard2Title->setText(QStringLiteral("TRANSACTIONS"));
  ui->lblCard2Value->setText(
      QStringLiteral("%1 Bills").arg(sum.transactionCount));
  ui->lblCard2Desc->setText(QStringLiteral("Avg Basket: %1%2 | Items: %3")
                                .arg(cur)
                                .arg(sum.avgBillValue, 0, 'f', 2)
                                .arg(sum.itemsSold, 0, 'f', 0));

  ui->lblCard3Title->setText(QStringLiteral("INVENTORY HEALTH"));
  ui->lblCard3Value->setText(QStringLiteral("%1 Low / %2 Out")
                                 .arg(sum.lowStockCount)
                                 .arg(sum.outOfStockCount));
  ui->lblCard3Desc->setText(QStringLiteral("Total Asset Value: %1%2")
                                .arg(cur)
                                .arg(sum.totalStockValue, 0, 'f', 2));

  ui->lblCard4Title->setText(QStringLiteral("RECEIVABLES / PAYABLES"));
  ui->lblCard4Value->setText(
      QStringLiteral("%1%2 Rec").arg(cur).arg(sum.receivables, 0, 'f', 2));
  ui->lblCard4Desc->setText(QStringLiteral("Supplier Payables: %1%2")
                                .arg(cur)
                                .arg(sum.payables, 0, 'f', 2));
}

void DashboardPage::on_btnQuickPOS_clicked() { emit openPOSRequested(); }
void DashboardPage::on_btnAddProduct_clicked() { emit openProductsRequested(); }
void DashboardPage::on_btnStockInward_clicked() { emit openStockRequested(); }
void DashboardPage::on_btnDailyReport_clicked() { emit openReportsRequested(); }
