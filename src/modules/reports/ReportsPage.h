#ifndef REPORTSPAGE_H
#define REPORTSPAGE_H

#include <QWidget>
#include "../../database/DatabaseManager.h"

class QTabWidget;
class QTableWidget;
class QComboBox;
class QLabel;

class ReportsPage : public QWidget {
    Q_OBJECT

public:
    explicit ReportsPage(QWidget *parent = nullptr);

public slots:
    void reloadData();
    void onExportClicked();
    void onPrintClicked();
    void switchToTab(int tabIndex);

private slots:
    void onPeriodChanged();

private:
    void setupUI();
    void loadSalesReport();
    void loadProfitReport();
    void loadStockValuationReport();
    void loadTaxReport();

    QComboBox *m_comboPeriod;
    QTabWidget *m_tabs;

    QTableWidget *m_tableSales;
    QLabel *m_lblSalesSummary;

    QTableWidget *m_tableProfit;
    QLabel *m_lblProfitSummary;

    QTableWidget *m_tableStock;
    QLabel *m_lblStockSummary;

    QTableWidget *m_tableTax;
    QLabel *m_lblTaxSummary;
};

#endif // REPORTSPAGE_H

