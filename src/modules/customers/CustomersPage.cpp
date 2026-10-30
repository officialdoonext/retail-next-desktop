#include "CustomersPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QDateTime>
#include <QFileDialog>
#include <QTextStream>
#include <QShortcut>

CustomersPage::CustomersPage(QWidget *parent)
    : QWidget(parent)
    , m_txtSearch(nullptr)
    , m_comboGroup(nullptr)
    , m_tableCustomers(nullptr)
    , m_lblSummary(nullptr)
{
    setupUI();
    reloadData();
}

void CustomersPage::setupUI() {
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

    QLabel *lblIcon = new QLabel(QStringLiteral("👥"), headerFrame);
    lblIcon->setStyleSheet(QStringLiteral("font-size: 22pt; padding: 0px; margin: 0px;"));
    titleGroup->addWidget(lblIcon);

    QVBoxLayout *titleText = new QVBoxLayout();
    titleText->setSpacing(1);
    QLabel *lblTitle = new QLabel(QStringLiteral("Customer Registry & CRM Ledger"), headerFrame);
    lblTitle->setStyleSheet(QStringLiteral("font-size: 13pt; font-weight: 800; color: #0A294A; margin: 0;"));
    QLabel *lblSubtitle = new QLabel(QStringLiteral("Manage customer accounts, loyalty reward points, store credit, and receivables ledger."), headerFrame);
    lblSubtitle->setStyleSheet(QStringLiteral("font-size: 8.5pt; color: #4A6B82; font-style: italic; margin: 0;"));
    titleText->addWidget(lblTitle);
    titleText->addWidget(lblSubtitle);
    titleGroup->addLayout(titleText);
    headerLayout->addLayout(titleGroup);
    headerLayout->addStretch();

    // Filters inside header
    QLabel *lblSearch = new QLabel(QStringLiteral("Search:"), headerFrame);
    lblSearch->setStyleSheet(QStringLiteral("font-weight: bold; color: #0A294A; font-size: 8.5pt;"));
    m_txtSearch = new QLineEdit(headerFrame);
    m_txtSearch->setPlaceholderText(QStringLiteral("Name, phone or code..."));
    m_txtSearch->setMinimumWidth(180);
    m_txtSearch->setStyleSheet(QStringLiteral(
        "QLineEdit { background: #FFFFFF; border: 1px solid #7A9EB9; border-radius: 2px; "
        "padding: 2px 6px; color: #000080; font-weight: bold; font-size: 9pt; } "
        "QLineEdit:focus { border: 1px solid #0055A5; background-color: #FFFFF2; }"
    ));

    QLabel *lblGrp = new QLabel(QStringLiteral("Group:"), headerFrame);
    lblGrp->setStyleSheet(QStringLiteral("font-weight: bold; color: #0A294A; font-size: 8.5pt;"));
    m_comboGroup = new QComboBox(headerFrame);
    m_comboGroup->addItems(QStringList() << QStringLiteral("All Groups") << QStringLiteral("Retail") << QStringLiteral("VIP") << QStringLiteral("Wholesale"));
    m_comboGroup->setStyleSheet(QStringLiteral(
        "QComboBox { background: #FFFFFF; border: 1px solid #7A9EB9; border-radius: 2px; "
        "padding: 2px 6px; color: #000080; font-weight: bold; font-size: 9pt; }"
    ));

    const QString erpBtnStyle = QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; border-color: #4A82AD; }"
    );

    QPushButton *btnAdd = new QPushButton(QStringLiteral("➕ New Customer (F3)"), headerFrame);
    btnAdd->setShortcut(QKeySequence(Qt::Key_F3));
    btnAdd->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 3px 12px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #C8E6C9; }"
    ));

    QPushButton *btnPayment = new QPushButton(QStringLiteral("💵 Record Payment (F8)"), headerFrame);
    btnPayment->setShortcut(QKeySequence(Qt::Key_F8));
    btnPayment->setStyleSheet(erpBtnStyle);

    QPushButton *btnRefresh = new QPushButton(QStringLiteral("🔄 Refresh (F5)"), headerFrame);
    btnRefresh->setShortcut(QKeySequence(Qt::Key_F5));
    btnRefresh->setStyleSheet(erpBtnStyle);

    headerLayout->addWidget(lblSearch);
    headerLayout->addWidget(m_txtSearch);
    headerLayout->addWidget(lblGrp);
    headerLayout->addWidget(m_comboGroup);
    headerLayout->addWidget(btnAdd);
    headerLayout->addWidget(btnPayment);
    headerLayout->addWidget(btnRefresh);
    mainLayout->addWidget(headerFrame);

    // ==========================================
    // Table View
    // ==========================================
    m_tableCustomers = new QTableWidget(this);
    m_tableCustomers->setColumnCount(9);
    m_tableCustomers->setHorizontalHeaderLabels(QStringList()
        << QStringLiteral("Code")
        << QStringLiteral("Customer Name")
        << QStringLiteral("Phone")
        << QStringLiteral("Email")
        << QStringLiteral("Group")
        << QStringLiteral("Loyalty Points")
        << QStringLiteral("Store Credit")
        << QStringLiteral("Outstanding Balance")
        << QStringLiteral("Credit Limit"));
    m_tableCustomers->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableCustomers->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableCustomers->verticalHeader()->setVisible(false);
    m_tableCustomers->verticalHeader()->setDefaultSectionSize(22);
    m_tableCustomers->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableCustomers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableCustomers->setAlternatingRowColors(true);
    m_tableCustomers->setStyleSheet(QStringLiteral(
        "QTableWidget { background-color: #FFFFFF; alternate-background-color: #F8FBFE; "
        "gridline-color: #C2D7EA; border: 1px solid #7A9EB9; font-size: 8.5pt; color: #0A294A; } "
        "QTableWidget::item { padding: 2px 4px; } "
        "QTableWidget::item:selected { background-color: #2D82D6; color: #FFFFFF; font-weight: bold; } "
        "QHeaderView::section { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B3D4F0, stop:1 #8DBDE0); "
        "color: #0A294A; font-weight: bold; font-size: 8pt; border: none; border-top: 1px solid #C2D7EA; "
        "border-bottom: 1px solid #6895BA; border-right: 1px solid #99BEDC; padding: 3px 5px; }"
    ));
    mainLayout->addWidget(m_tableCustomers, 1);

    // ==========================================
    // Footer Summary
    // ==========================================
    QFrame *footerFrame = new QFrame(this);
    footerFrame->setFixedHeight(28);
    footerFrame->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; }"
    ));
    QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(6, 1, 6, 1);
    m_lblSummary = new QLabel(footerFrame);
    m_lblSummary->setStyleSheet(QStringLiteral("font-weight: bold; color: #000080; font-size: 8.5pt;"));
    footerLayout->addWidget(m_lblSummary);
    footerLayout->addStretch();
    mainLayout->addWidget(footerFrame);

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

    QPushButton *bF2 = new QPushButton(QStringLiteral("F2 - Search"), fnBar);
    bF2->setStyleSheet(fnBtnStyle);
    bF2->setShortcut(QKeySequence(Qt::Key_F2));
    connect(bF2, &QPushButton::clicked, this, [this]() {
        if (m_txtSearch) {
            m_txtSearch->setFocus();
            m_txtSearch->selectAll();
        }
    });

    QPushButton *bF3 = new QPushButton(QStringLiteral("F3 - New Customer"), fnBar);
    bF3->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 2px 10px; font-size: 8.5pt; font-weight: bold; } "
        "QPushButton:hover { background: #C8E6C9; }"
    ));
    bF3->setShortcut(QKeySequence(Qt::Key_F3));
    connect(bF3, &QPushButton::clicked, this, &CustomersPage::onAddCustomerClicked);

    QPushButton *bF4 = new QPushButton(QStringLiteral("F4 - Edit Customer"), fnBar);
    bF4->setStyleSheet(fnBtnStyle);
    bF4->setShortcut(QKeySequence(Qt::Key_F4));
    connect(bF4, &QPushButton::clicked, this, &CustomersPage::onEditCustomerClicked);

    QPushButton *bF8 = new QPushButton(QStringLiteral("F8 - Record Payment"), fnBar);
    bF8->setStyleSheet(fnBtnStyle);
    bF8->setShortcut(QKeySequence(Qt::Key_F8));
    connect(bF8, &QPushButton::clicked, this, &CustomersPage::onRecordPaymentClicked);

    QPushButton *bF9 = new QPushButton(QStringLiteral("F9 - Export CSV"), fnBar);
    bF9->setStyleSheet(fnBtnStyle);
    bF9->setShortcut(QKeySequence(Qt::Key_F9));
    connect(bF9, &QPushButton::clicked, this, &CustomersPage::onExportCsvClicked);

    QPushButton *bEsc = new QPushButton(QStringLiteral("Esc - Cancel"), fnBar);
    bEsc->setStyleSheet(fnBtnStyle);
    bEsc->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(bEsc, &QPushButton::clicked, this, [this]() {
        if (m_txtSearch) m_txtSearch->clear();
    });

    fnLayout->addWidget(bF1);
    fnLayout->addWidget(bF2);
    fnLayout->addWidget(bF3);
    fnLayout->addWidget(bF4);
    fnLayout->addWidget(bF8);
    fnLayout->addWidget(bF9);
    fnLayout->addStretch();
    fnLayout->addWidget(bEsc);

    mainLayout->addWidget(fnBar);

    connect(m_txtSearch, &QLineEdit::textChanged, this, &CustomersPage::onSearchChanged);
    connect(m_comboGroup, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CustomersPage::onSearchChanged);
    connect(btnAdd, &QPushButton::clicked, this, &CustomersPage::onAddCustomerClicked);
    connect(btnPayment, &QPushButton::clicked, this, &CustomersPage::onRecordPaymentClicked);
    connect(btnRefresh, &QPushButton::clicked, this, &CustomersPage::reloadData);
    connect(m_tableCustomers, &QTableWidget::cellDoubleClicked, this, &CustomersPage::onEditCustomerClicked);
}

void CustomersPage::reloadData() {
    onSearchChanged();
}

void CustomersPage::onSearchChanged() {
    QString q = m_txtSearch ? m_txtSearch->text().trimmed() : QString();
    QString grp = m_comboGroup ? m_comboGroup->currentText() : QStringLiteral("All Groups");

    m_currentCustomers = DatabaseManager::instance().searchCustomers(q, grp);
    m_tableCustomers->setRowCount(m_currentCustomers.size());

    double totalReceivables = 0.0;
    QString cur = DatabaseManager::instance().currencySymbol();

    for (int r = 0; r < m_currentCustomers.size(); ++r) {
        const auto& c = m_currentCustomers[r];
        totalReceivables += c.outstandingBalance;

        QTableWidgetItem *codeItem = new QTableWidgetItem(c.code);
        codeItem->setForeground(QBrush(QColor("#000080")));
        codeItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_tableCustomers->setItem(r, 0, codeItem);

        m_tableCustomers->setItem(r, 1, new QTableWidgetItem(c.name));
        m_tableCustomers->setItem(r, 2, new QTableWidgetItem(c.phone));
        m_tableCustomers->setItem(r, 3, new QTableWidgetItem(c.email));
        m_tableCustomers->setItem(r, 4, new QTableWidgetItem(c.customerGroup));

        QTableWidgetItem *ptsItem = new QTableWidgetItem(QString::number(c.loyaltyPoints));
        ptsItem->setTextAlignment(Qt::AlignCenter);
        ptsItem->setForeground(QBrush(QColor("#E65100")));
        ptsItem->setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::Bold));
        m_tableCustomers->setItem(r, 5, ptsItem);

        QTableWidgetItem *scItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(c.storeCredit, 0, 'f', 2));
        scItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_tableCustomers->setItem(r, 6, scItem);

        QTableWidgetItem *balItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(c.outstandingBalance, 0, 'f', 2));
        balItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (c.outstandingBalance > 0.0) {
            balItem->setForeground(QBrush(QColor("#B71C1C")));
            balItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        }
        m_tableCustomers->setItem(r, 7, balItem);

        QTableWidgetItem *climItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(c.creditLimit, 0, 'f', 2));
        climItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_tableCustomers->setItem(r, 8, climItem);
    }

    m_lblSummary->setText(QStringLiteral("Customers Registered: %1 | Outstanding Accounts Receivable: %2%3")
        .arg(m_currentCustomers.size())
        .arg(cur)
        .arg(totalReceivables, 0, 'f', 2));
}

void CustomersPage::onAddCustomerClicked() {
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Register New Customer"));
    dlg.resize(440, 360);
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

    QLineEdit *txtName = new QLineEdit(formFrame);
    QLineEdit *txtPhone = new QLineEdit(formFrame);
    QLineEdit *txtEmail = new QLineEdit(formFrame);
    QLineEdit *txtAddress = new QLineEdit(formFrame);
    QComboBox *comboGrp = new QComboBox(formFrame);
    comboGrp->addItems(QStringList() << "Retail" << "VIP" << "Wholesale");
    QDoubleSpinBox *spinLimit = new QDoubleSpinBox(formFrame);
    spinLimit->setRange(0.0, 50000.0);
    spinLimit->setValue(500.0);

    form->addRow(QStringLiteral("Customer Full Name: *"), txtName);
    form->addRow(QStringLiteral("Mobile / Phone: *"), txtPhone);
    form->addRow(QStringLiteral("Email:"), txtEmail);
    form->addRow(QStringLiteral("Address:"), txtAddress);
    form->addRow(QStringLiteral("Customer Group:"), comboGrp);
    form->addRow(QStringLiteral("Credit Limit:"), spinLimit);

    dlgLayout->addWidget(formFrame);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    QPushButton *btnOk = new QPushButton(QStringLiteral("💾 Save Customer (F9)"), &dlg);
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
        if (txtName->text().trimmed().isEmpty()) return;

        RetailNext::CustomerRecord c;
        c.code = QStringLiteral("CUST-%1").arg(QDateTime::currentMSecsSinceEpoch() % 10000);
        c.name = txtName->text().trimmed();
        c.phone = txtPhone->text().trimmed();
        c.email = txtEmail->text().trimmed();
        c.address = txtAddress->text().trimmed();
        c.customerGroup = comboGrp->currentText();
        c.loyaltyPoints = 10;
        c.storeCredit = 0.0;
        c.outstandingBalance = 0.0;
        c.creditLimit = spinLimit->value();
        c.isActive = true;

        if (DatabaseManager::instance().saveCustomer(c, true)) {
            QMessageBox::information(this, QStringLiteral("Customer Registered"), QStringLiteral("Customer '%1' added successfully.").arg(c.name));
            reloadData();
        }
    }
}

void CustomersPage::onEditCustomerClicked() {
    int row = m_tableCustomers->currentRow();
    if (row < 0 || row >= m_currentCustomers.size()) return;

    auto c = m_currentCustomers[row];

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Edit Customer - %1").arg(c.name));
    dlg.resize(440, 360);
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

    QLineEdit *txtName = new QLineEdit(c.name, formFrame);
    QLineEdit *txtPhone = new QLineEdit(c.phone, formFrame);
    QLineEdit *txtEmail = new QLineEdit(c.email, formFrame);
    QLineEdit *txtAddress = new QLineEdit(c.address, formFrame);
    QComboBox *comboGrp = new QComboBox(formFrame);
    comboGrp->addItems(QStringList() << "Retail" << "VIP" << "Wholesale");
    comboGrp->setCurrentText(c.customerGroup);
    QDoubleSpinBox *spinLimit = new QDoubleSpinBox(formFrame);
    spinLimit->setRange(0.0, 50000.0);
    spinLimit->setValue(c.creditLimit);

    form->addRow(QStringLiteral("Customer Name:"), txtName);
    form->addRow(QStringLiteral("Phone Number:"), txtPhone);
    form->addRow(QStringLiteral("Email:"), txtEmail);
    form->addRow(QStringLiteral("Address:"), txtAddress);
    form->addRow(QStringLiteral("Customer Group:"), comboGrp);
    form->addRow(QStringLiteral("Credit Limit:"), spinLimit);

    dlgLayout->addWidget(formFrame);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    QPushButton *btnOk = new QPushButton(QStringLiteral("Update Customer (F9)"), &dlg);
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
        c.name = txtName->text().trimmed();
        c.phone = txtPhone->text().trimmed();
        c.email = txtEmail->text().trimmed();
        c.address = txtAddress->text().trimmed();
        c.customerGroup = comboGrp->currentText();
        c.creditLimit = spinLimit->value();

        if (DatabaseManager::instance().saveCustomer(c, false)) {
            reloadData();
        }
    }
}

void CustomersPage::onRecordPaymentClicked() {
    auto custs = DatabaseManager::instance().searchCustomers();
    if (custs.isEmpty()) return;

    QStringList options;
    for (const auto& c : custs) {
        options.append(QStringLiteral("%1 (Balance: $%2)").arg(c.name).arg(c.outstandingBalance, 0, 'f', 2));
    }

    bool ok = false;
    QString sel = QInputDialog::getItem(this, QStringLiteral("Customer Payment Receipt"), QStringLiteral("Select customer paying credit balance:"), options, 0, false, &ok);
    if (!ok || sel.isEmpty()) return;

    int idx = options.indexOf(sel);
    auto cust = custs[idx];

    double amt = QInputDialog::getDouble(
        this,
        QStringLiteral("Payment Amount"),
        QStringLiteral("Enter payment received from '%1':").arg(cust.name),
        cust.outstandingBalance > 0 ? cust.outstandingBalance : 50.0,
        1.0,
        10000.0,
        2,
        &ok
    );
    if (!ok) return;

    QString mode = QInputDialog::getItem(
        this,
        QStringLiteral("Payment Mode"),
        QStringLiteral("Select tender method:"),
        QStringList() << QStringLiteral("Cash") << QStringLiteral("UPI") << QStringLiteral("Card") << QStringLiteral("Bank Transfer"),
        0,
        false,
        &ok
    );
    if (!ok) return;

    if (DatabaseManager::instance().recordCustomerPayment(cust.id, amt, mode, QStringLiteral("Credit balance receipt"))) {
        QMessageBox::information(this, QStringLiteral("Receipt Recorded"), QStringLiteral("Successfully credited $%1 against '%2' balance.").arg(amt, 0, 'f', 2).arg(cust.name));
        reloadData();
    }
}

void CustomersPage::onExportCsvClicked() {
    if (m_tableCustomers->rowCount() == 0) {
        QMessageBox::information(this, QStringLiteral("Export CSV"), QStringLiteral("No customer records available to export."));
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(
        this, QStringLiteral("Export Customers to CSV"), QStringLiteral("Customers.csv"),
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
    for (int c = 0; c < m_tableCustomers->columnCount(); ++c) {
        headerLabels << QStringLiteral("\"%1\"").arg(m_tableCustomers->horizontalHeaderItem(c)->text());
    }
    out << headerLabels.join(QStringLiteral(",")) << "\n";

    for (int r = 0; r < m_tableCustomers->rowCount(); ++r) {
        QStringList rowValues;
        for (int c = 0; c < m_tableCustomers->columnCount(); ++c) {
            QTableWidgetItem *item = m_tableCustomers->item(r, c);
            QString text = item ? item->text() : QString();
            text.replace(QStringLiteral("\""), QStringLiteral("\"\""));
            rowValues << QStringLiteral("\"%1\"").arg(text);
        }
        out << rowValues.join(QStringLiteral(",")) << "\n";
    }

    file.close();
    QMessageBox::information(this, QStringLiteral("Export Completed"),
        QStringLiteral("Successfully exported %1 customers to:\n%2")
        .arg(m_tableCustomers->rowCount())
        .arg(filePath));
}
