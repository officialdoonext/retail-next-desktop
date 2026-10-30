#include "PurchasesPage.h"
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextStream>
#include <QVBoxLayout>
#include <QShortcut>

PurchasesPage::PurchasesPage(QWidget *parent)
    : QWidget(parent), m_tabs(nullptr), m_tableInvoices(nullptr),
      m_tableSuppliers(nullptr), m_lblInvoicesSummary(nullptr),
      m_lblSuppliersSummary(nullptr) {
  setupUI();
  reloadData();
}

void PurchasesPage::setupUI() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(12, 10, 12, 10);
  mainLayout->setSpacing(8);

  setStyleSheet(QStringLiteral("QWidget { background-color: #D9E8F5; }"));

  // ==========================================
  // Header Panel
  // ==========================================
  QFrame *headerFrame = new QFrame(this);
  headerFrame->setStyleSheet(QStringLiteral(
      "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; "
      "border-radius: 3px; padding: 4px; }"));
  QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
  headerLayout->setContentsMargins(6, 2, 6, 2);

  // Left title group
  QHBoxLayout *titleGroup = new QHBoxLayout();
  titleGroup->setSpacing(8);

  QLabel *lblIcon = new QLabel(QStringLiteral("📦"), headerFrame);
  lblIcon->setStyleSheet(
      QStringLiteral("font-size: 22pt; padding: 0px; margin: 0px;"));
  titleGroup->addWidget(lblIcon);

  QVBoxLayout *titleText = new QVBoxLayout();
  titleText->setSpacing(1);
  QLabel *lblTitle = new QLabel(
      QStringLiteral("Purchases & Vendor Management"), headerFrame);
  lblTitle->setStyleSheet(QStringLiteral(
      "font-size: 13pt; font-weight: 800; color: #0A294A; margin: 0;"));
  QLabel *lblSubtitle =
      new QLabel(QStringLiteral("Procurement, supplier invoices, goods "
                                "receiving (GRN), and vendor payables ledger."),
                 headerFrame);
  lblSubtitle->setStyleSheet(QStringLiteral(
      "font-size: 8.5pt; color: #4A6B82; font-style: italic; margin: 0;"));
  titleText->addWidget(lblTitle);
  titleText->addWidget(lblSubtitle);
  titleGroup->addLayout(titleText);
  headerLayout->addLayout(titleGroup);
  headerLayout->addStretch();

  const QString erpBtnStyle = QStringLiteral(
      "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
      "stop:0 #FFFFFF, stop:1 #E0EBF5); "
      "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: "
      "3px 10px; font-weight: bold; font-size: 8.5pt; } "
      "QPushButton:hover { background: #EDF4FB; border-color: #4A82AD; }");

  QPushButton *btnInward = new QPushButton(
      QStringLiteral("📦 Inward Goods Receipt (F6)"), headerFrame);
  btnInward->setShortcut(QKeySequence(Qt::Key_F6));
  btnInward->setStyleSheet(QStringLiteral(
      "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
      "stop:0 #E8F5E9, stop:1 #C8E6C9); "
      "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; "
      "padding: 3px 12px; font-weight: bold; font-size: 8.5pt; } "
      "QPushButton:hover { background: #C8E6C9; }"));

  QPushButton *btnNewSupp =
      new QPushButton(QStringLiteral("➕ New Supplier (F7)"), headerFrame);
  btnNewSupp->setShortcut(QKeySequence(Qt::Key_F7));
  btnNewSupp->setStyleSheet(erpBtnStyle);

  QPushButton *btnPaySupp =
      new QPushButton(QStringLiteral("💳 Record Payment (F8)"), headerFrame);
  btnPaySupp->setShortcut(QKeySequence(Qt::Key_F8));
  btnPaySupp->setStyleSheet(erpBtnStyle);

  QPushButton *btnRefresh =
      new QPushButton(QStringLiteral("🔄 Refresh (F5)"), headerFrame);
  btnRefresh->setShortcut(QKeySequence(Qt::Key_F5));
  btnRefresh->setStyleSheet(erpBtnStyle);

  headerLayout->addWidget(btnInward);
  headerLayout->addWidget(btnNewSupp);
  headerLayout->addWidget(btnPaySupp);
  headerLayout->addWidget(btnRefresh);
  mainLayout->addWidget(headerFrame);

  // ==========================================
  // Sub-tabs
  // ==========================================
  m_tabs = new QTabWidget(this);
  m_tabs->setStyleSheet(QStringLiteral(
      "QTabWidget::pane { border: 1px solid #8FAEC7; background-color: "
      "#D9E8F5; top: -1px; } "
      "QTabBar::tab { background-color: #D0E2F2; color: #0A294A; border: 1px "
      "solid #8FAEC7; border-bottom: none; "
      "border-top-left-radius: 3px; border-top-right-radius: 3px; padding: 3px "
      "14px; font-size: 8.5pt; font-weight: 600; margin-right: 2px; } "
      "QTabBar::tab:selected { background-color: #FFFFFF; color: #000080; "
      "border-bottom: 2px solid #0055A5; font-weight: bold; }"));

  const QString tableStyle = QStringLiteral(
      "QTableWidget { background-color: #FFFFFF; alternate-background-color: "
      "#F8FBFE; "
      "gridline-color: #C2D7EA; border: 1px solid #7A9EB9; font-size: 8.5pt; "
      "color: #0A294A; } "
      "QTableWidget::item { padding: 2px 4px; } "
      "QTableWidget::item:selected { background-color: #2D82D6; color: "
      "#FFFFFF; font-weight: bold; } "
      "QHeaderView::section { background: qlineargradient(x1:0, y1:0, x2:0, "
      "y2:1, stop:0 #B3D4F0, stop:1 #8DBDE0); "
      "color: #0A294A; font-weight: bold; font-size: 8pt; border: none; "
      "border-top: 1px solid #C2D7EA; "
      "border-bottom: 1px solid #6895BA; border-right: 1px solid #99BEDC; "
      "padding: 3px 5px; }");

  // Tab 1: Purchase Invoices & Orders
  QWidget *tabInvoices = new QWidget(m_tabs);
  QVBoxLayout *l1 = new QVBoxLayout(tabInvoices);
  l1->setContentsMargins(6, 6, 6, 6);
  l1->setSpacing(4);

  m_tableInvoices = new QTableWidget(tabInvoices);
  m_tableInvoices->setColumnCount(7);
  m_tableInvoices->setHorizontalHeaderLabels(
      QStringList() << QStringLiteral("Purchase Inv #")
                    << QStringLiteral("Vendor Inv #")
                    << QStringLiteral("Supplier Name") << QStringLiteral("Date")
                    << QStringLiteral("Subtotal")
                    << QStringLiteral("Total Amount")
                    << QStringLiteral("Payment Status"));
  m_tableInvoices->horizontalHeader()->setSectionResizeMode(
      QHeaderView::ResizeToContents);
  m_tableInvoices->horizontalHeader()->setSectionResizeMode(
      2, QHeaderView::Stretch);
  m_tableInvoices->verticalHeader()->setVisible(false);
  m_tableInvoices->verticalHeader()->setDefaultSectionSize(22);
  m_tableInvoices->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tableInvoices->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tableInvoices->setAlternatingRowColors(true);
  m_tableInvoices->setStyleSheet(tableStyle);
  l1->addWidget(m_tableInvoices);

  // Footer for Tab 1
  QFrame *footerInvoices = new QFrame(tabInvoices);
  footerInvoices->setFixedHeight(28);
  footerInvoices->setStyleSheet(QStringLiteral(
      "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; "
      "border-radius: 2px; }"));
  QHBoxLayout *footerInvLayout = new QHBoxLayout(footerInvoices);
  footerInvLayout->setContentsMargins(6, 1, 6, 1);
  m_lblInvoicesSummary = new QLabel(footerInvoices);
  m_lblInvoicesSummary->setStyleSheet(
      QStringLiteral("font-weight: bold; color: #000080; font-size: 8.5pt;"));
  footerInvLayout->addWidget(m_lblInvoicesSummary);
  footerInvLayout->addStretch();
  l1->addWidget(footerInvoices);

  m_tabs->addTab(tabInvoices,
                 QStringLiteral("📑 1. Purchase Invoices & Orders"));

  // Tab 2: Supplier Registry
  QWidget *tabSuppliers = new QWidget(m_tabs);
  QVBoxLayout *l2 = new QVBoxLayout(tabSuppliers);
  l2->setContentsMargins(6, 6, 6, 6);
  l2->setSpacing(4);

  m_tableSuppliers = new QTableWidget(tabSuppliers);
  m_tableSuppliers->setColumnCount(8);
  m_tableSuppliers->setHorizontalHeaderLabels(
      QStringList() << QStringLiteral("Code") << QStringLiteral("Supplier Name")
                    << QStringLiteral("Contact Person")
                    << QStringLiteral("Phone") << QStringLiteral("Email")
                    << QStringLiteral("Payment Terms")
                    << QStringLiteral("Outstanding Payable")
                    << QStringLiteral("Status"));
  m_tableSuppliers->horizontalHeader()->setSectionResizeMode(
      QHeaderView::ResizeToContents);
  m_tableSuppliers->horizontalHeader()->setSectionResizeMode(
      1, QHeaderView::Stretch);
  m_tableSuppliers->verticalHeader()->setVisible(false);
  m_tableSuppliers->verticalHeader()->setDefaultSectionSize(22);
  m_tableSuppliers->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tableSuppliers->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tableSuppliers->setAlternatingRowColors(true);
  m_tableSuppliers->setStyleSheet(tableStyle);
  l2->addWidget(m_tableSuppliers);

  // Footer for Tab 2
  QFrame *footerSuppliers = new QFrame(tabSuppliers);
  footerSuppliers->setFixedHeight(28);
  footerSuppliers->setStyleSheet(QStringLiteral(
      "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; "
      "border-radius: 2px; }"));
  QHBoxLayout *footerSuppLayout = new QHBoxLayout(footerSuppliers);
  footerSuppLayout->setContentsMargins(6, 1, 6, 1);
  m_lblSuppliersSummary = new QLabel(footerSuppliers);
  m_lblSuppliersSummary->setStyleSheet(
      QStringLiteral("font-weight: bold; color: #000080; font-size: 8.5pt;"));
  footerSuppLayout->addWidget(m_lblSuppliersSummary);
  footerSuppLayout->addStretch();
  l2->addWidget(footerSuppliers);

  m_tabs->addTab(tabSuppliers, QStringLiteral("🏢 2. Supplier Registry"));

  mainLayout->addWidget(m_tabs, 1);

  // ==========================================
  // Bottom Function Key Bar (Matching Reference Image)
  // ==========================================
  QFrame *fnBar = new QFrame(this);
  fnBar->setFixedHeight(32);
  fnBar->setStyleSheet(QStringLiteral(
      "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; "
      "border-radius: 2px; }"));
  QHBoxLayout *fnLayout = new QHBoxLayout(fnBar);
  fnLayout->setContentsMargins(4, 2, 4, 2);
  fnLayout->setSpacing(3);

  const QString fnBtnStyle = QStringLiteral(
      "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
      "stop:0 #FFFFFF, stop:1 #E0EBF5); "
      "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: "
      "2px 8px; font-size: 8.5pt; font-weight: bold; } "
      "QPushButton:hover { background: #EDF4FB; border-color: #4A82AD; }");

  QPushButton *bF1 = new QPushButton(QStringLiteral("F1 - Help"), fnBar);
  bF1->setStyleSheet(fnBtnStyle);
  bF1->setShortcut(QKeySequence(Qt::Key_F1));

  QPushButton *bF2 = new QPushButton(QStringLiteral("F2 - Search"), fnBar);
  bF2->setStyleSheet(fnBtnStyle);
  bF2->setShortcut(QKeySequence(Qt::Key_F2));

  QPushButton *bF6 = new QPushButton(QStringLiteral("F6 - Inward GRN"), fnBar);
  bF6->setStyleSheet(QStringLiteral(
      "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
      "stop:0 #E8F5E9, stop:1 #C8E6C9); "
      "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; "
      "padding: 2px 10px; font-size: 8.5pt; font-weight: bold; } "
      "QPushButton:hover { background: #C8E6C9; }"));
  bF6->setShortcut(QKeySequence(Qt::Key_F6));
  connect(bF6, &QPushButton::clicked, this,
          &PurchasesPage::onNewInwardReceiptClicked);

  QPushButton *bF7 =
      new QPushButton(QStringLiteral("F7 - New Supplier"), fnBar);
  bF7->setStyleSheet(fnBtnStyle);
  bF7->setShortcut(QKeySequence(Qt::Key_F7));
  connect(bF7, &QPushButton::clicked, this,
          &PurchasesPage::onNewSupplierClicked);

  QPushButton *bF8 =
      new QPushButton(QStringLiteral("F8 - Supplier Payment"), fnBar);
  bF8->setStyleSheet(fnBtnStyle);
  bF8->setShortcut(QKeySequence(Qt::Key_F8));
  connect(bF8, &QPushButton::clicked, this,
          &PurchasesPage::onRecordPaymentClicked);

  QPushButton *bF9 = new QPushButton(QStringLiteral("F9 - Export CSV"), fnBar);
  bF9->setStyleSheet(fnBtnStyle);
  bF9->setShortcut(QKeySequence(Qt::Key_F9));
  connect(bF9, &QPushButton::clicked, this, &PurchasesPage::onExportCsvClicked);

  QPushButton *bEsc = new QPushButton(QStringLiteral("Esc - Cancel"), fnBar);
  bEsc->setStyleSheet(fnBtnStyle);
  bEsc->setShortcut(QKeySequence(Qt::Key_Escape));

  fnLayout->addWidget(bF1);
  fnLayout->addWidget(bF2);
  fnLayout->addWidget(bF6);
  fnLayout->addWidget(bF7);
  fnLayout->addWidget(bF8);
  fnLayout->addWidget(bF9);
  fnLayout->addStretch();
  fnLayout->addWidget(bEsc);

  mainLayout->addWidget(fnBar);

  connect(btnInward, &QPushButton::clicked, this,
          &PurchasesPage::onNewInwardReceiptClicked);
  connect(btnNewSupp, &QPushButton::clicked, this,
          &PurchasesPage::onNewSupplierClicked);
  connect(btnPaySupp, &QPushButton::clicked, this,
          &PurchasesPage::onRecordPaymentClicked);
  connect(btnRefresh, &QPushButton::clicked, this, &PurchasesPage::reloadData);
}

void PurchasesPage::switchToTab(int tabIndex) {
  if (m_tabs && tabIndex >= 0 && tabIndex < m_tabs->count()) {
    m_tabs->setCurrentIndex(tabIndex);
  }
}

void PurchasesPage::reloadData() {
  loadInvoices();
  loadSuppliers();
}

void PurchasesPage::loadInvoices() {
  auto list = DatabaseManager::instance().getPurchaseInvoices();
  m_tableInvoices->setRowCount(list.size());

  double grandTotal = 0.0;
  QString cur = DatabaseManager::instance().currencySymbol();

  for (int r = 0; r < list.size(); ++r) {
    const auto &pi = list[r];
    grandTotal += pi.totalAmount;

    QTableWidgetItem *invItem = new QTableWidgetItem(pi.invoiceNo);
    invItem->setForeground(QBrush(QColor("#000080")));
    invItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
    m_tableInvoices->setItem(r, 0, invItem);

    m_tableInvoices->setItem(r, 1, new QTableWidgetItem(pi.supplierInvoiceNo));
    m_tableInvoices->setItem(r, 2, new QTableWidgetItem(pi.supplierName));
    m_tableInvoices->setItem(r, 3, new QTableWidgetItem(pi.invoiceDate));

    QTableWidgetItem *subItem = new QTableWidgetItem(
        QStringLiteral("%1%2").arg(cur).arg(pi.subtotal, 0, 'f', 2));
    subItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_tableInvoices->setItem(r, 4, subItem);

    QTableWidgetItem *totItem = new QTableWidgetItem(
        QStringLiteral("%1%2").arg(cur).arg(pi.totalAmount, 0, 'f', 2));
    totItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    totItem->setForeground(QBrush(QColor("#000080")));
    totItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
    m_tableInvoices->setItem(r, 5, totItem);

    QTableWidgetItem *statItem = new QTableWidgetItem(pi.paymentStatus);
    statItem->setTextAlignment(Qt::AlignCenter);
    if (pi.paymentStatus == "Unpaid") {
      statItem->setForeground(QBrush(QColor("#B71C1C")));
      statItem->setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::Bold));
    } else {
      statItem->setForeground(QBrush(QColor("#1B5E20")));
      statItem->setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::Bold));
    }
    m_tableInvoices->setItem(r, 6, statItem);
  }

  m_lblInvoicesSummary->setText(
      QStringLiteral("Total Invoices: %1 | Aggregate Procurement: %2%3")
          .arg(list.size())
          .arg(cur)
          .arg(grandTotal, 0, 'f', 2));
}

void PurchasesPage::loadSuppliers() {
  auto supps = DatabaseManager::instance().searchSuppliers();
  m_tableSuppliers->setRowCount(supps.size());

  double totalPayables = 0.0;
  QString cur = DatabaseManager::instance().currencySymbol();

  for (int r = 0; r < supps.size(); ++r) {
    const auto &s = supps[r];
    totalPayables += s.outstandingBalance;

    QTableWidgetItem *codeItem = new QTableWidgetItem(s.code);
    codeItem->setForeground(QBrush(QColor("#000080")));
    codeItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
    m_tableSuppliers->setItem(r, 0, codeItem);

    m_tableSuppliers->setItem(r, 1, new QTableWidgetItem(s.name));
    m_tableSuppliers->setItem(r, 2, new QTableWidgetItem(s.contactPerson));
    m_tableSuppliers->setItem(r, 3, new QTableWidgetItem(s.phone));
    m_tableSuppliers->setItem(r, 4, new QTableWidgetItem(s.email));
    m_tableSuppliers->setItem(r, 5, new QTableWidgetItem(s.paymentTerms));

    QTableWidgetItem *balItem = new QTableWidgetItem(
        QStringLiteral("%1%2").arg(cur).arg(s.outstandingBalance, 0, 'f', 2));
    balItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    if (s.outstandingBalance > 0.0) {
      balItem->setForeground(QBrush(QColor("#B71C1C")));
      balItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
    }
    m_tableSuppliers->setItem(r, 6, balItem);

    QTableWidgetItem *statItem = new QTableWidgetItem(
        s.isActive ? QStringLiteral("Active") : QStringLiteral("Inactive"));
    statItem->setTextAlignment(Qt::AlignCenter);
    if (s.isActive)
      statItem->setForeground(QBrush(QColor("#1B5E20")));
    else
      statItem->setForeground(QBrush(QColor("#757575")));
    m_tableSuppliers->setItem(r, 7, statItem);
  }

  m_lblSuppliersSummary->setText(
      QStringLiteral("Registered Suppliers: %1 | Accounts Payable: %2%3")
          .arg(supps.size())
          .arg(cur)
          .arg(totalPayables, 0, 'f', 2));
}

void PurchasesPage::onNewInwardReceiptClicked() {
  auto supps = DatabaseManager::instance().searchSuppliers();
  auto prods = DatabaseManager::instance().searchProducts();
  if (supps.isEmpty() || prods.isEmpty())
    return;

  QDialog dlg(this);
  dlg.setWindowTitle(
      QStringLiteral("Goods Receipt Note (GRN) / Purchase Inward"));
  dlg.resize(480, 360);
  dlg.setStyleSheet(QStringLiteral(
      "QDialog { background-color: #D9E8F5; } "
      "QLabel { color: #0A294A; font-weight: bold; font-size: 8.5pt; } "
      "QLineEdit, QComboBox, QDoubleSpinBox { background: #FFFFFF; border: 1px "
      "solid #7A9EB9; "
      "border-radius: 2px; padding: 3px 6px; color: #000080; font-weight: "
      "bold; font-size: 9pt; } "
      "QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus { border: 1px "
      "solid #0055A5; background-color: #FFFFF2; }"));

  QVBoxLayout *dlgLayout = new QVBoxLayout(&dlg);
  dlgLayout->setContentsMargins(12, 12, 12, 12);
  dlgLayout->setSpacing(8);

  QFrame *formFrame = new QFrame(&dlg);
  formFrame->setStyleSheet(QStringLiteral(
      "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; "
      "border-radius: 3px; padding: 8px; }"));
  QFormLayout *form = new QFormLayout(formFrame);
  form->setSpacing(6);

  QComboBox *comboSupp = new QComboBox(formFrame);
  for (const auto &s : supps) {
    comboSupp->addItem(s.name, s.id);
  }

  QLineEdit *txtSupplierInv = new QLineEdit(formFrame);
  txtSupplierInv->setPlaceholderText(QStringLiteral("Supplier bill/challan #"));

  QComboBox *comboProd = new QComboBox(formFrame);
  for (const auto &p : prods) {
    comboProd->addItem(QStringLiteral("%1 (%2)").arg(p.name, p.sku), p.id);
  }

  QDoubleSpinBox *spinQty = new QDoubleSpinBox(formFrame);
  spinQty->setRange(1.0, 10000.0);
  spinQty->setValue(10.0);

  QDoubleSpinBox *spinCost = new QDoubleSpinBox(formFrame);
  spinCost->setRange(0.01, 100000.0);
  spinCost->setValue(prods[0].costPrice);

  connect(comboProd, QOverload<int>::of(&QComboBox::currentIndexChanged),
          [&]() {
            int pid = comboProd->currentData().toInt();
            auto p = DatabaseManager::instance().getProductById(pid);
            spinCost->setValue(p.costPrice);
          });

  form->addRow(QStringLiteral("Select Supplier:"), comboSupp);
  form->addRow(QStringLiteral("Supplier Invoice / Challan #:"), txtSupplierInv);
  form->addRow(QStringLiteral("Product to Receive:"), comboProd);
  form->addRow(QStringLiteral("Received Quantity:"), spinQty);
  form->addRow(QStringLiteral("Agreed Unit Cost:"), spinCost);

  dlgLayout->addWidget(formFrame);

  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();

  QPushButton *btnOk =
      new QPushButton(QStringLiteral("✅ Post Inward to Stock (F9)"), &dlg);
  btnOk->setShortcut(QKeySequence(Qt::Key_F9));
  btnOk->setStyleSheet(QStringLiteral(
      "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
      "stop:0 #E8F5E9, stop:1 #C8E6C9); "
      "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; "
      "padding: 4px 14px; font-weight: bold; font-size: 8.5pt; } "
      "QPushButton:hover { background: #C8E6C9; }"));

  QPushButton *btnCancel =
      new QPushButton(QStringLiteral("❌ Cancel (Esc)"), &dlg);
  btnCancel->setShortcut(QKeySequence(Qt::Key_Escape));
  btnCancel->setStyleSheet(QStringLiteral(
      "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
      "stop:0 #FFFFFF, stop:1 #E0EBF5); "
      "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: "
      "4px 14px; font-weight: bold; font-size: 8.5pt; } "
      "QPushButton:hover { background: #EDF4FB; }"));

  btnLayout->addWidget(btnOk);
  btnLayout->addWidget(btnCancel);
  dlgLayout->addLayout(btnLayout);

  connect(btnOk, &QPushButton::clicked, &dlg, &QDialog::accept);
  connect(btnCancel, &QPushButton::clicked, &dlg, &QDialog::reject);

  if (dlg.exec() == QDialog::Accepted) {
    int sid = comboSupp->currentData().toInt();
    int pid = comboProd->currentData().toInt();
    auto prod = DatabaseManager::instance().getProductById(pid);

    RetailNext::CartItem item;
    item.productId = pid;
    item.qty = spinQty->value();
    item.unitPrice = spinCost->value();
    item.name = prod.name;
    item.barcode = prod.barcode;
    double totalCost = item.qty * item.unitPrice;

    QList<RetailNext::CartItem> items;
    items.append(item);

    QString sinv = txtSupplierInv->text().trimmed();
    if (sinv.isEmpty())
      sinv = QStringLiteral("CH-%1").arg(QDateTime::currentMSecsSinceEpoch() %
                                         10000);

    if (DatabaseManager::instance().recordPurchaseReceipt(
            QStringLiteral("PO-AUTO"), sid, sinv, items, totalCost,
            QStringLiteral("Stock receipt inward"))) {
      QMessageBox::information(
          this, QStringLiteral("Receipt Posted"),
          QStringLiteral("Successfully received %1 %2 of '%3' into stock.")
              .arg(item.qty)
              .arg(prod.unitCode, prod.name));
      reloadData();
    } else {
      QMessageBox::critical(this, QStringLiteral("Error"),
                            QStringLiteral("Failed to post goods receipt."));
    }
  }
}

void PurchasesPage::onNewSupplierClicked() {
  QDialog dlg(this);
  dlg.setWindowTitle(QStringLiteral("Add New Supplier"));
  dlg.resize(440, 340);
  dlg.setStyleSheet(QStringLiteral(
      "QDialog { background-color: #D9E8F5; } "
      "QLabel { color: #0A294A; font-weight: bold; font-size: 8.5pt; } "
      "QLineEdit, QComboBox { background: #FFFFFF; border: 1px solid #7A9EB9; "
      "border-radius: 2px; padding: 3px 6px; color: #000080; font-weight: "
      "bold; font-size: 9pt; } "
      "QLineEdit:focus, QComboBox:focus { border: 1px solid #0055A5; "
      "background-color: #FFFFF2; }"));

  QVBoxLayout *dlgLayout = new QVBoxLayout(&dlg);
  dlgLayout->setContentsMargins(12, 12, 12, 12);
  dlgLayout->setSpacing(8);

  QFrame *formFrame = new QFrame(&dlg);
  formFrame->setStyleSheet(QStringLiteral(
      "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; "
      "border-radius: 3px; padding: 8px; }"));
  QFormLayout *form = new QFormLayout(formFrame);
  form->setSpacing(6);

  QLineEdit *txtName = new QLineEdit(formFrame);
  QLineEdit *txtContact = new QLineEdit(formFrame);
  QLineEdit *txtPhone = new QLineEdit(formFrame);
  QLineEdit *txtEmail = new QLineEdit(formFrame);
  QLineEdit *txtAddress = new QLineEdit(formFrame);
  QComboBox *comboTerms = new QComboBox(formFrame);
  comboTerms->addItems(QStringList()
                       << "Immediate" << "Net 15" << "Net 30" << "Net 60");

  form->addRow(QStringLiteral("Supplier / Company Name: *"), txtName);
  form->addRow(QStringLiteral("Contact Person:"), txtContact);
  form->addRow(QStringLiteral("Phone Number:"), txtPhone);
  form->addRow(QStringLiteral("Email:"), txtEmail);
  form->addRow(QStringLiteral("Address:"), txtAddress);
  form->addRow(QStringLiteral("Payment Terms:"), comboTerms);

  dlgLayout->addWidget(formFrame);

  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();
  QPushButton *btnOk =
      new QPushButton(QStringLiteral("💾 Save Supplier (F9)"), &dlg);
  btnOk->setShortcut(QKeySequence(Qt::Key_F9));
  btnOk->setStyleSheet(QStringLiteral(
      "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
      "stop:0 #E8F5E9, stop:1 #C8E6C9); "
      "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; "
      "padding: 4px 14px; font-weight: bold; font-size: 8.5pt; } "
      "QPushButton:hover { background: #C8E6C9; }"));

  QPushButton *btnCancel =
      new QPushButton(QStringLiteral("❌ Cancel (Esc)"), &dlg);
  btnCancel->setShortcut(QKeySequence(Qt::Key_Escape));
  btnCancel->setStyleSheet(QStringLiteral(
      "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
      "stop:0 #FFFFFF, stop:1 #E0EBF5); "
      "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: "
      "4px 14px; font-weight: bold; font-size: 8.5pt; } "
      "QPushButton:hover { background: #EDF4FB; }"));

  btnLayout->addWidget(btnOk);
  btnLayout->addWidget(btnCancel);
  dlgLayout->addLayout(btnLayout);

  connect(btnOk, &QPushButton::clicked, &dlg, &QDialog::accept);
  connect(btnCancel, &QPushButton::clicked, &dlg, &QDialog::reject);

  if (dlg.exec() == QDialog::Accepted) {
    if (txtName->text().trimmed().isEmpty())
      return;

    RetailNext::SupplierRecord s;
    s.code = QStringLiteral("SUP-%1").arg(QDateTime::currentMSecsSinceEpoch() %
                                          1000);
    s.name = txtName->text().trimmed();
    s.contactPerson = txtContact->text().trimmed();
    s.phone = txtPhone->text().trimmed();
    s.email = txtEmail->text().trimmed();
    s.address = txtAddress->text().trimmed();
    s.paymentTerms = comboTerms->currentText();
    s.outstandingBalance = 0.0;
    s.creditLimit = 10000.0;
    s.isActive = true;

    if (DatabaseManager::instance().saveSupplier(s, true)) {
      QMessageBox::information(
          this, QStringLiteral("Saved"),
          QStringLiteral("Supplier '%1' registered.").arg(s.name));
      reloadData();
    }
  }
}

void PurchasesPage::onRecordPaymentClicked() {
  auto supps = DatabaseManager::instance().searchSuppliers();
  if (supps.isEmpty())
    return;

  QStringList options;
  for (const auto &s : supps) {
    options.append(QStringLiteral("%1 (Payable: $%2)")
                        .arg(s.name)
                        .arg(s.outstandingBalance, 0, 'f', 2));
  }

  bool ok = false;
  QString sel = QInputDialog::getItem(this, QStringLiteral("Supplier Payment"),
                                      QStringLiteral("Select supplier to pay:"),
                                      options, 0, false, &ok);
  if (!ok || sel.isEmpty())
    return;

  int idx = options.indexOf(sel);
  auto supp = supps[idx];

  double amt = QInputDialog::getDouble(
      this, QStringLiteral("Payment Amount"),
      QStringLiteral("Enter payment amount to '%1':").arg(supp.name),
      supp.outstandingBalance > 0 ? supp.outstandingBalance : 100.0, 1.0,
      100000.0, 2, &ok);
  if (!ok)
    return;

  QString mode = QInputDialog::getItem(
      this, QStringLiteral("Payment Mode"),
      QStringLiteral("Select disbursement tender:"),
      QStringList() << QStringLiteral("Bank Transfer") << QStringLiteral("Cash")
                    << QStringLiteral("Cheque"),
      0, false, &ok);
  if (!ok)
    return;

  if (DatabaseManager::instance().recordSupplierPayment(
          supp.id, amt, mode, QStringLiteral("Vendor settlement payment"))) {
    QMessageBox::information(this, QStringLiteral("Payment Recorded"),
                             QStringLiteral("Payment of $%1 recorded to %2.")
                                 .arg(amt, 0, 'f', 2)
                                 .arg(supp.name));
    reloadData();
  }
}

void PurchasesPage::onExportCsvClicked() {
  QTableWidget *currentTable =
      (m_tabs && m_tabs->currentIndex() == 1) ? m_tableSuppliers : m_tableInvoices;
  if (!currentTable || currentTable->rowCount() == 0) {
    QMessageBox::information(this, QStringLiteral("Export CSV"),
                             QStringLiteral("No records available to export."));
    return;
  }

  QString defaultName = (m_tabs && m_tabs->currentIndex() == 1)
                            ? QStringLiteral("Suppliers.csv")
                            : QStringLiteral("PurchaseInvoices.csv");
  QString filePath = QFileDialog::getSaveFileName(
      this, QStringLiteral("Export Data to CSV"), defaultName,
      QStringLiteral("CSV Files (*.csv)"));
  if (filePath.isEmpty())
    return;

  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::critical(this, QStringLiteral("Export Failed"),
                          QStringLiteral("Cannot open file for writing."));
    return;
  }

  QTextStream out(&file);
  QStringList headerLabels;
  for (int c = 0; c < currentTable->columnCount(); ++c) {
    headerLabels << QStringLiteral("\"%1\"").arg(
        currentTable->horizontalHeaderItem(c)->text());
  }
  out << headerLabels.join(QStringLiteral(",")) << "\n";

  for (int r = 0; r < currentTable->rowCount(); ++r) {
    QStringList rowValues;
    for (int c = 0; c < currentTable->columnCount(); ++c) {
      QTableWidgetItem *item = currentTable->item(r, c);
      QString text = item ? item->text() : QString();
      text.replace(QStringLiteral("\""), QStringLiteral("\"\""));
      rowValues << QStringLiteral("\"%1\"").arg(text);
    }
    out << rowValues.join(QStringLiteral(",")) << "\n";
  }

  file.close();
  QMessageBox::information(
      this, QStringLiteral("Export Completed"),
      QStringLiteral("Successfully exported %1 records to:\n%2")
          .arg(currentTable->rowCount())
          .arg(filePath));
}
