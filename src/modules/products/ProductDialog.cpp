#include "ProductDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

ProductDialog::ProductDialog(const RetailNext::ProductItem& product, bool isNew, QWidget *parent)
    : QDialog(parent)
    , m_product(product)
    , m_isNew(isNew)
    , m_txtName(nullptr)
    , m_txtSku(nullptr)
    , m_comboCategory(nullptr)
    , m_comboBrand(nullptr)
    , m_chkActive(nullptr)
    , m_txtDescription(nullptr)
    , m_txtBarcode(nullptr)
    , m_txtAltBarcode(nullptr)
    , m_spinCostPrice(nullptr)
    , m_spinSellingPrice(nullptr)
    , m_spinMRP(nullptr)
    , m_spinWholesalePrice(nullptr)
    , m_spinTaxRate(nullptr)
    , m_lblMargin(nullptr)
    , m_comboUnit(nullptr)
    , m_spinStock(nullptr)
    , m_spinReorder(nullptr)
    , m_txtVariantSize(nullptr)
    , m_txtVariantColor(nullptr)
    , m_txtVariantSku(nullptr)
{
    setWindowTitle(isNew ? QStringLiteral("Create New Product - Item Master") : QStringLiteral("Edit Product - %1").arg(product.name));
    resize(640, 500);

    setStyleSheet(QStringLiteral(
        "QDialog { background-color: #D9E8F5; } "
        "QLabel { color: #0A294A; font-weight: bold; font-size: 8.5pt; } "
        "QLineEdit, QComboBox, QDoubleSpinBox, QTextEdit { background: #FFFFFF; border: 1px solid #7A9EB9; "
        "border-radius: 2px; padding: 3px 6px; color: #000080; font-weight: bold; font-size: 9pt; } "
        "QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus, QTextEdit:focus { border: 1px solid #0055A5; background-color: #FFFFF2; } "
        "QCheckBox { color: #0A294A; font-weight: bold; font-size: 8.5pt; }"
    ));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    setupTabs();

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton *btnSave = new QPushButton(QStringLiteral("✅ OK (F9)"), this);
    btnSave->setShortcut(QKeySequence(Qt::Key_F9));
    btnSave->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 4px 16px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #C8E6C9; }"
    ));

    QPushButton *btnCancel = new QPushButton(QStringLiteral("❌ Cancel (Esc)"), this);
    btnCancel->setShortcut(QKeySequence(Qt::Key_Escape));
    btnCancel->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 4px 16px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; }"
    ));

    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnCancel);
    mainLayout->addLayout(btnLayout);

    connect(btnSave, &QPushButton::clicked, this, &ProductDialog::onSaveClicked);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    updateMarginCalculation();
}

void ProductDialog::setupTabs() {
    QTabWidget *tabs = new QTabWidget(this);
    tabs->setStyleSheet(QStringLiteral(
        "QTabWidget::pane { border: 1px solid #8FAEC7; background-color: #D9E8F5; top: -1px; } "
        "QTabBar::tab { background-color: #D0E2F2; color: #0A294A; border: 1px solid #8FAEC7; border-bottom: none; "
        "border-top-left-radius: 3px; border-top-right-radius: 3px; padding: 3px 12px; font-size: 8.5pt; font-weight: 600; margin-right: 2px; } "
        "QTabBar::tab:selected { background-color: #FFFFFF; color: #000080; border-bottom: 2px solid #0055A5; font-weight: bold; }"
    ));

    DatabaseManager &db = DatabaseManager::instance();
    QStringList categories = db.getCategoriesList();
    QStringList brands = db.getBrandsList();
    QStringList units = db.getUnitsList();

    // Tab 1: Basic Information
    QWidget *tabBasic = new QWidget(tabs);
    QFormLayout *f1 = new QFormLayout(tabBasic);
    f1->setContentsMargins(10, 10, 10, 10);
    f1->setSpacing(6);

    m_txtName = new QLineEdit(m_product.name, tabBasic);
    m_txtSku = new QLineEdit(m_product.sku, tabBasic);
    m_comboCategory = new QComboBox(tabBasic);
    m_comboCategory->addItems(categories);
    int catIdx = m_comboCategory->findText(m_product.categoryName);
    if (catIdx >= 0) m_comboCategory->setCurrentIndex(catIdx);

    m_comboBrand = new QComboBox(tabBasic);
    m_comboBrand->addItems(brands);
    int brandIdx = m_comboBrand->findText(m_product.brandName);
    if (brandIdx >= 0) m_comboBrand->setCurrentIndex(brandIdx);

    m_chkActive = new QCheckBox(QStringLiteral("Item is Active & Available for Sale"), tabBasic);
    m_chkActive->setChecked(m_product.isActive);

    m_txtDescription = new QTextEdit(tabBasic);
    m_txtDescription->setMaximumHeight(70);
    m_txtDescription->setPlaceholderText(QStringLiteral("Optional product description, manufacturer notes, or specifications..."));

    f1->addRow(QStringLiteral("Product Name: *"), m_txtName);
    f1->addRow(QStringLiteral("Item SKU Code: *"), m_txtSku);
    f1->addRow(QStringLiteral("Category:"), m_comboCategory);
    f1->addRow(QStringLiteral("Brand:"), m_comboBrand);
    f1->addRow(QStringLiteral("Status:"), m_chkActive);
    f1->addRow(QStringLiteral("Description:"), m_txtDescription);
    tabs->addTab(tabBasic, QStringLiteral("📦 1. Basic Info"));

    // Tab 2: Identification & Barcodes
    QWidget *tabId = new QWidget(tabs);
    QFormLayout *f2 = new QFormLayout(tabId);
    f2->setContentsMargins(10, 10, 10, 10);
    f2->setSpacing(6);

    m_txtBarcode = new QLineEdit(m_product.barcode, tabId);
    m_txtBarcode->setPlaceholderText(QStringLiteral("EAN-13, UPC, Code 128 (e.g. 890103000001)"));
    m_txtAltBarcode = new QLineEdit(tabId);
    m_txtAltBarcode->setPlaceholderText(QStringLiteral("Alternate supplier code or secondary barcode"));

    f2->addRow(QStringLiteral("Primary Barcode: *"), m_txtBarcode);
    f2->addRow(QStringLiteral("Alternate Barcode:"), m_txtAltBarcode);
    tabs->addTab(tabId, QStringLiteral("🏷️ 2. Identification"));

    // Tab 3: Pricing & Tax
    QWidget *tabPrice = new QWidget(tabs);
    QFormLayout *f3 = new QFormLayout(tabPrice);
    f3->setContentsMargins(10, 10, 10, 10);
    f3->setSpacing(6);

    m_spinCostPrice = new QDoubleSpinBox(tabPrice);
    m_spinCostPrice->setRange(0.0, 100000.0);
    m_spinCostPrice->setValue(m_product.costPrice);

    m_spinSellingPrice = new QDoubleSpinBox(tabPrice);
    m_spinSellingPrice->setRange(0.0, 100000.0);
    m_spinSellingPrice->setValue(m_product.sellingPrice);

    m_spinMRP = new QDoubleSpinBox(tabPrice);
    m_spinMRP->setRange(0.0, 100000.0);
    m_spinMRP->setValue(m_product.mrp);

    m_spinWholesalePrice = new QDoubleSpinBox(tabPrice);
    m_spinWholesalePrice->setRange(0.0, 100000.0);
    m_spinWholesalePrice->setValue(m_product.wholesalePrice);

    m_spinTaxRate = new QDoubleSpinBox(tabPrice);
    m_spinTaxRate->setRange(0.0, 100.0);
    m_spinTaxRate->setValue(m_product.taxRate);

    m_lblMargin = new QLineEdit(tabPrice);
    m_lblMargin->setReadOnly(true);
    m_lblMargin->setStyleSheet(QStringLiteral("background: #E8F5E9; font-weight: bold; color: #1B5E20;"));

    f3->addRow(QStringLiteral("Cost Price:"), m_spinCostPrice);
    f3->addRow(QStringLiteral("Selling Price: *"), m_spinSellingPrice);
    f3->addRow(QStringLiteral("MRP:"), m_spinMRP);
    f3->addRow(QStringLiteral("Wholesale Price:"), m_spinWholesalePrice);
    f3->addRow(QStringLiteral("Tax Rate (%):"), m_spinTaxRate);
    f3->addRow(QStringLiteral("Estimated Margin:"), m_lblMargin);

    connect(m_spinCostPrice, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ProductDialog::updateMarginCalculation);
    connect(m_spinSellingPrice, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ProductDialog::updateMarginCalculation);

    tabs->addTab(tabPrice, QStringLiteral("💰 3. Pricing & Tax"));

    // Tab 4: Inventory & Units
    QWidget *tabInv = new QWidget(tabs);
    QFormLayout *f4 = new QFormLayout(tabInv);
    f4->setContentsMargins(10, 10, 10, 10);
    f4->setSpacing(6);

    m_comboUnit = new QComboBox(tabInv);
    m_comboUnit->addItems(units);
    int unitIdx = m_comboUnit->findText(m_product.unitCode);
    if (unitIdx >= 0) m_comboUnit->setCurrentIndex(unitIdx);

    m_spinStock = new QDoubleSpinBox(tabInv);
    m_spinStock->setRange(-9999.0, 999999.0);
    m_spinStock->setValue(m_product.currentStock);

    m_spinReorder = new QDoubleSpinBox(tabInv);
    m_spinReorder->setRange(0.0, 99999.0);
    m_spinReorder->setValue(m_product.reorderLevel);

    f4->addRow(QStringLiteral("Base Unit:"), m_comboUnit);
    f4->addRow(QStringLiteral("Current Stock Quantity:"), m_spinStock);
    f4->addRow(QStringLiteral("Reorder Threshold Level:"), m_spinReorder);
    tabs->addTab(tabInv, QStringLiteral("📊 4. Inventory"));

    // Tab 5: Variants
    QWidget *tabVariants = new QWidget(tabs);
    QFormLayout *f5 = new QFormLayout(tabVariants);
    f5->setContentsMargins(10, 10, 10, 10);
    f5->setSpacing(6);

    m_txtVariantSize = new QLineEdit(tabVariants);
    m_txtVariantSize->setPlaceholderText(QStringLiteral("e.g. S, M, L, XL, 32, 34"));

    m_txtVariantColor = new QLineEdit(tabVariants);
    m_txtVariantColor->setPlaceholderText(QStringLiteral("e.g. Navy Blue, Black, Olive"));

    m_txtVariantSku = new QLineEdit(tabVariants);
    m_txtVariantSku->setPlaceholderText(QStringLiteral("e.g. SKU-1005-M-BLU"));

    f5->addRow(QStringLiteral("Size / Dimension:"), m_txtVariantSize);
    f5->addRow(QStringLiteral("Color / Finish:"), m_txtVariantColor);
    f5->addRow(QStringLiteral("Variant SKU:"), m_txtVariantSku);
    tabs->addTab(tabVariants, QStringLiteral("🔀 5. Variants"));

    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(this->layout());
    if (layout) {
        layout->addWidget(tabs);
    }
}

void ProductDialog::updateMarginCalculation() {
    double cost = m_spinCostPrice ? m_spinCostPrice->value() : 0.0;
    double sell = m_spinSellingPrice ? m_spinSellingPrice->value() : 0.0;
    if (m_lblMargin) {
        if (sell > 0.0) {
            double profit = sell - cost;
            double marginPct = (profit / sell) * 100.0;
            m_lblMargin->setText(QStringLiteral("$%1 profit (%2% margin)").arg(profit, 0, 'f', 2).arg(marginPct, 0, 'f', 1));
        } else {
            m_lblMargin->setText(QStringLiteral("N/A"));
        }
    }
}

void ProductDialog::onSaveClicked() {
    if (m_txtName->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Validation Error"), QStringLiteral("Product Name is required."));
        return;
    }
    if (m_txtSku->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Validation Error"), QStringLiteral("Product SKU is required."));
        return;
    }
    if (m_txtBarcode->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Validation Error"), QStringLiteral("Barcode is required for POS scanning."));
        return;
    }

    m_product.name = m_txtName->text().trimmed();
    m_product.sku = m_txtSku->text().trimmed();
    m_product.barcode = m_txtBarcode->text().trimmed();
    m_product.categoryName = m_comboCategory->currentText();
    m_product.brandName = m_comboBrand->currentText();
    m_product.unitCode = m_comboUnit->currentText();
    m_product.costPrice = m_spinCostPrice->value();
    m_product.sellingPrice = m_spinSellingPrice->value();
    m_product.mrp = m_spinMRP->value();
    m_product.wholesalePrice = m_spinWholesalePrice->value();
    m_product.taxRate = m_spinTaxRate->value();
    m_product.currentStock = m_spinStock->value();
    m_product.reorderLevel = m_spinReorder->value();
    m_product.isActive = m_chkActive->isChecked();

    if (DatabaseManager::instance().saveProduct(m_product, m_isNew)) {
        accept();
    } else {
        QMessageBox::critical(this, QStringLiteral("Database Error"), QStringLiteral("Failed to save product: %1").arg(DatabaseManager::instance().lastError()));
    }
}
