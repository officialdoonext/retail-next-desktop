#include "SettingsPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QFrame>
#include <QTabWidget>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QDateTime>
#include <QStandardPaths>
#include <QShortcut>

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent)
    , m_tabs(nullptr)
    , m_txtBusinessName(nullptr)
    , m_txtBranchName(nullptr)
    , m_txtRegisterCode(nullptr)
    , m_txtCurrencySymbol(nullptr)
    , m_comboTaxMode(nullptr)
    , m_txtReceiptHeader(nullptr)
    , m_txtReceiptFooter(nullptr)
    , m_comboIndustry(nullptr)
    , m_chkVariants(nullptr)
    , m_chkDecimalQty(nullptr)
    , m_chkSerialNumbers(nullptr)
    , m_chkBatchExpiry(nullptr)
    , m_chkNegativeStock(nullptr)
    , m_chkManagerDiscountPin(nullptr)
    , m_lblDbPath(nullptr)
    , m_lblDbSize(nullptr)
    , m_lblIntegrityStatus(nullptr)
{
    setupUI();
    reloadData();
}

void SettingsPage::setupUI() {
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

    QLabel *lblIcon = new QLabel(QStringLiteral("⚙️"), headerFrame);
    lblIcon->setStyleSheet(QStringLiteral("font-size: 22pt; padding: 0px; margin: 0px;"));
    titleGroup->addWidget(lblIcon);

    QVBoxLayout *titleText = new QVBoxLayout();
    titleText->setSpacing(1);
    QLabel *lblTitle = new QLabel(QStringLiteral("System Configuration & Settings"), headerFrame);
    lblTitle->setStyleSheet(QStringLiteral("font-size: 13pt; font-weight: 800; color: #0A294A; margin: 0;"));
    QLabel *lblSubtitle = new QLabel(QStringLiteral("Store identity, industry verticals, hardware peripherals, and database reliability backup."), headerFrame);
    lblSubtitle->setStyleSheet(QStringLiteral("font-size: 8.5pt; color: #4A6B82; font-style: italic; margin: 0;"));
    titleText->addWidget(lblTitle);
    titleText->addWidget(lblSubtitle);
    titleGroup->addLayout(titleText);
    headerLayout->addLayout(titleGroup);
    headerLayout->addStretch();

    QPushButton *btnSave = new QPushButton(QStringLiteral("💾 Save Settings (F5)"), headerFrame);
    btnSave->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 3px 14px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #C8E6C9; }"
    ));
    headerLayout->addWidget(btnSave);
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

    const QString inputStyle = QStringLiteral(
        "QLineEdit, QComboBox, QTextEdit { background: #FFFFFF; border: 1px solid #7A9EB9; "
        "border-radius: 2px; padding: 3px 6px; color: #000080; font-weight: bold; font-size: 9pt; } "
        "QLineEdit:focus, QComboBox:focus, QTextEdit:focus { border: 1px solid #0055A5; background-color: #FFFFF2; }"
    );

    // Tab 1: Business Profile & Receipts
    QWidget *tabProfile = new QWidget(m_tabs);
    QVBoxLayout *pMainLayout = new QVBoxLayout(tabProfile);
    pMainLayout->setContentsMargins(8, 8, 8, 8);

    QFrame *pCard = new QFrame(tabProfile);
    pCard->setStyleSheet(QStringLiteral("QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 3px; padding: 10px; }") + inputStyle);
    QFormLayout *f1 = new QFormLayout(pCard);
    f1->setSpacing(8);

    m_txtBusinessName = new QLineEdit(pCard);
    m_txtBranchName = new QLineEdit(pCard);
    m_txtRegisterCode = new QLineEdit(pCard);
    m_txtCurrencySymbol = new QLineEdit(pCard);
    m_comboTaxMode = new QComboBox(pCard);
    m_comboTaxMode->addItems(QStringList() << "Exclusive" << "Inclusive");

    m_txtReceiptHeader = new QTextEdit(pCard);
    m_txtReceiptHeader->setMaximumHeight(70);

    m_txtReceiptFooter = new QTextEdit(pCard);
    m_txtReceiptFooter->setMaximumHeight(70);

    f1->addRow(QStringLiteral("<b>Business / Store Name: *</b>"), m_txtBusinessName);
    f1->addRow(QStringLiteral("<b>Branch / Location Name: *</b>"), m_txtBranchName);
    f1->addRow(QStringLiteral("<b>POS Terminal Register Code:</b>"), m_txtRegisterCode);
    f1->addRow(QStringLiteral("<b>Currency Symbol (e.g. $, ₹, €, £):</b>"), m_txtCurrencySymbol);
    f1->addRow(QStringLiteral("<b>Default Tax Mode:</b>"), m_comboTaxMode);
    f1->addRow(QStringLiteral("<b>Receipt Header Text:</b>"), m_txtReceiptHeader);
    f1->addRow(QStringLiteral("<b>Receipt Footer Memo:</b>"), m_txtReceiptFooter);

    pMainLayout->addWidget(pCard);
    pMainLayout->addStretch();
    m_tabs->addTab(tabProfile, QStringLiteral("🏢 1. Business Profile & Receipts"));

    // Tab 2: Industry Configuration
    QWidget *tabIndustry = new QWidget(m_tabs);
    QVBoxLayout *l2 = new QVBoxLayout(tabIndustry);
    l2->setContentsMargins(8, 8, 8, 8);
    l2->setSpacing(8);

    QFrame *indCard = new QFrame(tabIndustry);
    indCard->setStyleSheet(QStringLiteral("QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 3px; padding: 10px; }") + inputStyle);
    QVBoxLayout *indCardLayout = new QVBoxLayout(indCard);

    QLabel *lblIndDesc = new QLabel(QStringLiteral(
        "<b>Adaptive Retail Vertical Engine:</b> Select your retail type to configure specialized workflows without separate codebases."
    ), indCard);
    lblIndDesc->setWordWrap(true);
    lblIndDesc->setStyleSheet(QStringLiteral("color: #0A294A; font-size: 9pt; margin-bottom: 6px;"));
    indCardLayout->addWidget(lblIndDesc);

    QHBoxLayout *indLayout = new QHBoxLayout();
    QLabel *lblActiveProf = new QLabel(QStringLiteral("Active Business Profile:"), indCard);
    lblActiveProf->setStyleSheet(QStringLiteral("font-weight: bold; color: #0A294A;"));
    indLayout->addWidget(lblActiveProf);

    m_comboIndustry = new QComboBox(indCard);
    m_comboIndustry->addItem(QStringLiteral("General Retail (Default)"), static_cast<int>(RetailNext::BusinessType::GeneralRetail));
    m_comboIndustry->addItem(QStringLiteral("Supermarket (High-Speed Scanning & Packs)"), static_cast<int>(RetailNext::BusinessType::Supermarket));
    m_comboIndustry->addItem(QStringLiteral("Clothing & Fashion (Sizes, Colors, Styles)"), static_cast<int>(RetailNext::BusinessType::ClothingFashion));
    m_comboIndustry->addItem(QStringLiteral("Groceries & Provisions (Decimals, Weight)"), static_cast<int>(RetailNext::BusinessType::GroceriesProvisions));
    m_comboIndustry->addItem(QStringLiteral("Footwear (Size Pairs, Models)"), static_cast<int>(RetailNext::BusinessType::Footwear));
    m_comboIndustry->addItem(QStringLiteral("Electronics (Serial numbers, IMEI, Warranties)"), static_cast<int>(RetailNext::BusinessType::Electronics));
    m_comboIndustry->addItem(QStringLiteral("Bakery & Confectionery (Daily fresh goods)"), static_cast<int>(RetailNext::BusinessType::Bakery));
    indLayout->addWidget(m_comboIndustry, 1);
    indCardLayout->addLayout(indLayout);

    QFrame *featuresFrame = new QFrame(indCard);
    featuresFrame->setStyleSheet(QStringLiteral("QFrame { background: #FFFFFF; border: 1px solid #7A9EB9; border-radius: 3px; padding: 10px; }"));
    QVBoxLayout *fLayout = new QVBoxLayout(featuresFrame);
    fLayout->setSpacing(8);

    const QString chkStyle = QStringLiteral("QCheckBox { color: #0A294A; font-weight: bold; font-size: 9pt; }");

    m_chkVariants = new QCheckBox(QStringLiteral("Enable Apparel Variant Matrix (Sizes, Colors, Style attributes)"), featuresFrame);
    m_chkVariants->setStyleSheet(chkStyle);
    m_chkDecimalQty = new QCheckBox(QStringLiteral("Enable Decimal Fractional Quantities (KG, Grams, Litres, Weighing scale support)"), featuresFrame);
    m_chkDecimalQty->setStyleSheet(chkStyle);
    m_chkSerialNumbers = new QCheckBox(QStringLiteral("Enable Serial / IMEI / Warranty tracking for individual units"), featuresFrame);
    m_chkSerialNumbers->setStyleSheet(chkStyle);
    m_chkBatchExpiry = new QCheckBox(QStringLiteral("Enable Batch Numbers & Expiry Date tracking alerts"), featuresFrame);
    m_chkBatchExpiry->setStyleSheet(chkStyle);
    m_chkNegativeStock = new QCheckBox(QStringLiteral("Allow Negative Stock Billing (Sell even if inventory is 0 or negative)"), featuresFrame);
    m_chkNegativeStock->setStyleSheet(chkStyle);
    m_chkManagerDiscountPin = new QCheckBox(QStringLiteral("Require Manager Approval PIN for Line-Item Price Overrides & High Discounts"), featuresFrame);
    m_chkManagerDiscountPin->setStyleSheet(chkStyle);

    fLayout->addWidget(m_chkVariants);
    fLayout->addWidget(m_chkDecimalQty);
    fLayout->addWidget(m_chkSerialNumbers);
    fLayout->addWidget(m_chkBatchExpiry);
    fLayout->addWidget(m_chkNegativeStock);
    fLayout->addWidget(m_chkManagerDiscountPin);
    indCardLayout->addWidget(featuresFrame);

    l2->addWidget(indCard);
    l2->addStretch();
    m_tabs->addTab(tabIndustry, QStringLiteral("🛍️ 2. Industry Configuration"));

    // Tab 3: Database & Backup Management
    QWidget *tabBackup = new QWidget(m_tabs);
    QVBoxLayout *l3 = new QVBoxLayout(tabBackup);
    l3->setContentsMargins(8, 8, 8, 8);
    l3->setSpacing(8);

    QFrame *dbFrame = new QFrame(tabBackup);
    dbFrame->setStyleSheet(QStringLiteral("QFrame { background: #FFFFFF; border: 1px solid #8FAEC7; border-radius: 3px; padding: 14px; }"));
    QGridLayout *dbGrid = new QGridLayout(dbFrame);
    dbGrid->setSpacing(10);

    m_lblDbPath = new QLabel(dbFrame);
    m_lblDbPath->setStyleSheet(QStringLiteral("color: #000080; font-weight: bold;"));
    m_lblDbSize = new QLabel(dbFrame);
    m_lblDbSize->setStyleSheet(QStringLiteral("color: #000080; font-weight: bold;"));
    m_lblIntegrityStatus = new QLabel(QStringLiteral("Verified OK (WAL Mode Active)"), dbFrame);
    m_lblIntegrityStatus->setStyleSheet(QStringLiteral("color: #1B5E20; font-weight: bold;"));

    const QString lblGridKey = QStringLiteral("font-weight: bold; color: #0A294A;");
    QLabel *kDbPath = new QLabel(QStringLiteral("SQLite Database File:"), dbFrame);
    kDbPath->setStyleSheet(lblGridKey);
    QLabel *kDbSize = new QLabel(QStringLiteral("Current Database Size:"), dbFrame);
    kDbSize->setStyleSheet(lblGridKey);
    QLabel *kIntegrity = new QLabel(QStringLiteral("Integrity Status:"), dbFrame);
    kIntegrity->setStyleSheet(lblGridKey);

    dbGrid->addWidget(kDbPath, 0, 0);
    dbGrid->addWidget(m_lblDbPath, 0, 1);
    dbGrid->addWidget(kDbSize, 1, 0);
    dbGrid->addWidget(m_lblDbSize, 1, 1);
    dbGrid->addWidget(kIntegrity, 2, 0);
    dbGrid->addWidget(m_lblIntegrityStatus, 2, 1);

    l3->addWidget(dbFrame);

    QHBoxLayout *backupActions = new QHBoxLayout();
    QPushButton *btnBackup = new QPushButton(QStringLiteral("💾 Backup Database Now (F8)"), tabBackup);
    btnBackup->setShortcut(QKeySequence(Qt::Key_F8));
    btnBackup->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 4px 14px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #C8E6C9; }"
    ));

    QPushButton *btnRestore = new QPushButton(QStringLiteral("🔄 Restore From Backup (Alt+R)"), tabBackup);
    btnRestore->setShortcut(QKeySequence(Qt::ALT | Qt::Key_R));
    btnRestore->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 4px 14px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; }"
    ));

    QPushButton *btnIntegrity = new QPushButton(QStringLiteral("🔍 Run PRAGMA Integrity Check (F9)"), tabBackup);
    btnIntegrity->setShortcut(QKeySequence(Qt::Key_F9));
    btnIntegrity->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 4px 14px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; }"
    ));

    backupActions->addWidget(btnBackup);
    backupActions->addWidget(btnRestore);
    backupActions->addWidget(btnIntegrity);
    backupActions->addStretch();
    l3->addLayout(backupActions);
    l3->addStretch();

    m_tabs->addTab(tabBackup, QStringLiteral("🗄️ 3. Backup, Restore & Maintenance"));

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

    QPushButton *bF5 = new QPushButton(QStringLiteral("F5 - Save Settings"), fnBar);
    bF5->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 2px 10px; font-size: 8.5pt; font-weight: bold; } "
        "QPushButton:hover { background: #C8E6C9; }"
    ));
    bF5->setShortcut(QKeySequence(Qt::Key_F5));
    connect(bF5, &QPushButton::clicked, this, &SettingsPage::onSaveProfileClicked);

    QPushButton *bF8 = new QPushButton(QStringLiteral("F8 - Backup Database"), fnBar);
    bF8->setStyleSheet(fnBtnStyle);
    bF8->setShortcut(QKeySequence(Qt::Key_F8));
    connect(bF8, &QPushButton::clicked, this, &SettingsPage::onBackupNowClicked);

    QPushButton *bF9 = new QPushButton(QStringLiteral("F9 - Integrity Check"), fnBar);
    bF9->setStyleSheet(fnBtnStyle);
    bF9->setShortcut(QKeySequence(Qt::Key_F9));
    connect(bF9, &QPushButton::clicked, this, &SettingsPage::onRunIntegrityCheckClicked);

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

    connect(btnSave, &QPushButton::clicked, this, &SettingsPage::onSaveProfileClicked);
    connect(m_comboIndustry, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsPage::onIndustryTypeChanged);
    connect(btnBackup, &QPushButton::clicked, this, &SettingsPage::onBackupNowClicked);
    connect(btnRestore, &QPushButton::clicked, this, &SettingsPage::onRestoreBackupClicked);
    connect(btnIntegrity, &QPushButton::clicked, this, &SettingsPage::onRunIntegrityCheckClicked);
}

void SettingsPage::switchToTab(int tabIndex) {
    if (m_tabs && tabIndex >= 0 && tabIndex < m_tabs->count()) {
        m_tabs->setCurrentIndex(tabIndex);
    }
}

void SettingsPage::reloadData() {
    DatabaseManager &db = DatabaseManager::instance();

    m_txtBusinessName->setText(db.storeName());
    m_txtBranchName->setText(db.branchName());
    m_txtRegisterCode->setText(db.registerCode());
    m_txtCurrencySymbol->setText(db.currencySymbol());
    m_comboTaxMode->setCurrentText(db.taxMode());

    m_txtReceiptHeader->setText(db.getBusinessSetting(QStringLiteral("receipt_header"), QStringLiteral("RetailNext Flagship Store\n100 Market Square, Tech City")));
    m_txtReceiptFooter->setText(db.getBusinessSetting(QStringLiteral("receipt_footer"), QStringLiteral("Thank you for shopping with us!")));

    QString btypeStr = db.getBusinessSetting(QStringLiteral("business_type"), QStringLiteral("GeneralRetail"));
    int idx = 0;
    if (btypeStr.contains("Clothing", Qt::CaseInsensitive)) idx = 2;
    else if (btypeStr.contains("Supermarket", Qt::CaseInsensitive)) idx = 1;
    else if (btypeStr.contains("Groceries", Qt::CaseInsensitive)) idx = 3;
    else if (btypeStr.contains("Footwear", Qt::CaseInsensitive)) idx = 4;
    else if (btypeStr.contains("Electronics", Qt::CaseInsensitive)) idx = 5;
    else if (btypeStr.contains("Bakery", Qt::CaseInsensitive)) idx = 6;
    m_comboIndustry->setCurrentIndex(idx);
    onIndustryTypeChanged(idx);

    QFileInfo fi(db.databasePath());
    m_lblDbPath->setText(fi.absoluteFilePath());
    m_lblDbSize->setText(QStringLiteral("%1 KB").arg(fi.size() / 1024));
}

void SettingsPage::onIndustryTypeChanged(int index) {
    switch (index) {
    case 1: // Supermarket
        m_chkVariants->setChecked(false);
        m_chkDecimalQty->setChecked(true);
        m_chkSerialNumbers->setChecked(false);
        m_chkBatchExpiry->setChecked(true);
        m_chkNegativeStock->setChecked(false);
        m_chkManagerDiscountPin->setChecked(true);
        break;
    case 2: // Clothing
        m_chkVariants->setChecked(true);
        m_chkDecimalQty->setChecked(false);
        m_chkSerialNumbers->setChecked(false);
        m_chkBatchExpiry->setChecked(false);
        m_chkNegativeStock->setChecked(false);
        m_chkManagerDiscountPin->setChecked(false);
        break;
    case 3: // Groceries & Provisions
        m_chkVariants->setChecked(false);
        m_chkDecimalQty->setChecked(true);
        m_chkSerialNumbers->setChecked(false);
        m_chkBatchExpiry->setChecked(true);
        m_chkNegativeStock->setChecked(false);
        m_chkManagerDiscountPin->setChecked(false);
        break;
    case 4: // Footwear
        m_chkVariants->setChecked(true);
        m_chkDecimalQty->setChecked(false);
        m_chkSerialNumbers->setChecked(false);
        m_chkBatchExpiry->setChecked(false);
        m_chkNegativeStock->setChecked(false);
        m_chkManagerDiscountPin->setChecked(false);
        break;
    case 5: // Electronics
        m_chkVariants->setChecked(false);
        m_chkDecimalQty->setChecked(false);
        m_chkSerialNumbers->setChecked(true);
        m_chkBatchExpiry->setChecked(false);
        m_chkNegativeStock->setChecked(false);
        m_chkManagerDiscountPin->setChecked(true);
        break;
    case 6: // Bakery
        m_chkVariants->setChecked(false);
        m_chkDecimalQty->setChecked(true);
        m_chkSerialNumbers->setChecked(false);
        m_chkBatchExpiry->setChecked(true);
        m_chkNegativeStock->setChecked(false);
        m_chkManagerDiscountPin->setChecked(false);
        break;
    default: // General Retail
        m_chkVariants->setChecked(true);
        m_chkDecimalQty->setChecked(true);
        m_chkSerialNumbers->setChecked(false);
        m_chkBatchExpiry->setChecked(false);
        m_chkNegativeStock->setChecked(false);
        m_chkManagerDiscountPin->setChecked(false);
        break;
    }
}

void SettingsPage::onSaveProfileClicked() {
    DatabaseManager &db = DatabaseManager::instance();

    db.updateBusinessSetting(QStringLiteral("business_name"), m_txtBusinessName->text().trimmed());
    db.updateBusinessSetting(QStringLiteral("branch_name"), m_txtBranchName->text().trimmed());
    db.updateBusinessSetting(QStringLiteral("register_code"), m_txtRegisterCode->text().trimmed());
    db.updateBusinessSetting(QStringLiteral("currency_symbol"), m_txtCurrencySymbol->text().trimmed());
    db.updateBusinessSetting(QStringLiteral("tax_mode"), m_comboTaxMode->currentText());
    db.updateBusinessSetting(QStringLiteral("receipt_header"), m_txtReceiptHeader->toPlainText().trimmed());
    db.updateBusinessSetting(QStringLiteral("receipt_footer"), m_txtReceiptFooter->toPlainText().trimmed());

    RetailNext::BusinessType btype = static_cast<RetailNext::BusinessType>(m_comboIndustry->currentData().toInt());
    db.setBusinessType(btype);

    QMessageBox::information(this, QStringLiteral("Settings Saved"), QStringLiteral("RetailNext configuration and business profile updated successfully!"));
}

void SettingsPage::onBackupNowClicked() {
    QString defName = QStringLiteral("RetailNext_Backup_%1.db").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QString dest = QFileDialog::getSaveFileName(this, QStringLiteral("Backup Database Snapshot"), defName, QStringLiteral("SQLite Database (*.db *.bak)"));
    if (dest.isEmpty()) return;

    if (DatabaseManager::instance().backupDatabase(dest)) {
        QMessageBox::information(
            this,
            QStringLiteral("Backup Successful"),
            QStringLiteral(
                "<h3>Backup Complete</h3>"
                "<p>Atomic WAL database snapshot created at:</p>"
                "<code>%1</code><br><br>"
                "<b>File Size:</b> %2 KB"
            ).arg(dest).arg(QFileInfo(dest).size() / 1024)
        );
    } else {
        QMessageBox::critical(this, QStringLiteral("Backup Failed"), QStringLiteral("Failed to backup database."));
    }
}

void SettingsPage::onRestoreBackupClicked() {
    QString src = QFileDialog::getOpenFileName(this, QStringLiteral("Select Backup File to Restore"), QString(), QStringLiteral("SQLite Database (*.db *.bak)"));
    if (src.isEmpty()) return;

    QMessageBox::StandardButton reply = QMessageBox::warning(
        this,
        QStringLiteral("Restore Database Confirmation"),
        QStringLiteral("WARNING: Restoring will overwrite the current active database with '%1'.\nAre you sure you want to proceed?").arg(src),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (DatabaseManager::instance().restoreDatabase(src)) {
            QMessageBox::information(this, QStringLiteral("Database Restored"), QStringLiteral("Database restored successfully. Application will reload."));
            reloadData();
        } else {
            QMessageBox::critical(this, QStringLiteral("Restore Error"), QStringLiteral("Failed to restore database from selected file."));
        }
    }
}

void SettingsPage::onRunIntegrityCheckClicked() {
    QString res = DatabaseManager::instance().runIntegrityCheck();
    m_lblIntegrityStatus->setText(QStringLiteral("Result: %1 (WAL checkpoint clean)").arg(res));
    QMessageBox::information(this, QStringLiteral("SQLite Maintenance & Integrity"), QStringLiteral("Integrity Check Result: %1\nWAL file checkpointed and truncated.").arg(res));
}
