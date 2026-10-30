#include "FinancePage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QTextStream>
#include <QShortcut>

FinancePage::FinancePage(QWidget *parent)
    : QWidget(parent)
    , m_tabs(nullptr)
    , m_lblRegCashier(nullptr)
    , m_lblRegOpened(nullptr)
    , m_lblRegOpeningFloat(nullptr)
    , m_lblRegCashSales(nullptr)
    , m_lblRegCashIn(nullptr)
    , m_lblRegCashOut(nullptr)
    , m_lblRegExpected(nullptr)
    , m_tableExpenses(nullptr)
    , m_lblExpensesSummary(nullptr)
{
    setupUI();
    reloadData();
}

void FinancePage::setupUI() {
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

    QLabel *lblIcon = new QLabel(QStringLiteral("💰"), headerFrame);
    lblIcon->setStyleSheet(QStringLiteral("font-size: 22pt; padding: 0px; margin: 0px;"));
    titleGroup->addWidget(lblIcon);

    QVBoxLayout *titleText = new QVBoxLayout();
    titleText->setSpacing(1);
    QLabel *lblTitle = new QLabel(QStringLiteral("Finance & Cash Drawer Management"), headerFrame);
    lblTitle->setStyleSheet(QStringLiteral("font-size: 13pt; font-weight: 800; color: #0A294A; margin: 0;"));
    QLabel *lblSubtitle = new QLabel(QStringLiteral("Cash drawer reconciliation, daily shift closing, petty cash payouts, and store expenses."), headerFrame);
    lblSubtitle->setStyleSheet(QStringLiteral("font-size: 8.5pt; color: #4A6B82; font-style: italic; margin: 0;"));
    titleText->addWidget(lblTitle);
    titleText->addWidget(lblSubtitle);
    titleGroup->addLayout(titleText);
    headerLayout->addLayout(titleGroup);
    headerLayout->addStretch();

    const QString erpBtnStyle = QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; border-color: #4A82AD; }"
    );

    QPushButton *btnCashIn = new QPushButton(QStringLiteral("💵 Cash In (F2)"), headerFrame);
    btnCashIn->setShortcut(QKeySequence(Qt::Key_F2));
    btnCashIn->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #C8E6C9; }"
    ));

    QPushButton *btnCashOut = new QPushButton(QStringLiteral("💸 Cash Out (F3)"), headerFrame);
    btnCashOut->setShortcut(QKeySequence(Qt::Key_F3));
    btnCashOut->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFEBEE, stop:1 #FFCDD2); "
        "color: #B71C1C; border: 1px solid #EF9A9A; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #FFCDD2; }"
    ));

    QPushButton *btnCloseReg = new QPushButton(QStringLiteral("🔒 Day Closing (F10)"), headerFrame);
    btnCloseReg->setShortcut(QKeySequence(Qt::Key_F10));
    btnCloseReg->setStyleSheet(erpBtnStyle);

    QPushButton *btnExpense = new QPushButton(QStringLiteral("➕ Record Expense (F8)"), headerFrame);
    btnExpense->setShortcut(QKeySequence(Qt::Key_F8));
    btnExpense->setStyleSheet(erpBtnStyle);

    QPushButton *btnRefresh = new QPushButton(QStringLiteral("🔄 Refresh (F5)"), headerFrame);
    btnRefresh->setShortcut(QKeySequence(Qt::Key_F5));
    btnRefresh->setStyleSheet(erpBtnStyle);

    headerLayout->addWidget(btnCashIn);
    headerLayout->addWidget(btnCashOut);
    headerLayout->addWidget(btnCloseReg);
    headerLayout->addWidget(btnExpense);
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

    // Tab 1: Cash Register Session & Drawer Status
    QWidget *tabRegister = new QWidget(m_tabs);
    QVBoxLayout *l1 = new QVBoxLayout(tabRegister);
    l1->setContentsMargins(10, 10, 10, 10);
    l1->setSpacing(10);

    QFrame *card = new QFrame(tabRegister);
    card->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #FFFFFF; border: 1px solid #8FAEC7; border-radius: 4px; padding: 16px; }"
    ));
    QGridLayout *g = new QGridLayout(card);
    g->setSpacing(12);

    const QString lblKeyStyle = QStringLiteral("font-weight: bold; color: #0A294A; font-size: 9.5pt;");
    const QString lblValStyle = QStringLiteral("font-weight: bold; color: #000080; font-size: 10pt;");

    QLabel *kCashier = new QLabel(QStringLiteral("Active Cashier:"), card);
    kCashier->setStyleSheet(lblKeyStyle);
    m_lblRegCashier = new QLabel(card);
    m_lblRegCashier->setStyleSheet(lblValStyle);

    QLabel *kOpened = new QLabel(QStringLiteral("Shift Started At:"), card);
    kOpened->setStyleSheet(lblKeyStyle);
    m_lblRegOpened = new QLabel(card);
    m_lblRegOpened->setStyleSheet(lblValStyle);

    QLabel *kFloat = new QLabel(QStringLiteral("Opening Cash Float:"), card);
    kFloat->setStyleSheet(lblKeyStyle);
    m_lblRegOpeningFloat = new QLabel(card);
    m_lblRegOpeningFloat->setStyleSheet(lblValStyle);

    QLabel *kSales = new QLabel(QStringLiteral("Cash Sales Inward:"), card);
    kSales->setStyleSheet(lblKeyStyle);
    m_lblRegCashSales = new QLabel(card);
    m_lblRegCashSales->setStyleSheet(lblValStyle);

    QLabel *kIn = new QLabel(QStringLiteral("Cash In (Manual Additions):"), card);
    kIn->setStyleSheet(lblKeyStyle);
    m_lblRegCashIn = new QLabel(card);
    m_lblRegCashIn->setStyleSheet(lblValStyle);

    QLabel *kOut = new QLabel(QStringLiteral("Cash Out (Payouts/Expenses):"), card);
    kOut->setStyleSheet(lblKeyStyle);
    m_lblRegCashOut = new QLabel(card);
    m_lblRegCashOut->setStyleSheet(lblValStyle);

    QLabel *kExpected = new QLabel(QStringLiteral("EXPECTED CASH IN DRAWER:"), card);
    kExpected->setStyleSheet(QStringLiteral("font-weight: 800; color: #0A294A; font-size: 11pt;"));
    m_lblRegExpected = new QLabel(card);
    m_lblRegExpected->setStyleSheet(QStringLiteral("font-size: 24pt; font-weight: 800; color: #1B5E20;"));

    g->addWidget(kCashier, 0, 0);
    g->addWidget(m_lblRegCashier, 0, 1);
    g->addWidget(kOpened, 0, 2);
    g->addWidget(m_lblRegOpened, 0, 3);

    g->addWidget(kFloat, 1, 0);
    g->addWidget(m_lblRegOpeningFloat, 1, 1);
    g->addWidget(kSales, 1, 2);
    g->addWidget(m_lblRegCashSales, 1, 3);

    g->addWidget(kIn, 2, 0);
    g->addWidget(m_lblRegCashIn, 2, 1);
    g->addWidget(kOut, 2, 2);
    g->addWidget(m_lblRegCashOut, 2, 3);

    g->addWidget(kExpected, 3, 0);
    g->addWidget(m_lblRegExpected, 3, 1, 1, 3);

    l1->addWidget(card);
    l1->addStretch();
    m_tabs->addTab(tabRegister, QStringLiteral("💰 1. Active Cash Register & Shift"));

    // Tab 2: Expenses Log
    QWidget *tabExpenses = new QWidget(m_tabs);
    QVBoxLayout *l2 = new QVBoxLayout(tabExpenses);
    l2->setContentsMargins(6, 6, 6, 6);
    l2->setSpacing(4);

    m_tableExpenses = new QTableWidget(tabExpenses);
    m_tableExpenses->setColumnCount(8);
    m_tableExpenses->setHorizontalHeaderLabels(QStringList()
        << QStringLiteral("Expense #")
        << QStringLiteral("Timestamp")
        << QStringLiteral("Category")
        << QStringLiteral("Payee / Vendor")
        << QStringLiteral("Amount")
        << QStringLiteral("Payment Method")
        << QStringLiteral("Recorded By")
        << QStringLiteral("Notes"));
    m_tableExpenses->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableExpenses->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_tableExpenses->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Stretch);
    m_tableExpenses->verticalHeader()->setVisible(false);
    m_tableExpenses->verticalHeader()->setDefaultSectionSize(22);
    m_tableExpenses->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableExpenses->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableExpenses->setAlternatingRowColors(true);
    m_tableExpenses->setStyleSheet(QStringLiteral(
        "QTableWidget { background-color: #FFFFFF; alternate-background-color: #F8FBFE; "
        "gridline-color: #C2D7EA; border: 1px solid #7A9EB9; font-size: 8.5pt; color: #0A294A; } "
        "QTableWidget::item { padding: 2px 4px; } "
        "QTableWidget::item:selected { background-color: #2D82D6; color: #FFFFFF; font-weight: bold; } "
        "QHeaderView::section { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B3D4F0, stop:1 #8DBDE0); "
        "color: #0A294A; font-weight: bold; font-size: 8pt; border: none; border-top: 1px solid #C2D7EA; "
        "border-bottom: 1px solid #6895BA; border-right: 1px solid #99BEDC; padding: 3px 5px; }"
    ));
    l2->addWidget(m_tableExpenses);

    QFrame *footerExpenses = new QFrame(tabExpenses);
    footerExpenses->setFixedHeight(28);
    footerExpenses->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; }"
    ));
    QHBoxLayout *footerExpLayout = new QHBoxLayout(footerExpenses);
    footerExpLayout->setContentsMargins(6, 1, 6, 1);
    m_lblExpensesSummary = new QLabel(footerExpenses);
    m_lblExpensesSummary->setStyleSheet(QStringLiteral("font-weight: bold; color: #000080; font-size: 8.5pt;"));
    footerExpLayout->addWidget(m_lblExpensesSummary);
    footerExpLayout->addStretch();
    l2->addWidget(footerExpenses);

    m_tabs->addTab(tabExpenses, QStringLiteral("🧾 2. Store Operational Expenses"));

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

    QPushButton *bF2 = new QPushButton(QStringLiteral("F2 - Cash In"), fnBar);
    bF2->setStyleSheet(fnBtnStyle);
    bF2->setShortcut(QKeySequence(Qt::Key_F2));
    connect(bF2, &QPushButton::clicked, this, &FinancePage::onCashInClicked);

    QPushButton *bF3 = new QPushButton(QStringLiteral("F3 - Cash Out"), fnBar);
    bF3->setStyleSheet(fnBtnStyle);
    bF3->setShortcut(QKeySequence(Qt::Key_F3));
    connect(bF3, &QPushButton::clicked, this, &FinancePage::onCashOutClicked);

    QPushButton *bF8 = new QPushButton(QStringLiteral("F8 - Record Expense"), fnBar);
    bF8->setStyleSheet(fnBtnStyle);
    bF8->setShortcut(QKeySequence(Qt::Key_F8));
    connect(bF8, &QPushButton::clicked, this, &FinancePage::onAddExpenseClicked);

    QPushButton *bF9 = new QPushButton(QStringLiteral("F9 - Export CSV"), fnBar);
    bF9->setStyleSheet(fnBtnStyle);
    bF9->setShortcut(QKeySequence(Qt::Key_F9));
    connect(bF9, &QPushButton::clicked, this, &FinancePage::onExportCsvClicked);

    QPushButton *bF10 = new QPushButton(QStringLiteral("F10 - Day Closing"), fnBar);
    bF10->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFF8E1, stop:1 #FFE082); "
        "color: #E65100; border: 1px solid #FFD54F; border-radius: 2px; padding: 2px 10px; font-size: 8.5pt; font-weight: bold; } "
        "QPushButton:hover { background: #FFE082; }"
    ));
    bF10->setShortcut(QKeySequence(Qt::Key_F10));
    connect(bF10, &QPushButton::clicked, this, &FinancePage::onCloseRegisterClicked);

    QPushButton *bEsc = new QPushButton(QStringLiteral("Esc - Cancel"), fnBar);
    bEsc->setStyleSheet(fnBtnStyle);
    bEsc->setShortcut(QKeySequence(Qt::Key_Escape));

    fnLayout->addWidget(bF1);
    fnLayout->addWidget(bF2);
    fnLayout->addWidget(bF3);
    fnLayout->addWidget(bF8);
    fnLayout->addWidget(bF9);
    fnLayout->addWidget(bF10);
    fnLayout->addStretch();
    fnLayout->addWidget(bEsc);

    mainLayout->addWidget(fnBar);

    connect(btnCashIn, &QPushButton::clicked, this, &FinancePage::onCashInClicked);
    connect(btnCashOut, &QPushButton::clicked, this, &FinancePage::onCashOutClicked);
    connect(btnCloseReg, &QPushButton::clicked, this, &FinancePage::onCloseRegisterClicked);
    connect(btnExpense, &QPushButton::clicked, this, &FinancePage::onAddExpenseClicked);
    connect(btnRefresh, &QPushButton::clicked, this, &FinancePage::reloadData);
}

void FinancePage::switchToTab(int tabIndex) {
    if (m_tabs && tabIndex >= 0 && tabIndex < m_tabs->count()) {
        m_tabs->setCurrentIndex(tabIndex);
    }
}

void FinancePage::reloadData() {
    loadRegisterSession();
    loadExpenses();
}

void FinancePage::loadRegisterSession() {
    auto s = DatabaseManager::instance().getActiveCashSession();
    QString cur = DatabaseManager::instance().currencySymbol();

    m_lblRegCashier->setText(s.cashierName.isEmpty() ? DatabaseManager::instance().currentUser() : s.cashierName);
    m_lblRegOpened->setText(s.openedAt);
    m_lblRegOpeningFloat->setText(QStringLiteral("%1%2").arg(cur).arg(s.openingCash, 0, 'f', 2));
    m_lblRegCashSales->setText(QStringLiteral("%1%2").arg(cur).arg(s.cashSales, 0, 'f', 2));
    m_lblRegCashIn->setText(QStringLiteral("%1%2").arg(cur).arg(s.cashIn, 0, 'f', 2));
    m_lblRegCashOut->setText(QStringLiteral("%1%2").arg(cur).arg(s.cashOut, 0, 'f', 2));
    m_lblRegExpected->setText(QStringLiteral("%1%2").arg(cur).arg(s.expectedCash, 0, 'f', 2));
}

void FinancePage::loadExpenses() {
    auto list = DatabaseManager::instance().getExpenses();
    m_tableExpenses->setRowCount(list.size());

    double totalExp = 0.0;
    QString cur = DatabaseManager::instance().currencySymbol();

    for (int r = 0; r < list.size(); ++r) {
        const auto& e = list[r];
        totalExp += e.amount;

        QTableWidgetItem *expItem = new QTableWidgetItem(e.expenseNo);
        expItem->setForeground(QBrush(QColor("#000080")));
        expItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_tableExpenses->setItem(r, 0, expItem);

        m_tableExpenses->setItem(r, 1, new QTableWidgetItem(e.createdAt));
        m_tableExpenses->setItem(r, 2, new QTableWidgetItem(e.categoryName));
        m_tableExpenses->setItem(r, 3, new QTableWidgetItem(e.payee));

        QTableWidgetItem *amtItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(e.amount, 0, 'f', 2));
        amtItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        amtItem->setForeground(QBrush(QColor("#B71C1C")));
        amtItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_tableExpenses->setItem(r, 4, amtItem);

        QTableWidgetItem *mItem = new QTableWidgetItem(e.paymentMethod);
        mItem->setTextAlignment(Qt::AlignCenter);
        m_tableExpenses->setItem(r, 5, mItem);

        m_tableExpenses->setItem(r, 6, new QTableWidgetItem(e.createdBy));
        m_tableExpenses->setItem(r, 7, new QTableWidgetItem(e.notes));
    }

    m_lblExpensesSummary->setText(QStringLiteral("Total Expenses: %1 vouchers | Cumulative Payout: %2%3")
        .arg(list.size())
        .arg(cur)
        .arg(totalExp, 0, 'f', 2));
}

void FinancePage::onCashInClicked() {
    bool ok = false;
    double amt = QInputDialog::getDouble(this, QStringLiteral("Cash In"), QStringLiteral("Enter cash amount to add into drawer:"), 50.0, 1.0, 50000.0, 2, &ok);
    if (!ok) return;

    QString reason = QInputDialog::getText(this, QStringLiteral("Reason"), QStringLiteral("Reason for cash addition:"), QLineEdit::Normal, QStringLiteral("Additional change / opening float"), &ok);
    if (!ok) return;

    if (DatabaseManager::instance().recordCashInOut(amt, QStringLiteral("Cash In"), reason, QStringLiteral("MAN-IN"))) {
        QMessageBox::information(this, QStringLiteral("Success"), QStringLiteral("Added $%1 to drawer float.").arg(amt, 0, 'f', 2));
        reloadData();
    }
}

void FinancePage::onCashOutClicked() {
    bool ok = false;
    double amt = QInputDialog::getDouble(this, QStringLiteral("Cash Out"), QStringLiteral("Enter petty cash amount to remove:"), 20.0, 1.0, 50000.0, 2, &ok);
    if (!ok) return;

    QString reason = QInputDialog::getText(this, QStringLiteral("Reason"), QStringLiteral("Reason for petty cash withdrawal:"), QLineEdit::Normal, QStringLiteral("Store supplies / courier / tea"), &ok);
    if (!ok) return;

    if (DatabaseManager::instance().recordCashInOut(amt, QStringLiteral("Cash Out"), reason, QStringLiteral("MAN-OUT"))) {
        QMessageBox::information(this, QStringLiteral("Success"), QStringLiteral("Withdrew $%1 from drawer.").arg(amt, 0, 'f', 2));
        reloadData();
    }
}

void FinancePage::onCloseRegisterClicked() {
    auto s = DatabaseManager::instance().getActiveCashSession();
    if (s.id == 0) return;

    bool ok = false;
    double counted = QInputDialog::getDouble(
        this,
        QStringLiteral("Day Closing & Shift Reconciliation"),
        QStringLiteral("Expected Cash in drawer: $%1\n\nPlease count physical cash and enter total:").arg(s.expectedCash, 0, 'f', 2),
        s.expectedCash,
        0.0,
        100000.0,
        2,
        &ok
    );
    if (!ok) return;

    double variance = counted - s.expectedCash;
    QString varMsg = (variance == 0.0) ? QStringLiteral("PERFECT BALANCE ($0.00)") : QStringLiteral("VARIANCE: %1$%2 (%3)")
        .arg(variance > 0 ? "+" : "-")
        .arg(qAbs(variance), 0, 'f', 2)
        .arg(variance > 0 ? "OVERAGE" : "SHORTAGE");

    if (DatabaseManager::instance().closeCashRegisterSession(s.id, counted, QStringLiteral("Shift closed"))) {
        QMessageBox::information(
            this,
            QStringLiteral("Day Closing Complete"),
            QStringLiteral(
                "<h3>Shift Closed Successfully</h3>"
                "<p><b>Expected Cash:</b> $%1</p>"
                "<p><b>Counted Cash:</b> $%2</p>"
                "<p><b>Reconciliation:</b> %3</p>"
                "<hr/>"
                "<p>A new shift has been automatically initialized for the next cashier.</p>"
            ).arg(s.expectedCash, 0, 'f', 2).arg(counted, 0, 'f', 2).arg(varMsg)
        );
        reloadData();
    }
}

void FinancePage::onAddExpenseClicked() {
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Record Store Operational Expense"));
    dlg.resize(440, 340);
    dlg.setStyleSheet(QStringLiteral(
        "QDialog { background-color: #D9E8F5; } "
        "QLabel { color: #0A294A; font-weight: bold; font-size: 8.5pt; } "
        "QLineEdit, QComboBox, QDoubleSpinBox { background: #FFFFFF; border: 1px solid #7A9EB9; "
        "border-radius: 2px; padding: 3px 6px; color: #000080; font-weight: bold; font-size: 9pt; } "
        "QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus { border: 1px solid #0055A5; background-color: #FFFFF2; }"
    ));

    QVBoxLayout *dlgLayout = new QVBoxLayout(&dlg);
    dlgLayout->setContentsMargins(12, 12, 12, 12);
    dlgLayout->setSpacing(8);

    QFrame *formFrame = new QFrame(&dlg);
    formFrame->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 3px; padding: 8px; }"
    ));
    QFormLayout *form = new QFormLayout(formFrame);
    form->setSpacing(6);

    QComboBox *comboCat = new QComboBox(formFrame);
    comboCat->addItems(QStringList() << "Utilities" << "Packaging & Supplies" << "Repairs & Maintenance" << "Rent" << "Salaries / Wages" << "Logistics & Freight" << "Marketing" << "Miscellaneous");

    QLineEdit *txtPayee = new QLineEdit(formFrame);
    QDoubleSpinBox *spinAmt = new QDoubleSpinBox(formFrame);
    spinAmt->setRange(0.01, 100000.0);
    spinAmt->setValue(25.0);

    QComboBox *comboMode = new QComboBox(formFrame);
    comboMode->addItems(QStringList() << "Cash" << "Bank Transfer" << "Card" << "Cheque");

    QLineEdit *txtNotes = new QLineEdit(formFrame);

    form->addRow(QStringLiteral("Expense Category:"), comboCat);
    form->addRow(QStringLiteral("Payee / Vendor Name: *"), txtPayee);
    form->addRow(QStringLiteral("Amount: *"), spinAmt);
    form->addRow(QStringLiteral("Payment Tender:"), comboMode);
    form->addRow(QStringLiteral("Notes / Voucher Memo:"), txtNotes);

    dlgLayout->addWidget(formFrame);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    QPushButton *btnOk = new QPushButton(QStringLiteral("💾 Save Expense (F9)"), &dlg);
    btnOk->setShortcut(QKeySequence(Qt::Key_F9));
    btnOk->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 4px 14px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #C8E6C9; }"
    ));

    QPushButton *btnCancel = new QPushButton(QStringLiteral("❌ Cancel (Esc)"), &dlg);
    btnCancel->setShortcut(QKeySequence(Qt::Key_Escape));
    btnCancel->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 4px 14px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; }"
    ));

    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);
    dlgLayout->addLayout(btnLayout);

    connect(btnOk, &QPushButton::clicked, &dlg, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        if (txtPayee->text().trimmed().isEmpty()) return;

        if (DatabaseManager::instance().recordExpense(
            comboCat->currentText(),
            txtPayee->text().trimmed(),
            spinAmt->value(),
            0.0,
            comboMode->currentText(),
            txtNotes->text().trimmed()
        )) {
            QMessageBox::information(this, QStringLiteral("Saved"), QStringLiteral("Expense of $%1 recorded.").arg(spinAmt->value(), 0, 'f', 2));
            reloadData();
        }
    }
}

void FinancePage::onExportCsvClicked() {
    if (m_tableExpenses->rowCount() == 0) {
        QMessageBox::information(this, QStringLiteral("Export CSV"), QStringLiteral("No expense vouchers available to export."));
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(
        this, QStringLiteral("Export Expenses to CSV"), QStringLiteral("StoreExpenses.csv"),
        QStringLiteral("CSV Files (*.csv)")
    );
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, QStringLiteral("Export Failed"), QStringLiteral("Cannot open file for writing."));
        return;
    }

    QTextStream out(&file);
    QStringList headerLabels;
    for (int c = 0; c < m_tableExpenses->columnCount(); ++c) {
        headerLabels << QStringLiteral("\"%1\"").arg(m_tableExpenses->horizontalHeaderItem(c)->text());
    }
    out << headerLabels.join(QStringLiteral(",")) << "\n";

    for (int r = 0; r < m_tableExpenses->rowCount(); ++r) {
        QStringList rowValues;
        for (int c = 0; c < m_tableExpenses->columnCount(); ++c) {
            QTableWidgetItem *item = m_tableExpenses->item(r, c);
            QString text = item ? item->text() : QString();
            text.replace(QStringLiteral("\""), QStringLiteral("\"\""));
            rowValues << QStringLiteral("\"%1\"").arg(text);
        }
        out << rowValues.join(QStringLiteral(",")) << "\n";
    }

    file.close();
    QMessageBox::information(this, QStringLiteral("Export Completed"),
        QStringLiteral("Successfully exported %1 expense records to:\n%2")
        .arg(m_tableExpenses->rowCount())
        .arg(filePath));
}
