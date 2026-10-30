#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include <QWidget>

namespace Ui {
class DashboardPage;
}

class DashboardPage : public QWidget {
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage();

    void refreshData();

signals:
    void openPOSRequested();
    void openProductsRequested();
    void openStockRequested();
    void openReportsRequested();
    void openPurchasesRequested();
    void openFinanceRequested();

private slots:
    void on_btnQuickPOS_clicked();
    void on_btnAddProduct_clicked();
    void on_btnStockInward_clicked();
    void on_btnDailyReport_clicked();

private:
    void setupRecentTable();
    void updateKPICards();

    Ui::DashboardPage *ui;
};

#endif // DASHBOARDPAGE_H
