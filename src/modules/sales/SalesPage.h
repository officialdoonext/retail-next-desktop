#ifndef SALESPAGE_H
#define SALESPAGE_H

#include <QWidget>
#include "../../database/DatabaseManager.h"

class QTabWidget;
class QTableWidget;
class QLineEdit;
class QLabel;

class SalesPage : public QWidget {
    Q_OBJECT

public:
    explicit SalesPage(QWidget *parent = nullptr);

public slots:
    void reloadData();
    void onViewInvoiceDetailsClicked();
    void onProcessReturnClicked();
    void onExportCsvClicked();
    void switchToTab(int tabIndex);

private slots:
    void onSearchChanged();

private:
    void setupUI();
    void loadInvoices();
    void loadHeldBills();

    QTabWidget *m_tabs;
    QLineEdit *m_txtSearch;
    QTableWidget *m_tableInvoices;
    QTableWidget *m_tableHeld;
    QLabel *m_lblInvoicesSummary;

    QList<RetailNext::SaleInvoiceSummary> m_currentInvoices;
};

#endif // SALESPAGE_H

