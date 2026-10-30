#ifndef QUICKCUSTOMERDIALOG_H
#define QUICKCUSTOMERDIALOG_H

#include <QDialog>
#include "../database/DatabaseManager.h"

class QLineEdit;
class QComboBox;

class QuickCustomerDialog : public QDialog {
    Q_OBJECT

public:
    explicit QuickCustomerDialog(QWidget *parent = nullptr);
    RetailNext::CustomerRecord createdCustomer() const { return m_customer; }

private slots:
    void onSaveClicked();

private:
    QLineEdit *m_nameEdit;
    QLineEdit *m_phoneEdit;
    QLineEdit *m_emailEdit;
    QComboBox *m_groupCombo;
    RetailNext::CustomerRecord m_customer;
};

#endif // QUICKCUSTOMERDIALOG_H

