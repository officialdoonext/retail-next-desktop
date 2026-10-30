#include "AccessPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QShortcut>
#include <QKeySequence>
#include <QFont>
#include <QDebug>

// =========================================================================
// Dialog: Add / Edit Store
// =========================================================================
class StoreDialog : public QDialog {
public:
    StoreDialog(const RetailNext::StoreRecord &store, bool isNew, QWidget *parent = nullptr)
        : QDialog(parent), m_store(store), m_isNew(isNew) {
        setWindowTitle(isNew ? QStringLiteral("🏪 New Store Registration") 
                             : QStringLiteral("✏️ Edit Store - %1").arg(store.name));
        resize(480, 360);
        setStyleSheet(QStringLiteral(
            "QDialog { background-color: #D9E8F5; } "
            "QLabel { color: #0A294A; font-weight: bold; font-size: 8.5pt; } "
            "QLineEdit, QTextEdit { background: #FFFFFF; border: 1px solid #7A9EB9; "
            "border-radius: 2px; padding: 3px 6px; color: #000080; font-weight: bold; font-size: 9pt; } "
            "QLineEdit:focus, QTextEdit:focus { border: 1px solid #0055A5; background-color: #FFFFF2; } "
            "QCheckBox { color: #0A294A; font-weight: bold; font-size: 8.5pt; }"
        ));

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(14, 14, 14, 14);
        mainLayout->setSpacing(10);

        QFrame *formFrame = new QFrame(this);
        formFrame->setStyleSheet(QStringLiteral("QFrame { background-color: #FFFFFF; border: 1px solid #8FAEC7; border-radius: 3px; padding: 10px; }"));
        QFormLayout *form = new QFormLayout(formFrame);
        form->setLabelAlignment(Qt::AlignRight);
        form->setSpacing(8);

        m_txtName = new QLineEdit(store.name, formFrame);
        m_txtName->setPlaceholderText(QStringLiteral("Required (e.g. Flagship Superstore)"));

        m_txtPhone = new QLineEdit(store.phone, formFrame);
        m_txtPhone->setPlaceholderText(QStringLiteral("Required (e.g. +1 555-0199)"));

        m_txtEmail = new QLineEdit(store.email, formFrame);
        m_txtEmail->setPlaceholderText(QStringLiteral("Optional (e.g. store@company.com)"));

        m_txtCity = new QLineEdit(store.city, formFrame);
        m_txtCity->setPlaceholderText(QStringLiteral("City name"));

        m_txtAddress = new QLineEdit(store.address, formFrame);
        m_txtAddress->setPlaceholderText(QStringLiteral("Street, Suite, Landmark"));

        m_txtGstNo = new QLineEdit(store.gstNo, formFrame);
        m_txtGstNo->setPlaceholderText(QStringLiteral("GSTIN / Tax ID Number"));

        m_chkActive = new QCheckBox(QStringLiteral("Store is Active and Operable"), formFrame);
        m_chkActive->setChecked(store.isActive);

        form->addRow(QStringLiteral("Store Name *:"), m_txtName);
        form->addRow(QStringLiteral("Mobile Number *:"), m_txtPhone);
        form->addRow(QStringLiteral("Email (Optional):"), m_txtEmail);
        form->addRow(QStringLiteral("City:"), m_txtCity);
        form->addRow(QStringLiteral("Address:"), m_txtAddress);
        form->addRow(QStringLiteral("GST Number:"), m_txtGstNo);
        form->addRow(QStringLiteral("Status:"), m_chkActive);

        mainLayout->addWidget(formFrame);

        QHBoxLayout *btnLayout = new QHBoxLayout();
        btnLayout->addStretch();

        QPushButton *btnSave = new QPushButton(QStringLiteral("💾 Save Store (F9)"), this);
        btnSave->setShortcut(QKeySequence(Qt::Key_F9));
        btnSave->setStyleSheet(QStringLiteral(
            "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
            "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 5px 16px; font-weight: bold; font-size: 8.5pt; } "
            "QPushButton:hover { background: #C8E6C9; border-color: #388E3C; }"
        ));
        connect(btnSave, &QPushButton::clicked, this, &StoreDialog::onSaveClicked);

        QPushButton *btnCancel = new QPushButton(QStringLiteral("❌ Cancel (Esc)"), this);
        btnCancel->setShortcut(QKeySequence(Qt::Key_Escape));
        btnCancel->setStyleSheet(QStringLiteral(
            "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFEBEE, stop:1 #FFCDD2); "
            "color: #B71C1C; border: 1px solid #EF9A9A; border-radius: 2px; padding: 5px 14px; font-weight: bold; font-size: 8.5pt; } "
            "QPushButton:hover { background: #FFCDD2; border-color: #D32F2F; }"
        ));
        connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

        btnLayout->addWidget(btnSave);
        btnLayout->addWidget(btnCancel);
        mainLayout->addLayout(btnLayout);

        m_txtName->setFocus();
    }

    RetailNext::StoreRecord getStore() const { return m_store; }

private:
    void onSaveClicked() {
        QString name = m_txtName->text().trimmed();
        QString phone = m_txtPhone->text().trimmed();

        if (name.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("Validation Error"),
                                 QStringLiteral("Store Name is mandatory. Please enter a valid name."));
            m_txtName->setFocus();
            return;
        }

        if (phone.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("Validation Error"),
                                 QStringLiteral("Mobile Number is mandatory. Please enter a valid contact phone."));
            m_txtPhone->setFocus();
            return;
        }

        m_store.name = name;
        m_store.phone = phone;
        m_store.email = m_txtEmail->text().trimmed();
        m_store.city = m_txtCity->text().trimmed();
        m_store.address = m_txtAddress->text().trimmed();
        m_store.gstNo = m_txtGstNo->text().trimmed();
        m_store.isActive = m_chkActive->isChecked();

        accept();
    }

    RetailNext::StoreRecord m_store;
    bool m_isNew;
    QLineEdit *m_txtName;
    QLineEdit *m_txtPhone;
    QLineEdit *m_txtEmail;
    QLineEdit *m_txtCity;
    QLineEdit *m_txtAddress;
    QLineEdit *m_txtGstNo;
    QCheckBox *m_chkActive;
};

// =========================================================================
// Dialog: Add / Edit Warehouse
// =========================================================================
class WarehouseDialog : public QDialog {
public:
    WarehouseDialog(const RetailNext::WarehouseRecord &wh, bool isNew, QWidget *parent = nullptr)
        : QDialog(parent), m_warehouse(wh), m_isNew(isNew) {
        setWindowTitle(isNew ? QStringLiteral("🏬 New Warehouse Registration") 
                             : QStringLiteral("✏️ Edit Warehouse - %1").arg(wh.name));
        resize(480, 330);
        setStyleSheet(QStringLiteral(
            "QDialog { background-color: #D9E8F5; } "
            "QLabel { color: #0A294A; font-weight: bold; font-size: 8.5pt; } "
            "QLineEdit { background: #FFFFFF; border: 1px solid #7A9EB9; "
            "border-radius: 2px; padding: 3px 6px; color: #000080; font-weight: bold; font-size: 9pt; } "
            "QLineEdit:focus { border: 1px solid #0055A5; background-color: #FFFFF2; } "
            "QCheckBox { color: #0A294A; font-weight: bold; font-size: 8.5pt; }"
        ));

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(14, 14, 14, 14);
        mainLayout->setSpacing(10);

        QFrame *formFrame = new QFrame(this);
        formFrame->setStyleSheet(QStringLiteral("QFrame { background-color: #FFFFFF; border: 1px solid #8FAEC7; border-radius: 3px; padding: 10px; }"));
        QFormLayout *form = new QFormLayout(formFrame);
        form->setLabelAlignment(Qt::AlignRight);
        form->setSpacing(8);

        m_txtName = new QLineEdit(wh.name, formFrame);
        m_txtName->setPlaceholderText(QStringLiteral("Required (e.g. Central Logistics Hub)"));

        m_txtPhone = new QLineEdit(wh.phone, formFrame);
        m_txtPhone->setPlaceholderText(QStringLiteral("Mobile / Contact phone"));

        m_txtEmail = new QLineEdit(wh.email, formFrame);
        m_txtEmail->setPlaceholderText(QStringLiteral("Optional (e.g. logistics@company.com)"));

        m_txtCity = new QLineEdit(wh.city, formFrame);
        m_txtCity->setPlaceholderText(QStringLiteral("City"));

        m_txtAddress = new QLineEdit(wh.address, formFrame);
        m_txtAddress->setPlaceholderText(QStringLiteral("Street, Depot, Sector"));

        m_chkActive = new QCheckBox(QStringLiteral("Warehouse is Active and In Service"), formFrame);
        m_chkActive->setChecked(wh.isActive);

        form->addRow(QStringLiteral("Warehouse Name *:"), m_txtName);
        form->addRow(QStringLiteral("Mobile Number:"), m_txtPhone);
        form->addRow(QStringLiteral("Email (Optional):"), m_txtEmail);
        form->addRow(QStringLiteral("City:"), m_txtCity);
        form->addRow(QStringLiteral("Address:"), m_txtAddress);
        form->addRow(QStringLiteral("Status:"), m_chkActive);

        mainLayout->addWidget(formFrame);

        QHBoxLayout *btnLayout = new QHBoxLayout();
        btnLayout->addStretch();

        QPushButton *btnSave = new QPushButton(QStringLiteral("💾 Save Warehouse (F9)"), this);
        btnSave->setShortcut(QKeySequence(Qt::Key_F9));
        btnSave->setStyleSheet(QStringLiteral(
            "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
            "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 5px 16px; font-weight: bold; font-size: 8.5pt; } "
            "QPushButton:hover { background: #C8E6C9; border-color: #388E3C; }"
        ));
        connect(btnSave, &QPushButton::clicked, this, &WarehouseDialog::onSaveClicked);

        QPushButton *btnCancel = new QPushButton(QStringLiteral("❌ Cancel (Esc)"), this);
        btnCancel->setShortcut(QKeySequence(Qt::Key_Escape));
        btnCancel->setStyleSheet(QStringLiteral(
            "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFEBEE, stop:1 #FFCDD2); "
            "color: #B71C1C; border: 1px solid #EF9A9A; border-radius: 2px; padding: 5px 14px; font-weight: bold; font-size: 8.5pt; } "
            "QPushButton:hover { background: #FFCDD2; border-color: #D32F2F; }"
        ));
        connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

        btnLayout->addWidget(btnSave);
        btnLayout->addWidget(btnCancel);
        mainLayout->addLayout(btnLayout);

        m_txtName->setFocus();
    }

    RetailNext::WarehouseRecord getWarehouse() const { return m_warehouse; }

private:
    void onSaveClicked() {
        QString name = m_txtName->text().trimmed();
        if (name.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("Validation Error"),
                                 QStringLiteral("Warehouse Name is mandatory. Please enter a valid name."));
            m_txtName->setFocus();
            return;
        }

        m_warehouse.name = name;
        m_warehouse.phone = m_txtPhone->text().trimmed();
        m_warehouse.email = m_txtEmail->text().trimmed();
        m_warehouse.city = m_txtCity->text().trimmed();
        m_warehouse.address = m_txtAddress->text().trimmed();
        m_warehouse.isActive = m_chkActive->isChecked();

        accept();
    }

    RetailNext::WarehouseRecord m_warehouse;
    bool m_isNew;
    QLineEdit *m_txtName;
    QLineEdit *m_txtPhone;
    QLineEdit *m_txtEmail;
    QLineEdit *m_txtCity;
    QLineEdit *m_txtAddress;
    QCheckBox *m_chkActive;
};

// =========================================================================
// Dialog: Add / Edit User with Permissions Matrix
// =========================================================================
class UserDialog : public QDialog {
public:
    struct ModulePermissionRow {
        QString moduleKey;
        QString displayName;
        QCheckBox *chkView;
        QCheckBox *chkEdit;
    };

    UserDialog(const RetailNext::UserRecord &user, bool isNew, QWidget *parent = nullptr)
        : QDialog(parent), m_user(user), m_isNew(isNew) {
        setWindowTitle(isNew ? QStringLiteral("👤 New User Account & Security Permissions") 
                             : QStringLiteral("✏️ Edit User - %1").arg(user.displayName));
        resize(560, 520);
        setStyleSheet(QStringLiteral(
            "QDialog { background-color: #D9E8F5; } "
            "QLabel { color: #0A294A; font-weight: bold; font-size: 8.5pt; } "
            "QLineEdit, QComboBox { background: #FFFFFF; border: 1px solid #7A9EB9; "
            "border-radius: 2px; padding: 3px 6px; color: #000080; font-weight: bold; font-size: 9pt; } "
            "QLineEdit:focus, QComboBox:focus { border: 1px solid #0055A5; background-color: #FFFFF2; } "
            "QCheckBox { color: #0A294A; font-weight: bold; font-size: 8.5pt; }"
        ));

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(12, 12, 12, 12);
        mainLayout->setSpacing(8);

        // Section 1: User Profile Details
        QFrame *profileFrame = new QFrame(this);
        profileFrame->setStyleSheet(QStringLiteral("QFrame { background-color: #FFFFFF; border: 1px solid #8FAEC7; border-radius: 3px; padding: 8px; }"));
        QGridLayout *grid = new QGridLayout(profileFrame);
        grid->setSpacing(6);

        grid->addWidget(new QLabel(QStringLiteral("User Name (Display) *:"), profileFrame), 0, 0);
        m_txtDisplayName = new QLineEdit(user.displayName, profileFrame);
        m_txtDisplayName->setPlaceholderText(QStringLiteral("Full name (e.g. Alex Johnson)"));
        grid->addWidget(m_txtDisplayName, 0, 1);

        grid->addWidget(new QLabel(QStringLiteral("Mobile Number:"), profileFrame), 0, 2);
        m_txtPhone = new QLineEdit(user.phone, profileFrame);
        m_txtPhone->setPlaceholderText(QStringLiteral("e.g. +1 555-0101"));
        grid->addWidget(m_txtPhone, 0, 3);

        grid->addWidget(new QLabel(QStringLiteral("Username (Login) *:"), profileFrame), 1, 0);
        m_txtUsername = new QLineEdit(user.username, profileFrame);
        m_txtUsername->setPlaceholderText(QStringLiteral("Login username (e.g. alexj)"));
        grid->addWidget(m_txtUsername, 1, 1);

        grid->addWidget(new QLabel(QStringLiteral("Password *:"), profileFrame), 1, 2);
        m_txtPassword = new QLineEdit(user.password, profileFrame);
        m_txtPassword->setEchoMode(QLineEdit::Password);
        m_txtPassword->setPlaceholderText(QStringLiteral("Login password"));
        grid->addWidget(m_txtPassword, 1, 3);

        grid->addWidget(new QLabel(QStringLiteral("Role:"), profileFrame), 2, 0);
        m_comboRole = new QComboBox(profileFrame);
        m_comboRole->addItems(QStringList() << QStringLiteral("Store Admin") 
                                           << QStringLiteral("Manager") 
                                           << QStringLiteral("Cashier") 
                                           << QStringLiteral("Accountant") 
                                           << QStringLiteral("Inventory Clerk"));
        m_comboRole->setCurrentText(user.role.isEmpty() ? QStringLiteral("Cashier") : user.role);
        grid->addWidget(m_comboRole, 2, 1);

        m_chkActive = new QCheckBox(QStringLiteral("Account Active"), profileFrame);
        m_chkActive->setChecked(user.isActive);
        grid->addWidget(m_chkActive, 2, 2, 1, 2);

        mainLayout->addWidget(profileFrame);

        // Section 2: Permissions Matrix
        QLabel *lblPermTitle = new QLabel(QStringLiteral("🔐 Menu Access & Permissions Matrix:"), this);
        lblPermTitle->setStyleSheet(QStringLiteral("color: #000080; font-weight: bold; font-size: 9pt;"));
        mainLayout->addWidget(lblPermTitle);

        QFrame *permFrame = new QFrame(this);
        permFrame->setStyleSheet(QStringLiteral("QFrame { background-color: #FFFFFF; border: 1px solid #8FAEC7; border-radius: 3px; padding: 6px; }"));
        QVBoxLayout *permLayout = new QVBoxLayout(permFrame);
        permLayout->setContentsMargins(4, 4, 4, 4);
        permLayout->setSpacing(4);

        // Quick Preset Toolbar
        QHBoxLayout *presetBar = new QHBoxLayout();
        presetBar->setSpacing(6);
        presetBar->addWidget(new QLabel(QStringLiteral("Quick Presets:"), permFrame));

        auto makePresetBtn = [this, permFrame](const QString &text, const QString &shortcut, auto func) {
            QPushButton *b = new QPushButton(text, permFrame);
            b->setShortcut(QKeySequence(shortcut));
            b->setStyleSheet(QStringLiteral(
                "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
                "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 2px 7px; font-size: 8pt; font-weight: bold; } "
                "QPushButton:hover { background: #EDF4FB; border-color: #0055A5; }"
            ));
            connect(b, &QPushButton::clicked, this, func);
            return b;
        };

        presetBar->addWidget(makePresetBtn(QStringLiteral("👑 Admin All (Alt+A)"), QStringLiteral("Alt+A"), [this]() { setPreset("admin"); }));
        presetBar->addWidget(makePresetBtn(QStringLiteral("🛒 Cashier (Alt+C)"), QStringLiteral("Alt+C"), [this]() { setPreset("cashier"); }));
        presetBar->addWidget(makePresetBtn(QStringLiteral("📦 Inventory (Alt+I)"), QStringLiteral("Alt+I"), [this]() { setPreset("inventory"); }));
        presetBar->addWidget(makePresetBtn(QStringLiteral("🧹 Clear All (Alt+X)"), QStringLiteral("Alt+X"), [this]() { setPreset("none"); }));
        presetBar->addStretch();
        permLayout->addLayout(presetBar);

        // Matrix Header
        QFrame *matrixHeader = new QFrame(permFrame);
        matrixHeader->setFixedHeight(24);
        matrixHeader->setStyleSheet(QStringLiteral("background: #D9E8F5; border-bottom: 1px solid #7A9EB9; font-weight: bold; color: #0A294A;"));
        QHBoxLayout *mhl = new QHBoxLayout(matrixHeader);
        mhl->setContentsMargins(8, 0, 8, 0);
        QLabel *mhMod = new QLabel(QStringLiteral("Menu / Module"), matrixHeader);
        QLabel *mhView = new QLabel(QStringLiteral("👁️ View Access"), matrixHeader);
        QLabel *mhEdit = new QLabel(QStringLiteral("✏️ Edit / Modify"), matrixHeader);
        mhView->setFixedWidth(100);
        mhEdit->setFixedWidth(100);
        mhl->addWidget(mhMod, 1);
        mhl->addWidget(mhView);
        mhl->addWidget(mhEdit);
        permLayout->addWidget(matrixHeader);

        // Modules List
        const QList<QPair<QString, QString>> modules = {
            {QStringLiteral("Dashboard"), QStringLiteral("📁 Dashboard & Daily Overview")},
            {QStringLiteral("Masters"), QStringLiteral("📦 Masters (Products, Categories, Brands, Units)")},
            {QStringLiteral("Purchase"), QStringLiteral("📥 Purchase (Invoices, Orders, Goods Received)")},
            {QStringLiteral("Inventory"), QStringLiteral("📊 Inventory (Stock Ledger, Adjustments, Transfers)")},
            {QStringLiteral("Sales"), QStringLiteral("🛒 Sales (POS Billing, Invoices, Orders, Returns)")},
            {QStringLiteral("Customers"), QStringLiteral("👥 Customers (Registry, Ledger, Outstanding Dues)")},
            {QStringLiteral("Accounts"), QStringLiteral("💰 Accounts (Cash Register, Expenses, Day Closing)")},
            {QStringLiteral("Reports"), QStringLiteral("📈 Reports (Sales, Profit, Inventory, Tax Reports)")},
            {QStringLiteral("Access"), QStringLiteral("🔐 Access (Stores, Warehouses, Users)")},
            {QStringLiteral("Settings"), QStringLiteral("⚙️ Settings (Business Profile, Branch, Hardware)")}
        };

        // Parse existing permissions string
        // format: "Dashboard:view,edit;Sales:view;..."
        QMap<QString, QStringList> currentPermMap;
        for (const QString &part : user.permissions.split(';', Qt::SkipEmptyParts)) {
            QStringList tokens = part.split(':');
            if (tokens.size() == 2) {
                currentPermMap[tokens[0]] = tokens[1].split(',', Qt::SkipEmptyParts);
            }
        }

        for (const auto &pair : modules) {
            ModulePermissionRow row;
            row.moduleKey = pair.first;
            row.displayName = pair.second;

            QFrame *rowFrame = new QFrame(permFrame);
            rowFrame->setFixedHeight(22);
            rowFrame->setStyleSheet(QStringLiteral("border-bottom: 1px solid #ECECEC;"));
            QHBoxLayout *rhl = new QHBoxLayout(rowFrame);
            rhl->setContentsMargins(8, 0, 8, 0);

            QLabel *lblMod = new QLabel(pair.second, rowFrame);
            lblMod->setStyleSheet(QStringLiteral("color: #0A294A; font-size: 8.5pt;"));
            rhl->addWidget(lblMod, 1);

            row.chkView = new QCheckBox(QStringLiteral("Can View"), rowFrame);
            row.chkView->setFixedWidth(100);
            row.chkEdit = new QCheckBox(QStringLiteral("Can Edit"), rowFrame);
            row.chkEdit->setFixedWidth(100);

            bool hasView = currentPermMap.contains(pair.first) && currentPermMap[pair.first].contains("view");
            bool hasEdit = currentPermMap.contains(pair.first) && currentPermMap[pair.first].contains("edit");

            // If empty permissions on new user, grant default cashier permissions
            if (user.permissions.isEmpty() && isNew) {
                if (pair.first == "Dashboard" || pair.first == "Sales") {
                    hasView = true;
                    hasEdit = true;
                } else if (pair.first == "Customers") {
                    hasView = true;
                }
            }

            row.chkView->setChecked(hasView);
            row.chkEdit->setChecked(hasEdit);

            // If Edit checked, automatically ensure View is checked
            connect(row.chkEdit, &QCheckBox::toggled, row.chkView, [row](bool checked) {
                if (checked) row.chkView->setChecked(true);
            });

            rhl->addWidget(row.chkView);
            rhl->addWidget(row.chkEdit);
            permLayout->addWidget(rowFrame);

            m_permRows.append(row);
        }

        mainLayout->addWidget(permFrame);

        // Buttons
        QHBoxLayout *btnLayout = new QHBoxLayout();
        btnLayout->addStretch();

        QPushButton *btnSave = new QPushButton(QStringLiteral("💾 Save User (F9)"), this);
        btnSave->setShortcut(QKeySequence(Qt::Key_F9));
        btnSave->setStyleSheet(QStringLiteral(
            "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
            "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 5px 16px; font-weight: bold; font-size: 8.5pt; } "
            "QPushButton:hover { background: #C8E6C9; border-color: #388E3C; }"
        ));
        connect(btnSave, &QPushButton::clicked, this, &UserDialog::onSaveClicked);

        QPushButton *btnCancel = new QPushButton(QStringLiteral("❌ Cancel (Esc)"), this);
        btnCancel->setShortcut(QKeySequence(Qt::Key_Escape));
        btnCancel->setStyleSheet(QStringLiteral(
            "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFEBEE, stop:1 #FFCDD2); "
            "color: #B71C1C; border: 1px solid #EF9A9A; border-radius: 2px; padding: 5px 14px; font-weight: bold; font-size: 8.5pt; } "
            "QPushButton:hover { background: #FFCDD2; border-color: #D32F2F; }"
        ));
        connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

        btnLayout->addWidget(btnSave);
        btnLayout->addWidget(btnCancel);
        mainLayout->addLayout(btnLayout);

        m_txtDisplayName->setFocus();
    }

    RetailNext::UserRecord getUser() const { return m_user; }

private:
    void setPreset(const QString &preset) {
        for (auto &row : m_permRows) {
            if (preset == "admin") {
                row.chkView->setChecked(true);
                row.chkEdit->setChecked(true);
            } else if (preset == "cashier") {
                bool isPos = (row.moduleKey == "Dashboard" || row.moduleKey == "Sales");
                bool isCust = (row.moduleKey == "Customers");
                row.chkView->setChecked(isPos || isCust);
                row.chkEdit->setChecked(isPos);
            } else if (preset == "inventory") {
                bool isInv = (row.moduleKey == "Dashboard" || row.moduleKey == "Masters" || 
                              row.moduleKey == "Inventory" || row.moduleKey == "Purchase");
                row.chkView->setChecked(isInv);
                row.chkEdit->setChecked(isInv);
            } else {
                row.chkView->setChecked(false);
                row.chkEdit->setChecked(false);
            }
        }
    }

    void onSaveClicked() {
        QString displayName = m_txtDisplayName->text().trimmed();
        QString username = m_txtUsername->text().trimmed();
        QString password = m_txtPassword->text().trimmed();

        if (displayName.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("Validation Error"),
                                 QStringLiteral("User Name (Display Name) is mandatory."));
            m_txtDisplayName->setFocus();
            return;
        }

        if (username.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("Validation Error"),
                                 QStringLiteral("Username is mandatory."));
            m_txtUsername->setFocus();
            return;
        }

        if (password.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("Validation Error"),
                                 QStringLiteral("Password is mandatory."));
            m_txtPassword->setFocus();
            return;
        }

        // Build permissions string
        QStringList parts;
        for (const auto &row : m_permRows) {
            QStringList perms;
            if (row.chkView->isChecked()) perms << QStringLiteral("view");
            if (row.chkEdit->isChecked()) perms << QStringLiteral("edit");
            if (!perms.isEmpty()) {
                parts << QStringLiteral("%1:%2").arg(row.moduleKey, perms.join(','));
            }
        }

        m_user.displayName = displayName;
        m_user.phone = m_txtPhone->text().trimmed();
        m_user.username = username;
        m_user.password = password;
        m_user.role = m_comboRole->currentText();
        m_user.permissions = parts.join(';');
        m_user.isActive = m_chkActive->isChecked();

        accept();
    }

    RetailNext::UserRecord m_user;
    bool m_isNew;
    QLineEdit *m_txtDisplayName;
    QLineEdit *m_txtPhone;
    QLineEdit *m_txtUsername;
    QLineEdit *m_txtPassword;
    QComboBox *m_comboRole;
    QCheckBox *m_chkActive;
    QList<ModulePermissionRow> m_permRows;
};

// =========================================================================
// Main AccessPage Widget Implementation
// =========================================================================

AccessPage::AccessPage(QWidget *parent)
    : QWidget(parent)
    , m_subTabs(nullptr)
    , m_txtStoreSearch(nullptr)
    , m_tableStores(nullptr)
    , m_emptyStoresBanner(nullptr)
    , m_lblStoresSummary(nullptr)
    , m_txtWarehouseSearch(nullptr)
    , m_tableWarehouses(nullptr)
    , m_emptyWarehousesBanner(nullptr)
    , m_lblWarehousesSummary(nullptr)
    , m_txtUserSearch(nullptr)
    , m_tableUsers(nullptr)
    , m_emptyUsersBanner(nullptr)
    , m_lblUsersSummary(nullptr)
{
    setupUI();
    reloadStores();
    reloadWarehouses();
    reloadUsers();
}

void AccessPage::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 8, 10, 8);
    mainLayout->setSpacing(6);

    setStyleSheet(QStringLiteral("QWidget { background-color: #D9E8F5; }"));

    m_subTabs = new QTabWidget(this);
    m_subTabs->setStyleSheet(QStringLiteral(
        "QTabWidget::pane { border: 1px solid #7A9EB9; background-color: #D9E8F5; } "
        "QTabBar::tab { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #F0F6FC, stop:1 #D9E8F5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-bottom: none; border-top-left-radius: 3px; "
        "border-top-right-radius: 3px; padding: 4px 16px; font-weight: bold; font-size: 8.5pt; margin-right: 2px; } "
        "QTabBar::tab:selected { background: #FFFFFF; color: #000080; border-bottom: 1px solid #FFFFFF; } "
        "QTabBar::tab:hover { background: #E5EFF8; }"
    ));

    m_subTabs->addTab(createStoresTab(), QStringLiteral("🏪 Stores Registry (Alt+1)"));
    m_subTabs->addTab(createWarehousesTab(), QStringLiteral("🏬 Warehouses Registry (Alt+2)"));
    m_subTabs->addTab(createUsersTab(), QStringLiteral("👤 Users & Permissions Matrix (Alt+3)"));

    connect(m_subTabs, &QTabWidget::currentChanged, this, &AccessPage::onTabChanged);
    mainLayout->addWidget(m_subTabs, 1);

    // Bottom Function Key Bar
    mainLayout->addWidget(createBottomFunctionBar());
}

QWidget* AccessPage::createStoresTab() {
    QWidget *tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    // Header Toolbar
    QFrame *tb = new QFrame(tab);
    tb->setStyleSheet(QStringLiteral("QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; padding: 3px; }"));
    QHBoxLayout *tbLayout = new QHBoxLayout(tb);
    tbLayout->setContentsMargins(6, 2, 6, 2);
    tbLayout->setSpacing(8);

    QLabel *lblIcon = new QLabel(QStringLiteral("🏪"), tb);
    lblIcon->setStyleSheet(QStringLiteral("font-size: 16pt;"));
    tbLayout->addWidget(lblIcon);

    QVBoxLayout *lblTitles = new QVBoxLayout();
    lblTitles->setSpacing(0);
    QLabel *lblTitle = new QLabel(QStringLiteral("Stores & Retail Outlets"), tb);
    lblTitle->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 9.5pt; color: #000080;"));
    QLabel *lblSub = new QLabel(QStringLiteral("Manage registered retail store locations, GST details, contact numbers, and address info."), tb);
    lblSub->setStyleSheet(QStringLiteral("font-size: 8pt; color: #4A6B82; font-style: italic;"));
    lblTitles->addWidget(lblTitle);
    lblTitles->addWidget(lblSub);
    tbLayout->addLayout(lblTitles);
    tbLayout->addStretch();

    QLabel *lblSearch = new QLabel(QStringLiteral("Search:"), tb);
    lblSearch->setStyleSheet(QStringLiteral("font-weight: bold; color: #0A294A; font-size: 8.5pt;"));
    m_txtStoreSearch = new QLineEdit(tb);
    m_txtStoreSearch->setPlaceholderText(QStringLiteral("Search store name, city, GST, phone... (Ctrl+F)"));
    m_txtStoreSearch->setMinimumWidth(220);
    m_txtStoreSearch->setStyleSheet(QStringLiteral(
        "QLineEdit { background: #FFFFFF; border: 1px solid #7A9EB9; border-radius: 2px; "
        "padding: 2px 6px; color: #000080; font-weight: bold; font-size: 8.5pt; } "
        "QLineEdit:focus { border: 1px solid #0055A5; background-color: #FFFFF2; }"
    ));
    connect(m_txtStoreSearch, &QLineEdit::textChanged, this, &AccessPage::onStoresSearchChanged);
    tbLayout->addWidget(lblSearch);
    tbLayout->addWidget(m_txtStoreSearch);

    QPushButton *btnNew = new QPushButton(QStringLiteral("➕ New Store (F3)"), tb);
    btnNew->setShortcut(QKeySequence(Qt::Key_F3));
    btnNew->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 3px 12px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #C8E6C9; }"
    ));
    connect(btnNew, &QPushButton::clicked, this, &AccessPage::openNewStoreDialog);
    tbLayout->addWidget(btnNew);

    QPushButton *btnEdit = new QPushButton(QStringLiteral("✏️ Edit Store (F4)"), tb);
    btnEdit->setShortcut(QKeySequence(Qt::Key_F4));
    btnEdit->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; }"
    ));
    connect(btnEdit, &QPushButton::clicked, this, &AccessPage::openEditStoreDialog);
    tbLayout->addWidget(btnEdit);

    QPushButton *btnDelete = new QPushButton(QStringLiteral("🗑️ Delete (Del)"), tb);
    btnDelete->setShortcut(QKeySequence(Qt::Key_Delete));
    btnDelete->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFEBEE, stop:1 #FFCDD2); "
        "color: #B71C1C; border: 1px solid #EF9A9A; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #FFCDD2; }"
    ));
    connect(btnDelete, &QPushButton::clicked, this, &AccessPage::deleteSelectedStore);
    tbLayout->addWidget(btnDelete);

    QPushButton *btnRefresh = new QPushButton(QStringLiteral("🔄 Refresh (F5)"), tb);
    btnRefresh->setShortcut(QKeySequence(Qt::Key_F5));
    btnRefresh->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; }"
    ));
    connect(btnRefresh, &QPushButton::clicked, this, &AccessPage::reloadStores);
    tbLayout->addWidget(btnRefresh);

    layout->addWidget(tb);

    // Empty state banner
    m_emptyStoresBanner = new QFrame(tab);
    m_emptyStoresBanner->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #FFF9C4; border: 1px solid #FBC02D; border-radius: 3px; padding: 6px; }"
    ));
    QHBoxLayout *esLayout = new QHBoxLayout(m_emptyStoresBanner);
    esLayout->setContentsMargins(8, 2, 8, 2);
    QLabel *esLabel = new QLabel(QStringLiteral("⚠️ No stores configured. Click '➕ New Store (F3)' to create your first store location."), m_emptyStoresBanner);
    esLabel->setStyleSheet(QStringLiteral("font-weight: bold; color: #827717; font-size: 8.5pt;"));
    esLayout->addWidget(esLabel);
    m_emptyStoresBanner->hide();
    layout->addWidget(m_emptyStoresBanner);

    // Table
    m_tableStores = new QTableWidget(tab);
    m_tableStores->setColumnCount(8);
    m_tableStores->setHorizontalHeaderLabels(QStringList()
        << QStringLiteral("Store Code")
        << QStringLiteral("Store Name")
        << QStringLiteral("Mobile Number")
        << QStringLiteral("Email")
        << QStringLiteral("City")
        << QStringLiteral("Address")
        << QStringLiteral("GST Number")
        << QStringLiteral("Status"));
    m_tableStores->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableStores->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableStores->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    m_tableStores->verticalHeader()->setVisible(false);
    m_tableStores->verticalHeader()->setDefaultSectionSize(22);
    m_tableStores->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableStores->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableStores->setAlternatingRowColors(true);
    m_tableStores->setStyleSheet(QStringLiteral(
        "QTableWidget { background-color: #FFFFFF; alternate-background-color: #F8FBFE; "
        "gridline-color: #C2D7EA; border: 1px solid #7A9EB9; font-size: 8.5pt; color: #0A294A; } "
        "QTableWidget::item { padding: 2px 4px; } "
        "QTableWidget::item:selected { background-color: #2D82D6; color: #FFFFFF; font-weight: bold; } "
        "QHeaderView::section { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B3D4F0, stop:1 #8DBDE0); "
        "color: #0A294A; font-weight: bold; font-size: 8pt; border: none; border-top: 1px solid #C2D7EA; "
        "border-bottom: 1px solid #6895BA; border-right: 1px solid #99BEDC; padding: 3px 5px; }"
    ));
    connect(m_tableStores, &QTableWidget::cellDoubleClicked, this, &AccessPage::openEditStoreDialog);
    layout->addWidget(m_tableStores, 1);

    // Footer
    QFrame *fb = new QFrame(tab);
    fb->setFixedHeight(26);
    fb->setStyleSheet(QStringLiteral("QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; }"));
    QHBoxLayout *fbLayout = new QHBoxLayout(fb);
    fbLayout->setContentsMargins(6, 1, 6, 1);
    m_lblStoresSummary = new QLabel(fb);
    m_lblStoresSummary->setStyleSheet(QStringLiteral("font-weight: bold; color: #000080; font-size: 8.5pt;"));
    fbLayout->addWidget(m_lblStoresSummary);
    fbLayout->addStretch();
    layout->addWidget(fb);

    return tab;
}

QWidget* AccessPage::createWarehousesTab() {
    QWidget *tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    // Header Toolbar
    QFrame *tb = new QFrame(tab);
    tb->setStyleSheet(QStringLiteral("QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; padding: 3px; }"));
    QHBoxLayout *tbLayout = new QHBoxLayout(tb);
    tbLayout->setContentsMargins(6, 2, 6, 2);
    tbLayout->setSpacing(8);

    QLabel *lblIcon = new QLabel(QStringLiteral("🏬"), tb);
    lblIcon->setStyleSheet(QStringLiteral("font-size: 16pt;"));
    tbLayout->addWidget(lblIcon);

    QVBoxLayout *lblTitles = new QVBoxLayout();
    lblTitles->setSpacing(0);
    QLabel *lblTitle = new QLabel(QStringLiteral("Warehouses & Storage Depots"), tb);
    lblTitle->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 9.5pt; color: #000080;"));
    QLabel *lblSub = new QLabel(QStringLiteral("Manage central warehouse logistics, storage fulfillment hubs, contact details, and locations."), tb);
    lblSub->setStyleSheet(QStringLiteral("font-size: 8pt; color: #4A6B82; font-style: italic;"));
    lblTitles->addWidget(lblTitle);
    lblTitles->addWidget(lblSub);
    tbLayout->addLayout(lblTitles);
    tbLayout->addStretch();

    QLabel *lblSearch = new QLabel(QStringLiteral("Search:"), tb);
    lblSearch->setStyleSheet(QStringLiteral("font-weight: bold; color: #0A294A; font-size: 8.5pt;"));
    m_txtWarehouseSearch = new QLineEdit(tb);
    m_txtWarehouseSearch->setPlaceholderText(QStringLiteral("Search warehouse name, city, phone... (Ctrl+F)"));
    m_txtWarehouseSearch->setMinimumWidth(220);
    m_txtWarehouseSearch->setStyleSheet(QStringLiteral(
        "QLineEdit { background: #FFFFFF; border: 1px solid #7A9EB9; border-radius: 2px; "
        "padding: 2px 6px; color: #000080; font-weight: bold; font-size: 8.5pt; } "
        "QLineEdit:focus { border: 1px solid #0055A5; background-color: #FFFFF2; }"
    ));
    connect(m_txtWarehouseSearch, &QLineEdit::textChanged, this, &AccessPage::onWarehousesSearchChanged);
    tbLayout->addWidget(lblSearch);
    tbLayout->addWidget(m_txtWarehouseSearch);

    QPushButton *btnNew = new QPushButton(QStringLiteral("➕ New Warehouse (F3)"), tb);
    btnNew->setShortcut(QKeySequence(Qt::Key_F3));
    btnNew->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 3px 12px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #C8E6C9; }"
    ));
    connect(btnNew, &QPushButton::clicked, this, &AccessPage::openNewWarehouseDialog);
    tbLayout->addWidget(btnNew);

    QPushButton *btnEdit = new QPushButton(QStringLiteral("✏️ Edit Warehouse (F4)"), tb);
    btnEdit->setShortcut(QKeySequence(Qt::Key_F4));
    btnEdit->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; }"
    ));
    connect(btnEdit, &QPushButton::clicked, this, &AccessPage::openEditWarehouseDialog);
    tbLayout->addWidget(btnEdit);

    QPushButton *btnDelete = new QPushButton(QStringLiteral("🗑️ Delete (Del)"), tb);
    btnDelete->setShortcut(QKeySequence(Qt::Key_Delete));
    btnDelete->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFEBEE, stop:1 #FFCDD2); "
        "color: #B71C1C; border: 1px solid #EF9A9A; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #FFCDD2; }"
    ));
    connect(btnDelete, &QPushButton::clicked, this, &AccessPage::deleteSelectedWarehouse);
    tbLayout->addWidget(btnDelete);

    QPushButton *btnRefresh = new QPushButton(QStringLiteral("🔄 Refresh (F5)"), tb);
    btnRefresh->setShortcut(QKeySequence(Qt::Key_F5));
    btnRefresh->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; }"
    ));
    connect(btnRefresh, &QPushButton::clicked, this, &AccessPage::reloadWarehouses);
    tbLayout->addWidget(btnRefresh);

    layout->addWidget(tb);

    // Empty state banner
    m_emptyWarehousesBanner = new QFrame(tab);
    m_emptyWarehousesBanner->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #FFF9C4; border: 1px solid #FBC02D; border-radius: 3px; padding: 6px; }"
    ));
    QHBoxLayout *ewLayout = new QHBoxLayout(m_emptyWarehousesBanner);
    ewLayout->setContentsMargins(8, 2, 8, 2);
    QLabel *ewLabel = new QLabel(QStringLiteral("⚠️ No warehouses configured. Click '➕ New Warehouse (F3)' to create your first warehouse depot."), m_emptyWarehousesBanner);
    ewLabel->setStyleSheet(QStringLiteral("font-weight: bold; color: #827717; font-size: 8.5pt;"));
    ewLayout->addWidget(ewLabel);
    m_emptyWarehousesBanner->hide();
    layout->addWidget(m_emptyWarehousesBanner);

    // Table
    m_tableWarehouses = new QTableWidget(tab);
    m_tableWarehouses->setColumnCount(7);
    m_tableWarehouses->setHorizontalHeaderLabels(QStringList()
        << QStringLiteral("Warehouse Code")
        << QStringLiteral("Warehouse Name")
        << QStringLiteral("Mobile Number")
        << QStringLiteral("Email")
        << QStringLiteral("City")
        << QStringLiteral("Address")
        << QStringLiteral("Status"));
    m_tableWarehouses->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableWarehouses->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableWarehouses->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    m_tableWarehouses->verticalHeader()->setVisible(false);
    m_tableWarehouses->verticalHeader()->setDefaultSectionSize(22);
    m_tableWarehouses->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWarehouses->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWarehouses->setAlternatingRowColors(true);
    m_tableWarehouses->setStyleSheet(QStringLiteral(
        "QTableWidget { background-color: #FFFFFF; alternate-background-color: #F8FBFE; "
        "gridline-color: #C2D7EA; border: 1px solid #7A9EB9; font-size: 8.5pt; color: #0A294A; } "
        "QTableWidget::item { padding: 2px 4px; } "
        "QTableWidget::item:selected { background-color: #2D82D6; color: #FFFFFF; font-weight: bold; } "
        "QHeaderView::section { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B3D4F0, stop:1 #8DBDE0); "
        "color: #0A294A; font-weight: bold; font-size: 8pt; border: none; border-top: 1px solid #C2D7EA; "
        "border-bottom: 1px solid #6895BA; border-right: 1px solid #99BEDC; padding: 3px 5px; }"
    ));
    connect(m_tableWarehouses, &QTableWidget::cellDoubleClicked, this, &AccessPage::openEditWarehouseDialog);
    layout->addWidget(m_tableWarehouses, 1);

    // Footer
    QFrame *fb = new QFrame(tab);
    fb->setFixedHeight(26);
    fb->setStyleSheet(QStringLiteral("QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; }"));
    QHBoxLayout *fbLayout = new QHBoxLayout(fb);
    fbLayout->setContentsMargins(6, 1, 6, 1);
    m_lblWarehousesSummary = new QLabel(fb);
    m_lblWarehousesSummary->setStyleSheet(QStringLiteral("font-weight: bold; color: #000080; font-size: 8.5pt;"));
    fbLayout->addWidget(m_lblWarehousesSummary);
    fbLayout->addStretch();
    layout->addWidget(fb);

    return tab;
}

QWidget* AccessPage::createUsersTab() {
    QWidget *tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    // Header Toolbar
    QFrame *tb = new QFrame(tab);
    tb->setStyleSheet(QStringLiteral("QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; padding: 3px; }"));
    QHBoxLayout *tbLayout = new QHBoxLayout(tb);
    tbLayout->setContentsMargins(6, 2, 6, 2);
    tbLayout->setSpacing(8);

    QLabel *lblIcon = new QLabel(QStringLiteral("👤"), tb);
    lblIcon->setStyleSheet(QStringLiteral("font-size: 16pt;"));
    tbLayout->addWidget(lblIcon);

    QVBoxLayout *lblTitles = new QVBoxLayout();
    lblTitles->setSpacing(0);
    QLabel *lblTitle = new QLabel(QStringLiteral("User Accounts & Access Permissions"), tb);
    lblTitle->setStyleSheet(QStringLiteral("font-weight: bold; font-size: 9.5pt; color: #000080;"));
    QLabel *lblSub = new QLabel(QStringLiteral("Configure operator logins, contact phones, role matrices, and granular menu View/Edit privileges."), tb);
    lblSub->setStyleSheet(QStringLiteral("font-size: 8pt; color: #4A6B82; font-style: italic;"));
    lblTitles->addWidget(lblTitle);
    lblTitles->addWidget(lblSub);
    tbLayout->addLayout(lblTitles);
    tbLayout->addStretch();

    QLabel *lblSearch = new QLabel(QStringLiteral("Search:"), tb);
    lblSearch->setStyleSheet(QStringLiteral("font-weight: bold; color: #0A294A; font-size: 8.5pt;"));
    m_txtUserSearch = new QLineEdit(tb);
    m_txtUserSearch->setPlaceholderText(QStringLiteral("Search name, username, role, phone... (Ctrl+F)"));
    m_txtUserSearch->setMinimumWidth(220);
    m_txtUserSearch->setStyleSheet(QStringLiteral(
        "QLineEdit { background: #FFFFFF; border: 1px solid #7A9EB9; border-radius: 2px; "
        "padding: 2px 6px; color: #000080; font-weight: bold; font-size: 8.5pt; } "
        "QLineEdit:focus { border: 1px solid #0055A5; background-color: #FFFFF2; }"
    ));
    connect(m_txtUserSearch, &QLineEdit::textChanged, this, &AccessPage::onUsersSearchChanged);
    tbLayout->addWidget(lblSearch);
    tbLayout->addWidget(m_txtUserSearch);

    QPushButton *btnNew = new QPushButton(QStringLiteral("➕ New User (F3)"), tb);
    btnNew->setShortcut(QKeySequence(Qt::Key_F3));
    btnNew->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 3px 12px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #C8E6C9; }"
    ));
    connect(btnNew, &QPushButton::clicked, this, &AccessPage::openNewUserDialog);
    tbLayout->addWidget(btnNew);

    QPushButton *btnEdit = new QPushButton(QStringLiteral("✏️ Edit User (F4)"), tb);
    btnEdit->setShortcut(QKeySequence(Qt::Key_F4));
    btnEdit->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; }"
    ));
    connect(btnEdit, &QPushButton::clicked, this, &AccessPage::openEditUserDialog);
    tbLayout->addWidget(btnEdit);

    QPushButton *btnDelete = new QPushButton(QStringLiteral("🗑️ Delete (Del)"), tb);
    btnDelete->setShortcut(QKeySequence(Qt::Key_Delete));
    btnDelete->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFEBEE, stop:1 #FFCDD2); "
        "color: #B71C1C; border: 1px solid #EF9A9A; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #FFCDD2; }"
    ));
    connect(btnDelete, &QPushButton::clicked, this, &AccessPage::deleteSelectedUser);
    tbLayout->addWidget(btnDelete);

    QPushButton *btnRefresh = new QPushButton(QStringLiteral("🔄 Refresh (F5)"), tb);
    btnRefresh->setShortcut(QKeySequence(Qt::Key_F5));
    btnRefresh->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; }"
    ));
    connect(btnRefresh, &QPushButton::clicked, this, &AccessPage::reloadUsers);
    tbLayout->addWidget(btnRefresh);

    layout->addWidget(tb);

    // Empty state banner
    m_emptyUsersBanner = new QFrame(tab);
    m_emptyUsersBanner->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #FFF9C4; border: 1px solid #FBC02D; border-radius: 3px; padding: 6px; }"
    ));
    QHBoxLayout *euLayout = new QHBoxLayout(m_emptyUsersBanner);
    euLayout->setContentsMargins(8, 2, 8, 2);
    QLabel *euLabel = new QLabel(QStringLiteral("⚠️ No users found. Click '➕ New User (F3)' to create an operator login account."), m_emptyUsersBanner);
    euLabel->setStyleSheet(QStringLiteral("font-weight: bold; color: #827717; font-size: 8.5pt;"));
    euLayout->addWidget(euLabel);
    m_emptyUsersBanner->hide();
    layout->addWidget(m_emptyUsersBanner);

    // Table
    m_tableUsers = new QTableWidget(tab);
    m_tableUsers->setColumnCount(7);
    m_tableUsers->setHorizontalHeaderLabels(QStringList()
        << QStringLiteral("Username")
        << QStringLiteral("Display Name")
        << QStringLiteral("Mobile Number")
        << QStringLiteral("Role")
        << QStringLiteral("Permitted Menus (View / Edit)")
        << QStringLiteral("Status")
        << QStringLiteral("Created Date"));
    m_tableUsers->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableUsers->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tableUsers->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_tableUsers->verticalHeader()->setVisible(false);
    m_tableUsers->verticalHeader()->setDefaultSectionSize(22);
    m_tableUsers->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableUsers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableUsers->setAlternatingRowColors(true);
    m_tableUsers->setStyleSheet(QStringLiteral(
        "QTableWidget { background-color: #FFFFFF; alternate-background-color: #F8FBFE; "
        "gridline-color: #C2D7EA; border: 1px solid #7A9EB9; font-size: 8.5pt; color: #0A294A; } "
        "QTableWidget::item { padding: 2px 4px; } "
        "QTableWidget::item:selected { background-color: #2D82D6; color: #FFFFFF; font-weight: bold; } "
        "QHeaderView::section { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B3D4F0, stop:1 #8DBDE0); "
        "color: #0A294A; font-weight: bold; font-size: 8pt; border: none; border-top: 1px solid #C2D7EA; "
        "border-bottom: 1px solid #6895BA; border-right: 1px solid #99BEDC; padding: 3px 5px; }"
    ));
    connect(m_tableUsers, &QTableWidget::cellDoubleClicked, this, &AccessPage::openEditUserDialog);
    layout->addWidget(m_tableUsers, 1);

    // Footer
    QFrame *fb = new QFrame(tab);
    fb->setFixedHeight(26);
    fb->setStyleSheet(QStringLiteral("QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; }"));
    QHBoxLayout *fbLayout = new QHBoxLayout(fb);
    fbLayout->setContentsMargins(6, 1, 6, 1);
    m_lblUsersSummary = new QLabel(fb);
    m_lblUsersSummary->setStyleSheet(QStringLiteral("font-weight: bold; color: #000080; font-size: 8.5pt;"));
    fbLayout->addWidget(m_lblUsersSummary);
    fbLayout->addStretch();
    layout->addWidget(fb);

    return tab;
}

QFrame* AccessPage::createBottomFunctionBar() {
    QFrame *fnBar = new QFrame(this);
    fnBar->setFixedHeight(30);
    fnBar->setStyleSheet(QStringLiteral("QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; }"));
    QHBoxLayout *fnLayout = new QHBoxLayout(fnBar);
    fnLayout->setContentsMargins(4, 2, 4, 2);
    fnLayout->setSpacing(4);

    auto makeFnBtn = [this, fnBar](const QString &text, const QString &seq, auto slot) {
        QPushButton *btn = new QPushButton(text, fnBar);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setShortcut(QKeySequence(seq));
        btn->setStyleSheet(QStringLiteral(
            "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
            "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 2px 8px; font-size: 8pt; font-weight: bold; } "
            "QPushButton:hover { background: #EDF4FB; border-color: #0055A5; }"
        ));
        connect(btn, &QPushButton::clicked, this, slot);
        new QShortcut(QKeySequence(seq), btn, [btn]() { btn->click(); });
        return btn;
    };

    fnLayout->addWidget(makeFnBtn(QStringLiteral("F1 - Help"), QStringLiteral("F1"), [this]() {
        QMessageBox::information(this, QStringLiteral("Access Help"),
            QStringLiteral("Access Control Module:\n\n"
                           "• Stores: Register and manage physical branches & stores.\n"
                           "• Warehouses: Manage storage depots and distribution points.\n"
                           "• Users: Create user logins and grant module view/edit rights.\n\n"
                           "Shortcuts:\n"
                           "F3: New Record\n"
                           "F4: Edit Selected Record\n"
                           "Del: Delete Record\n"
                           "F5: Refresh Data Table\n"
                           "Ctrl+F: Focus Search Filter"));
    }));

    fnLayout->addWidget(makeFnBtn(QStringLiteral("F2 - Search (Ctrl+F)"), QStringLiteral("Ctrl+F"), [this]() {
        int idx = m_subTabs->currentIndex();
        if (idx == 0 && m_txtStoreSearch) m_txtStoreSearch->setFocus();
        else if (idx == 1 && m_txtWarehouseSearch) m_txtWarehouseSearch->setFocus();
        else if (idx == 2 && m_txtUserSearch) m_txtUserSearch->setFocus();
    }));

    fnLayout->addWidget(makeFnBtn(QStringLiteral("F3 - New Record"), QStringLiteral("F3"), [this]() {
        int idx = m_subTabs->currentIndex();
        if (idx == 0) openNewStoreDialog();
        else if (idx == 1) openNewWarehouseDialog();
        else if (idx == 2) openNewUserDialog();
    }));

    fnLayout->addWidget(makeFnBtn(QStringLiteral("F4 - Edit Record"), QStringLiteral("F4"), [this]() {
        int idx = m_subTabs->currentIndex();
        if (idx == 0) openEditStoreDialog();
        else if (idx == 1) openEditWarehouseDialog();
        else if (idx == 2) openEditUserDialog();
    }));

    fnLayout->addWidget(makeFnBtn(QStringLiteral("F5 - Refresh"), QStringLiteral("F5"), [this]() {
        refreshCurrentTab();
    }));

    fnLayout->addWidget(makeFnBtn(QStringLiteral("Del - Delete"), QStringLiteral("Del"), [this]() {
        int idx = m_subTabs->currentIndex();
        if (idx == 0) deleteSelectedStore();
        else if (idx == 1) deleteSelectedWarehouse();
        else if (idx == 2) deleteSelectedUser();
    }));

    fnLayout->addStretch();
    return fnBar;
}

void AccessPage::switchToSubTab(int tabIndex) {
    if (tabIndex >= 0 && tabIndex < m_subTabs->count()) {
        m_subTabs->setCurrentIndex(tabIndex);
        refreshCurrentTab();
    }
}

void AccessPage::onTabChanged(int index) {
    Q_UNUSED(index);
    refreshCurrentTab();
}

void AccessPage::refreshCurrentTab() {
    int idx = m_subTabs->currentIndex();
    if (idx == 0) reloadStores();
    else if (idx == 1) reloadWarehouses();
    else if (idx == 2) reloadUsers();
}

// =========================================================================
// Stores Logic
// =========================================================================

void AccessPage::onStoresSearchChanged() {
    reloadStores();
}

void AccessPage::reloadStores() {
    QString q = m_txtStoreSearch ? m_txtStoreSearch->text() : QString();
    m_currentStores = DatabaseManager::instance().getStoresList(q);

    m_tableStores->setRowCount(0);

    if (m_currentStores.isEmpty()) {
        m_emptyStoresBanner->show();
        m_tableStores->setRowCount(1);
        QTableWidgetItem *item = new QTableWidgetItem(
            q.isEmpty() ? QStringLiteral("⚠️ No store locations registered. Click ➕ New Store (F3 / Alt+1) to add a store.")
                        : QStringLiteral("🔍 No stores matching '%1'.").arg(q));
        item->setTextAlignment(Qt::AlignCenter);
        QFont f = item->font();
        f.setBold(true);
        item->setFont(f);
        item->setForeground(QBrush(QColor(QStringLiteral("#827717"))));
        m_tableStores->setItem(0, 1, item);
        m_lblStoresSummary->setText(QStringLiteral("Total Stores: 0 | Active Outlets: 0"));
        return;
    }

    m_emptyStoresBanner->hide();
    m_tableStores->setRowCount(m_currentStores.size());

    int activeCount = 0;
    for (int i = 0; i < m_currentStores.size(); ++i) {
        const auto &s = m_currentStores[i];
        if (s.isActive) activeCount++;

        QTableWidgetItem *itCode = new QTableWidgetItem(s.code);
        itCode->setFont(QFont(itCode->font().family(), -1, QFont::Bold));
        itCode->setForeground(QBrush(QColor(QStringLiteral("#000080"))));

        QTableWidgetItem *itName = new QTableWidgetItem(s.name);
        itName->setFont(QFont(itName->font().family(), -1, QFont::Bold));

        QTableWidgetItem *itPhone = new QTableWidgetItem(s.phone);
        QTableWidgetItem *itEmail = new QTableWidgetItem(s.email);
        QTableWidgetItem *itCity = new QTableWidgetItem(s.city);
        QTableWidgetItem *itAddr = new QTableWidgetItem(s.address);
        QTableWidgetItem *itGst = new QTableWidgetItem(s.gstNo);

        QTableWidgetItem *itStatus = new QTableWidgetItem(s.isActive ? QStringLiteral("🟢 Active") : QStringLiteral("⚪ Inactive"));
        itStatus->setTextAlignment(Qt::AlignCenter);
        itStatus->setFont(QFont(itStatus->font().family(), -1, QFont::Bold));
        itStatus->setForeground(QBrush(QColor(s.isActive ? QStringLiteral("#1B5E20") : QStringLiteral("#757575"))));

        m_tableStores->setItem(i, 0, itCode);
        m_tableStores->setItem(i, 1, itName);
        m_tableStores->setItem(i, 2, itPhone);
        m_tableStores->setItem(i, 3, itEmail);
        m_tableStores->setItem(i, 4, itCity);
        m_tableStores->setItem(i, 5, itAddr);
        m_tableStores->setItem(i, 6, itGst);
        m_tableStores->setItem(i, 7, itStatus);
    }

    m_lblStoresSummary->setText(QStringLiteral("Total Registered Stores: %1  |  Active Outlets: %2")
                                .arg(m_currentStores.size()).arg(activeCount));
}

void AccessPage::openNewStoreDialog() {
    RetailNext::StoreRecord newStore;
    newStore.isActive = true;

    StoreDialog dlg(newStore, true, this);
    if (dlg.exec() == QDialog::Accepted) {
        RetailNext::StoreRecord toSave = dlg.getStore();
        if (DatabaseManager::instance().saveStore(toSave, true)) {
            reloadStores();
            QMessageBox::information(this, QStringLiteral("Store Created"),
                                     QStringLiteral("Store '%1' has been registered successfully!").arg(toSave.name));
        } else {
            QMessageBox::critical(this, QStringLiteral("Database Error"),
                                  QStringLiteral("Failed to save store: %1").arg(DatabaseManager::instance().lastError()));
        }
    }
}

void AccessPage::openEditStoreDialog() {
    int row = m_tableStores->currentRow();
    if (row < 0 || row >= m_currentStores.size()) {
        QMessageBox::warning(this, QStringLiteral("No Selection"),
                             QStringLiteral("Please select a store from the list to edit."));
        return;
    }

    const auto &selected = m_currentStores[row];
    StoreDialog dlg(selected, false, this);
    if (dlg.exec() == QDialog::Accepted) {
        RetailNext::StoreRecord toSave = dlg.getStore();
        if (DatabaseManager::instance().saveStore(toSave, false)) {
            reloadStores();
            QMessageBox::information(this, QStringLiteral("Store Updated"),
                                     QStringLiteral("Store '%1' updated successfully!").arg(toSave.name));
        } else {
            QMessageBox::critical(this, QStringLiteral("Database Error"),
                                  QStringLiteral("Failed to update store: %1").arg(DatabaseManager::instance().lastError()));
        }
    }
}

void AccessPage::deleteSelectedStore() {
    int row = m_tableStores->currentRow();
    if (row < 0 || row >= m_currentStores.size()) {
        QMessageBox::warning(this, QStringLiteral("No Selection"),
                             QStringLiteral("Please select a store to delete."));
        return;
    }

    const auto &selected = m_currentStores[row];
    auto res = QMessageBox::question(this, QStringLiteral("Confirm Deletion"),
                                     QStringLiteral("Are you sure you want to delete store '%1' (%2)?")
                                     .arg(selected.name, selected.code),
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (res == QMessageBox::Yes) {
        if (DatabaseManager::instance().deleteStore(selected.id)) {
            reloadStores();
        } else {
            QMessageBox::critical(this, QStringLiteral("Delete Failed"),
                                  QStringLiteral("Could not delete store."));
        }
    }
}

// =========================================================================
// Warehouses Logic
// =========================================================================

void AccessPage::onWarehousesSearchChanged() {
    reloadWarehouses();
}

void AccessPage::reloadWarehouses() {
    QString q = m_txtWarehouseSearch ? m_txtWarehouseSearch->text() : QString();
    m_currentWarehouses = DatabaseManager::instance().getWarehousesList(q);

    m_tableWarehouses->setRowCount(0);

    if (m_currentWarehouses.isEmpty()) {
        m_emptyWarehousesBanner->show();
        m_tableWarehouses->setRowCount(1);
        QTableWidgetItem *item = new QTableWidgetItem(
            q.isEmpty() ? QStringLiteral("⚠️ No warehouses configured. Click ➕ New Warehouse (F3 / Alt+1) to add a depot.")
                        : QStringLiteral("🔍 No warehouses matching '%1'.").arg(q));
        item->setTextAlignment(Qt::AlignCenter);
        QFont f = item->font();
        f.setBold(true);
        item->setFont(f);
        item->setForeground(QBrush(QColor(QStringLiteral("#827717"))));
        m_tableWarehouses->setItem(0, 1, item);
        m_lblWarehousesSummary->setText(QStringLiteral("Total Warehouses: 0 | Active Hubs: 0"));
        return;
    }

    m_emptyWarehousesBanner->hide();
    m_tableWarehouses->setRowCount(m_currentWarehouses.size());

    int activeCount = 0;
    for (int i = 0; i < m_currentWarehouses.size(); ++i) {
        const auto &wh = m_currentWarehouses[i];
        if (wh.isActive) activeCount++;

        QTableWidgetItem *itCode = new QTableWidgetItem(wh.code);
        itCode->setFont(QFont(itCode->font().family(), -1, QFont::Bold));
        itCode->setForeground(QBrush(QColor(QStringLiteral("#000080"))));

        QTableWidgetItem *itName = new QTableWidgetItem(wh.name);
        itName->setFont(QFont(itName->font().family(), -1, QFont::Bold));

        QTableWidgetItem *itPhone = new QTableWidgetItem(wh.phone);
        QTableWidgetItem *itEmail = new QTableWidgetItem(wh.email);
        QTableWidgetItem *itCity = new QTableWidgetItem(wh.city);
        QTableWidgetItem *itAddr = new QTableWidgetItem(wh.address);

        QTableWidgetItem *itStatus = new QTableWidgetItem(wh.isActive ? QStringLiteral("🟢 Active") : QStringLiteral("⚪ Inactive"));
        itStatus->setTextAlignment(Qt::AlignCenter);
        itStatus->setFont(QFont(itStatus->font().family(), -1, QFont::Bold));
        itStatus->setForeground(QBrush(QColor(wh.isActive ? QStringLiteral("#1B5E20") : QStringLiteral("#757575"))));

        m_tableWarehouses->setItem(i, 0, itCode);
        m_tableWarehouses->setItem(i, 1, itName);
        m_tableWarehouses->setItem(i, 2, itPhone);
        m_tableWarehouses->setItem(i, 3, itEmail);
        m_tableWarehouses->setItem(i, 4, itCity);
        m_tableWarehouses->setItem(i, 5, itAddr);
        m_tableWarehouses->setItem(i, 6, itStatus);
    }

    m_lblWarehousesSummary->setText(QStringLiteral("Total Registered Warehouses: %1  |  Active Hubs: %2")
                                   .arg(m_currentWarehouses.size()).arg(activeCount));
}

void AccessPage::openNewWarehouseDialog() {
    RetailNext::WarehouseRecord newWh;
    newWh.isActive = true;

    WarehouseDialog dlg(newWh, true, this);
    if (dlg.exec() == QDialog::Accepted) {
        RetailNext::WarehouseRecord toSave = dlg.getWarehouse();
        if (DatabaseManager::instance().saveWarehouse(toSave, true)) {
            reloadWarehouses();
            QMessageBox::information(this, QStringLiteral("Warehouse Created"),
                                     QStringLiteral("Warehouse '%1' registered successfully!").arg(toSave.name));
        } else {
            QMessageBox::critical(this, QStringLiteral("Database Error"),
                                  QStringLiteral("Failed to save warehouse: %1").arg(DatabaseManager::instance().lastError()));
        }
    }
}

void AccessPage::openEditWarehouseDialog() {
    int row = m_tableWarehouses->currentRow();
    if (row < 0 || row >= m_currentWarehouses.size()) {
        QMessageBox::warning(this, QStringLiteral("No Selection"),
                             QStringLiteral("Please select a warehouse to edit."));
        return;
    }

    const auto &selected = m_currentWarehouses[row];
    WarehouseDialog dlg(selected, false, this);
    if (dlg.exec() == QDialog::Accepted) {
        RetailNext::WarehouseRecord toSave = dlg.getWarehouse();
        if (DatabaseManager::instance().saveWarehouse(toSave, false)) {
            reloadWarehouses();
            QMessageBox::information(this, QStringLiteral("Warehouse Updated"),
                                     QStringLiteral("Warehouse '%1' updated successfully!").arg(toSave.name));
        } else {
            QMessageBox::critical(this, QStringLiteral("Database Error"),
                                  QStringLiteral("Failed to update warehouse: %1").arg(DatabaseManager::instance().lastError()));
        }
    }
}

void AccessPage::deleteSelectedWarehouse() {
    int row = m_tableWarehouses->currentRow();
    if (row < 0 || row >= m_currentWarehouses.size()) {
        QMessageBox::warning(this, QStringLiteral("No Selection"),
                             QStringLiteral("Please select a warehouse to delete."));
        return;
    }

    const auto &selected = m_currentWarehouses[row];
    auto res = QMessageBox::question(this, QStringLiteral("Confirm Deletion"),
                                     QStringLiteral("Are you sure you want to delete warehouse '%1' (%2)?")
                                     .arg(selected.name, selected.code),
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (res == QMessageBox::Yes) {
        if (DatabaseManager::instance().deleteWarehouse(selected.id)) {
            reloadWarehouses();
        } else {
            QMessageBox::critical(this, QStringLiteral("Delete Failed"),
                                  QStringLiteral("Could not delete warehouse."));
        }
    }
}

// =========================================================================
// Users Logic
// =========================================================================

void AccessPage::onUsersSearchChanged() {
    reloadUsers();
}

void AccessPage::reloadUsers() {
    QString q = m_txtUserSearch ? m_txtUserSearch->text() : QString();
    m_currentUsers = DatabaseManager::instance().getUsersList(q);

    m_tableUsers->setRowCount(0);

    if (m_currentUsers.isEmpty()) {
        m_emptyUsersBanner->show();
        m_tableUsers->setRowCount(1);
        QTableWidgetItem *item = new QTableWidgetItem(
            q.isEmpty() ? QStringLiteral("⚠️ No users found. Click ➕ New User (F3 / Alt+1) to add an operator login.")
                        : QStringLiteral("🔍 No users matching '%1'.").arg(q));
        item->setTextAlignment(Qt::AlignCenter);
        QFont f = item->font();
        f.setBold(true);
        item->setFont(f);
        item->setForeground(QBrush(QColor(QStringLiteral("#827717"))));
        m_tableUsers->setItem(0, 1, item);
        m_lblUsersSummary->setText(QStringLiteral("Total Users: 0 | Active Logins: 0"));
        return;
    }

    m_emptyUsersBanner->hide();
    m_tableUsers->setRowCount(m_currentUsers.size());

    int activeCount = 0;
    for (int i = 0; i < m_currentUsers.size(); ++i) {
        const auto &u = m_currentUsers[i];
        if (u.isActive) activeCount++;

        QTableWidgetItem *itUser = new QTableWidgetItem(u.username);
        itUser->setFont(QFont(itUser->font().family(), -1, QFont::Bold));
        itUser->setForeground(QBrush(QColor(QStringLiteral("#000080"))));

        QTableWidgetItem *itName = new QTableWidgetItem(u.displayName);
        itName->setFont(QFont(itName->font().family(), -1, QFont::Bold));

        QTableWidgetItem *itPhone = new QTableWidgetItem(u.phone.isEmpty() ? QStringLiteral("-") : u.phone);

        QTableWidgetItem *itRole = new QTableWidgetItem(u.role);
        itRole->setFont(QFont(itRole->font().family(), -1, QFont::Bold));
        if (u.role == "Store Admin") itRole->setForeground(QBrush(QColor(QStringLiteral("#8E24AA"))));
        else if (u.role == "Manager") itRole->setForeground(QBrush(QColor(QStringLiteral("#0055A5"))));
        else itRole->setForeground(QBrush(QColor(QStringLiteral("#1B5E20"))));

        // Format permissions nicely
        QString readablePerms;
        if (u.permissions.isEmpty()) {
            readablePerms = QStringLiteral("Default Standard Access");
        } else {
            QStringList parts;
            for (const QString &item : u.permissions.split(';', Qt::SkipEmptyParts)) {
                QStringList t = item.split(':');
                if (t.size() == 2) {
                    parts << QStringLiteral("%1 (%2)").arg(t[0], t[1]);
                }
            }
            readablePerms = parts.join(QStringLiteral(" • "));
        }
        QTableWidgetItem *itPerm = new QTableWidgetItem(readablePerms);

        QTableWidgetItem *itStatus = new QTableWidgetItem(u.isActive ? QStringLiteral("🟢 Active") : QStringLiteral("⚪ Suspended"));
        itStatus->setTextAlignment(Qt::AlignCenter);
        itStatus->setFont(QFont(itStatus->font().family(), -1, QFont::Bold));
        itStatus->setForeground(QBrush(QColor(u.isActive ? QStringLiteral("#1B5E20") : QStringLiteral("#B71C1C"))));

        QTableWidgetItem *itDate = new QTableWidgetItem(u.createdAt);

        m_tableUsers->setItem(i, 0, itUser);
        m_tableUsers->setItem(i, 1, itName);
        m_tableUsers->setItem(i, 2, itPhone);
        m_tableUsers->setItem(i, 3, itRole);
        m_tableUsers->setItem(i, 4, itPerm);
        m_tableUsers->setItem(i, 5, itStatus);
        m_tableUsers->setItem(i, 6, itDate);
    }

    m_lblUsersSummary->setText(QStringLiteral("Total Registered Users: %1  |  Active Accounts: %2")
                              .arg(m_currentUsers.size()).arg(activeCount));
}

void AccessPage::openNewUserDialog() {
    RetailNext::UserRecord newUser;
    newUser.isActive = true;
    newUser.role = QStringLiteral("Cashier");

    UserDialog dlg(newUser, true, this);
    if (dlg.exec() == QDialog::Accepted) {
        RetailNext::UserRecord toSave = dlg.getUser();
        if (DatabaseManager::instance().saveUser(toSave, true)) {
            reloadUsers();
            QMessageBox::information(this, QStringLiteral("User Account Created"),
                                     QStringLiteral("User '%1' (%2) created successfully!").arg(toSave.displayName, toSave.username));
        } else {
            QMessageBox::critical(this, QStringLiteral("Database Error"),
                                  QStringLiteral("Failed to create user account: %1").arg(DatabaseManager::instance().lastError()));
        }
    }
}

void AccessPage::openEditUserDialog() {
    int row = m_tableUsers->currentRow();
    if (row < 0 || row >= m_currentUsers.size()) {
        QMessageBox::warning(this, QStringLiteral("No Selection"),
                             QStringLiteral("Please select a user account to edit."));
        return;
    }

    const auto &selected = m_currentUsers[row];
    UserDialog dlg(selected, false, this);
    if (dlg.exec() == QDialog::Accepted) {
        RetailNext::UserRecord toSave = dlg.getUser();
        if (DatabaseManager::instance().saveUser(toSave, false)) {
            reloadUsers();
            QMessageBox::information(this, QStringLiteral("User Account Updated"),
                                     QStringLiteral("User '%1' updated successfully!").arg(toSave.displayName));
        } else {
            QMessageBox::critical(this, QStringLiteral("Database Error"),
                                  QStringLiteral("Failed to update user account: %1").arg(DatabaseManager::instance().lastError()));
        }
    }
}

void AccessPage::deleteSelectedUser() {
    int row = m_tableUsers->currentRow();
    if (row < 0 || row >= m_currentUsers.size()) {
        QMessageBox::warning(this, QStringLiteral("No Selection"),
                             QStringLiteral("Please select a user account to delete."));
        return;
    }

    const auto &selected = m_currentUsers[row];
    if (selected.username == "admin") {
        QMessageBox::warning(this, QStringLiteral("Action Prohibited"),
                             QStringLiteral("The master 'admin' account cannot be deleted."));
        return;
    }

    auto res = QMessageBox::question(this, QStringLiteral("Confirm Deletion"),
                                     QStringLiteral("Are you sure you want to delete user account '%1' (%2)?")
                                     .arg(selected.displayName, selected.username),
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (res == QMessageBox::Yes) {
        if (DatabaseManager::instance().deleteUser(selected.id)) {
            reloadUsers();
        } else {
            QMessageBox::critical(this, QStringLiteral("Delete Failed"),
                                  QStringLiteral("Could not delete user account."));
        }
    }
}

