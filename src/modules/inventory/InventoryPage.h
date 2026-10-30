#ifndef INVENTORYPAGE_H
#define INVENTORYPAGE_H

#include <QWidget>
#include "../../database/DatabaseManager.h"

class QTabWidget;
class QTableWidget;
class QLineEdit;
class QComboBox;
class QLabel;

class InventoryPage : public QWidget {
    Q_OBJECT

public:
    explicit InventoryPage(QWidget *parent = nullptr);

public slots:
    void reloadData();
    void onStockAdjustmentClicked();
    void onStockTransferClicked();
    void onExportLedgerClicked();
    void switchToTab(int tabIndex);

private:
    void setupUI();
    void loadOverview();
    void loadLedger();
    void loadReorderList();

    QTabWidget *m_tabs;
    QTableWidget *m_tableOverview;
    QTableWidget *m_tableLedger;
    QTableWidget *m_tableReorder;

    QLabel *m_lblOverviewStats;
    QLabel *m_lblLedgerStats;
    QLabel *m_lblReorderStats;
};

#endif // INVENTORYPAGE_H

