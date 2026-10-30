#ifndef CUSTOMERSPAGE_H
#define CUSTOMERSPAGE_H

#include <QWidget>
#include "../../database/DatabaseManager.h"

class QLineEdit;
class QComboBox;
class QTableWidget;
class QLabel;

class CustomersPage : public QWidget {
    Q_OBJECT

public:
    explicit CustomersPage(QWidget *parent = nullptr);

public slots:
    void reloadData();
    void onAddCustomerClicked();
    void onEditCustomerClicked();
    void onRecordPaymentClicked();
    void onExportCsvClicked();

private slots:
    void onSearchChanged();

private:
    void setupUI();

    QLineEdit *m_txtSearch;
    QComboBox *m_comboGroup;
    QTableWidget *m_tableCustomers;
    QLabel *m_lblSummary;

    QList<RetailNext::CustomerRecord> m_currentCustomers;
};

#endif // CUSTOMERSPAGE_H

