#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QString>
#include <QStringList>

namespace Ui {
class MainWindow;
}

class QLabel;
class QLineEdit;
class QTabWidget;
class QFrame;
class QHBoxLayout;
class QVBoxLayout;
class QPushButton;
class QTimer;
class QMenu;

class DashboardPage;
class POSBillingPage;
class ProductsPage;
class InventoryPage;
class PurchasesPage;
class SalesPage;
class CustomersPage;
class FinancePage;
class ReportsPage;
class SettingsPage;
class AccessPage;
class PlaceholderPage;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

public slots:
  void openModule(int moduleIndex);
  void selectCategorySubmenu(const QString &category, const QString &submenu);

private slots:
  void onTabCloseRequested(int index);
  void onGlobalSearchActivated();

  // Menu and toolbar action handlers
  void onActionNewTransaction();
  void onActionSave();
  void onActionPrint();
  void onActionExport();
  void onActionBackupDatabase();
  void onActionRestoreBackup();
  void onActionSwitchBranch();
  void onActionLockApplication();
  void onActionExit();

  void onActionStockAdjustment();
  void onActionStockTransfer();
  void onActionCashInOut();
  void onActionDayClosing();
  void onActionDbMaintenance();
  void onActionPrinterDiagnostics();
  void onActionSystemLogs();
  void onActionAbout();
  void onActionKeyboardShortcuts();
  void onActionDocumentation();
  void onActionCalculator();

  // Window menu actions
  void onActionNextTab();
  void onActionPreviousTab();
  void onActionCloseCurrentTab();
  void onActionCloseAllTabs();
  void onActionResetWorkspace();

  // Secondary toolbar option dispatcher
  void onSecondaryActionClicked(const QString &action);

private:
  void setupNativeRibbon();
  void setupCommandMenuBar(QWidget *parent, QVBoxLayout *rootLayout);
  void setupActionToolbar(QWidget *parent, QVBoxLayout *rootLayout);
  void updateSecondaryToolbar(const QString &category, const QString &submenu);
  QPushButton* createToolbarButton(const QString &text, const QString &tooltip);

  void setupWorkspaceTabs();
  void setupGlobalKeyboardShortcuts();
  void setupStatusBar();
  void updateStatusBarIndicators();

  Ui::MainWindow *ui;

  // Header Bars (Matching Reference Image)
  QFrame *m_topMenuBar;
  QFrame *m_topToolbar;
  QHBoxLayout *m_secondaryActionsLayout;
  QPushButton *m_btnSubmenuPicker;

  // Active Category & Submenu tracking
  QString m_currentCategory;
  QString m_currentSubmenu;
  QMap<QString, QPushButton*> m_menuButtons;

  // Workspace Tab Widget container (full width, tabs hidden)
  QTabWidget *m_workspaceTabs;

  // Module Pages
  DashboardPage *m_dashboardPage;
  POSBillingPage *m_posPage;
  ProductsPage *m_productsPage;
  InventoryPage *m_inventoryPage;
  PurchasesPage *m_purchasesPage;
  SalesPage *m_salesPage;
  CustomersPage *m_customersPage;
  FinancePage *m_financePage;
  ReportsPage *m_reportsPage;
  SettingsPage *m_settingsPage;
  AccessPage *m_accessPage;
  PlaceholderPage *m_genericPlaceholderPage;

  // Status bar permanent widgets
  QLabel *m_statusReadyLabel;
  QLabel *m_statusUserLabel;
  QLabel *m_statusBranchLabel;
  QLabel *m_statusDbLabel;
  QLabel *m_statusClockLabel;
  QTimer *m_clockTimer;
};

#endif // MAINWINDOW_H
