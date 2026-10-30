#ifndef POSBILLINGPAGE_H
#define POSBILLINGPAGE_H

#include <QWidget>
#include <QList>
#include <QDate>
#include "../../database/DatabaseManager.h"

class QLineEdit;
class QComboBox;
class QTableWidget;
class QLabel;
class QPushButton;
class QDoubleSpinBox;
class QDateEdit;
class QTabWidget;

class POSBillingPage : public QWidget {
    Q_OBJECT

public:
    explicit POSBillingPage(QWidget *parent = nullptr);
    void resetBillingSession();

public slots:
    void reloadData();
    void focusBarcodeField();
    void onBarcodeScanned();
    void onProductLookupChanged(int index);
    void onAddItemClicked();
    void onItemCellChanged(int row, int col);
    void onRemoveItemClicked(int row);
    void onDiscountChanged();
    void onTenderedChanged();
    void onCheckoutClicked(const QString& paymentMode = QStringLiteral("Cash"));
    void onHoldBillClicked();
    void onRecallBillClicked();
    void onVoidBillClicked();
    void onQuickCustomerClicked();
    void onCartRowSelected(int row, int col);

signals:
    void navigateToModule(int moduleIndex);

private:
    void setupUI();
    void updateTotals();
    void populateCustomerCombo();
    void populateProductLookup();
    void addProductToCart(const RetailNext::ProductItem& product, double qty = 1.0);
    void updateBottomStockDetails(const RetailNext::ProductItem &product);

    // Header Controls: Left Column (Bill & Customer Details)
    QComboBox *m_comboBillPrefix;
    QLabel *m_lblBillNo;
    QComboBox *m_comboBillType;
    QDateEdit *m_dateBill;
    QLabel *m_lblGstTerms;
    QLineEdit *m_txtCustCode;
    QPushButton *m_btnBrowseCustomer;
    QLineEdit *m_txtCustPhone;
    QLineEdit *m_txtCustName;
    QLineEdit *m_txtCustAddress;
    QLineEdit *m_txtCustStreet;
    QLineEdit *m_txtCustPlace;
    QLineEdit *m_txtRepName;

    // Header Controls: Middle Column (Additional Details)
    QLineEdit *m_txtDoctor;
    QLineEdit *m_txtPrescription;
    QLineEdit *m_txtPatient;
    QLineEdit *m_txtAgeSex;
    QLineEdit *m_txtRemarks;

    // Header Controls: Right Column (Financial Grid)
    QLabel *m_lblGoodsValue;
    QDoubleSpinBox *m_spinCashDisc;
    QLabel *m_lblCashDiscAmt;
    QDoubleSpinBox *m_spinSplDisc;
    QLabel *m_lblSplDiscAmt;
    QLabel *m_lblTaxAmt;
    QDoubleSpinBox *m_spinFreight;
    QLabel *m_lblTotalBalance;
    QLabel *m_lblBigBillAmount;

    // Summary Strip & Item Entry
    QLabel *m_lblItemCount;
    QLabel *m_lblTotalQty;
    QLineEdit *m_txtBarcode;
    QComboBox *m_comboProductLookup;
    QDoubleSpinBox *m_spinQty;
    QPushButton *m_btnAddItem;

    // Cart Grid Table
    QTableWidget *m_tableCart;

    // Bottom Detail Tabs
    QTabWidget *m_detailTabs;
    QTableWidget *m_tableStockDetails;

    // Function Key Bar Buttons
    QPushButton *m_btnF1;
    QPushButton *m_btnF2;
    QPushButton *m_btnF3;
    QPushButton *m_btnF4;
    QPushButton *m_btnF5;
    QPushButton *m_btnF6;
    QPushButton *m_btnF7;
    QPushButton *m_btnF8;
    QPushButton *m_btnF9;
    QPushButton *m_btnF10;
    QPushButton *m_btnEsc;

    // State
    QList<RetailNext::CartItem> m_cart;
    bool m_updatingCart;
};

#endif // POSBILLINGPAGE_H
