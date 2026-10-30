#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include "../../database/DatabaseManager.h"

class QTabWidget;
class QLineEdit;
class QComboBox;
class QTextEdit;
class QCheckBox;
class QLabel;

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);

public slots:
    void reloadData();
    void onBackupNowClicked();
    void onRestoreBackupClicked();
    void onRunIntegrityCheckClicked();
    void switchToTab(int tabIndex);

private slots:
    void onSaveProfileClicked();
    void onIndustryTypeChanged(int index);

private:
    void setupUI();

    QTabWidget *m_tabs;

    // Tab 1: Business Profile
    QLineEdit *m_txtBusinessName;
    QLineEdit *m_txtBranchName;
    QLineEdit *m_txtRegisterCode;
    QLineEdit *m_txtCurrencySymbol;
    QComboBox *m_comboTaxMode;
    QTextEdit *m_txtReceiptHeader;
    QTextEdit *m_txtReceiptFooter;

    // Tab 2: Industry Profiles
    QComboBox *m_comboIndustry;
    QCheckBox *m_chkVariants;
    QCheckBox *m_chkDecimalQty;
    QCheckBox *m_chkSerialNumbers;
    QCheckBox *m_chkBatchExpiry;
    QCheckBox *m_chkNegativeStock;
    QCheckBox *m_chkManagerDiscountPin;

    // Tab 3: Backup & Database
    QLabel *m_lblDbPath;
    QLabel *m_lblDbSize;
    QLabel *m_lblIntegrityStatus;
};

#endif // SETTINGSPAGE_H

