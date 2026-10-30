#ifndef FINANCEPAGE_H
#define FINANCEPAGE_H

#include <QWidget>
#include "../../database/DatabaseManager.h"

class QTabWidget;
class QTableWidget;
class QLabel;

class FinancePage : public QWidget {
    Q_OBJECT

public:
    explicit FinancePage(QWidget *parent = nullptr);

public slots:
    void reloadData();
    void onCashInClicked();
    void onCashOutClicked();
    void onCloseRegisterClicked();
    void onAddExpenseClicked();
    void onExportCsvClicked();
    void switchToTab(int tabIndex);

private:
    void setupUI();
    void loadRegisterSession();
    void loadExpenses();

    QTabWidget *m_tabs;

    // Register Session widgets
    QLabel *m_lblRegCashier;
    QLabel *m_lblRegOpened;
    QLabel *m_lblRegOpeningFloat;
    QLabel *m_lblRegCashSales;
    QLabel *m_lblRegCashIn;
    QLabel *m_lblRegCashOut;
    QLabel *m_lblRegExpected;

    // Expenses
    QTableWidget *m_tableExpenses;
    QLabel *m_lblExpensesSummary;
};

#endif // FINANCEPAGE_H

