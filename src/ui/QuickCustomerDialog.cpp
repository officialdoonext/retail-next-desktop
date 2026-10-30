#include "QuickCustomerDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDateTime>

QuickCustomerDialog::QuickCustomerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Quick Customer Registration (POS)"));
    resize(380, 240);

    setStyleSheet(QStringLiteral(
        "QDialog { background-color: #D9E8F5; } "
        "QLabel { color: #0A294A; font-weight: bold; font-size: 8.5pt; } "
        "QLineEdit, QComboBox { background: #FFFFFF; border: 1px solid #7A9EB9; "
        "border-radius: 2px; padding: 3px 6px; color: #000080; font-weight: bold; font-size: 9pt; } "
        "QLineEdit:focus, QComboBox:focus { border: 1px solid #0055A5; background-color: #FFFFF2; }"
    ));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(6);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(QStringLiteral("Customer Full Name"));

    m_phoneEdit = new QLineEdit(this);
    m_phoneEdit->setPlaceholderText(QStringLiteral("Mobile / Phone number"));

    m_emailEdit = new QLineEdit(this);
    m_emailEdit->setPlaceholderText(QStringLiteral("Email (optional)"));

    m_groupCombo = new QComboBox(this);
    m_groupCombo->addItems(QStringList() << QStringLiteral("Retail") << QStringLiteral("VIP") << QStringLiteral("Wholesale"));

    formLayout->addRow(QStringLiteral("Customer Name: *"), m_nameEdit);
    formLayout->addRow(QStringLiteral("Phone Number: *"), m_phoneEdit);
    formLayout->addRow(QStringLiteral("Email:"), m_emailEdit);
    formLayout->addRow(QStringLiteral("Customer Group:"), m_groupCombo);
    mainLayout->addLayout(formLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    QPushButton *btnSave = new QPushButton(QStringLiteral("✅ OK (F9)"), this);
    btnSave->setShortcut(QKeySequence(Qt::Key_F9));
    btnSave->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 4px 16px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #C8E6C9; }"
    ));
    QPushButton *btnCancel = new QPushButton(QStringLiteral("❌ Cancel (Esc)"), this);
    btnCancel->setShortcut(QKeySequence(Qt::Key_Escape));
    btnCancel->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 4px 16px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; }"
    ));
    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnCancel);
    mainLayout->addLayout(btnLayout);

    connect(btnSave, &QPushButton::clicked, this, &QuickCustomerDialog::onSaveClicked);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void QuickCustomerDialog::onSaveClicked() {
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Validation Error"), QStringLiteral("Please enter a customer name."));
        m_nameEdit->setFocus();
        return;
    }

    m_customer.code = QStringLiteral("CUST-%1").arg(QDateTime::currentMSecsSinceEpoch() % 100000);
    m_customer.name = m_nameEdit->text().trimmed();
    m_customer.phone = m_phoneEdit->text().trimmed();
    m_customer.email = m_emailEdit->text().trimmed();
    m_customer.customerGroup = m_groupCombo->currentText();
    m_customer.loyaltyPoints = 10; // Welcome points!
    m_customer.storeCredit = 0.0;
    m_customer.outstandingBalance = 0.0;
    m_customer.creditLimit = 200.0;
    m_customer.isActive = true;

    if (DatabaseManager::instance().saveCustomer(m_customer, true)) {
        accept();
    } else {
        QMessageBox::critical(this, QStringLiteral("Database Error"), QStringLiteral("Failed to save customer."));
    }
}

