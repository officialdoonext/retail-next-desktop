#ifndef PURCHASESPAGE_H
#define PURCHASESPAGE_H

#include <QWidget>
#include "../../database/DatabaseManager.h"

class QTabWidget;
class QTableWidget;
class QLabel;

class PurchasesPage : public QWidget {
    Q_OBJECT

public:
    explicit PurchasesPage(QWidget *parent = nullptr);

public slots:
    void reloadData();
    void onNewInwardReceiptClicked();
    void onNewSupplierClicked();
    void onRecordPaymentClicked();
    void onExportCsvClicked();
    void switchToTab(int tabIndex);

private:
    void setupUI();
    void loadInvoices();
    void loadSuppliers();

    QTabWidget *m_tabs;
    QTableWidget *m_tableInvoices;
    QTableWidget *m_tableSuppliers;
    QLabel *m_lblInvoicesSummary;
    QLabel *m_lblSuppliersSummary;
};

#endif // PURCHASESPAGE_H

