#ifndef PRODUCTDIALOG_H
#define PRODUCTDIALOG_H

#include <QDialog>
#include "../../database/DatabaseManager.h"

class QLineEdit;
class QComboBox;
class QDoubleSpinBox;
class QCheckBox;
class QTextEdit;

class ProductDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProductDialog(const RetailNext::ProductItem& product, bool isNew = false, QWidget *parent = nullptr);
    RetailNext::ProductItem getProduct() const { return m_product; }

private slots:
    void onSaveClicked();
    void updateMarginCalculation();

private:
    void setupTabs();

    RetailNext::ProductItem m_product;
    bool m_isNew;

    // Tab 1: Basic
    QLineEdit *m_txtName;
    QLineEdit *m_txtSku;
    QComboBox *m_comboCategory;
    QComboBox *m_comboBrand;
    QCheckBox *m_chkActive;
    QTextEdit *m_txtDescription;

    // Tab 2: Identification
    QLineEdit *m_txtBarcode;
    QLineEdit *m_txtAltBarcode;

    // Tab 3: Pricing & Tax
    QDoubleSpinBox *m_spinCostPrice;
    QDoubleSpinBox *m_spinSellingPrice;
    QDoubleSpinBox *m_spinMRP;
    QDoubleSpinBox *m_spinWholesalePrice;
    QDoubleSpinBox *m_spinTaxRate;
    QLineEdit *m_lblMargin;

    // Tab 4: Inventory & Units
    QComboBox *m_comboUnit;
    QDoubleSpinBox *m_spinStock;
    QDoubleSpinBox *m_spinReorder;

    // Tab 5: Variants
    QLineEdit *m_txtVariantSize;
    QLineEdit *m_txtVariantColor;
    QLineEdit *m_txtVariantSku;
};

#endif // PRODUCTDIALOG_H

