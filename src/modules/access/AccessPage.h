#ifndef ACCESSPAGE_H
#define ACCESSPAGE_H

#include <QWidget>
#include <QList>
#include "../../database/DatabaseManager.h"

class QTabWidget;
class QTableWidget;
class QLineEdit;
class QLabel;
class QFrame;
class QPushButton;

class AccessPage : public QWidget {
    Q_OBJECT

public:
    explicit AccessPage(QWidget *parent = nullptr);
    ~AccessPage() override = default;

public slots:
    void switchToSubTab(int tabIndex);
    void refreshCurrentTab();

    // Stores actions
    void openNewStoreDialog();
    void openEditStoreDialog();
    void deleteSelectedStore();

    // Warehouses actions
    void openNewWarehouseDialog();
    void openEditWarehouseDialog();
    void deleteSelectedWarehouse();

    // Users actions
    void openNewUserDialog();
    void openEditUserDialog();
    void deleteSelectedUser();

private slots:
    void onStoresSearchChanged();
    void onWarehousesSearchChanged();
    void onUsersSearchChanged();
    void onTabChanged(int index);

private:
    void setupUI();
    QWidget* createStoresTab();
    QWidget* createWarehousesTab();
    QWidget* createUsersTab();
    QFrame* createBottomFunctionBar();

    void reloadStores();
    void reloadWarehouses();
    void reloadUsers();

    // UI elements
    QTabWidget *m_subTabs;

    // Stores Tab
    QLineEdit *m_txtStoreSearch;
    QTableWidget *m_tableStores;
    QFrame *m_emptyStoresBanner;
    QLabel *m_lblStoresSummary;
    QList<RetailNext::StoreRecord> m_currentStores;

    // Warehouses Tab
    QLineEdit *m_txtWarehouseSearch;
    QTableWidget *m_tableWarehouses;
    QFrame *m_emptyWarehousesBanner;
    QLabel *m_lblWarehousesSummary;
    QList<RetailNext::WarehouseRecord> m_currentWarehouses;

    // Users Tab
    QLineEdit *m_txtUserSearch;
    QTableWidget *m_tableUsers;
    QFrame *m_emptyUsersBanner;
    QLabel *m_lblUsersSummary;
    QList<RetailNext::UserRecord> m_currentUsers;
};

#endif // ACCESSPAGE_H

