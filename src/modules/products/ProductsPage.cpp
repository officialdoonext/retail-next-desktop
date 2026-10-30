#include "ProductsPage.h"
#include "ProductDialog.h"
#include "../../ui/BarcodeLabelDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QInputDialog>
#include <QShortcut>
#include <algorithm>

ProductsPage::ProductsPage(QWidget *parent)
    : QWidget(parent)
    , m_txtSearch(nullptr)
    , m_comboCategory(nullptr)
    , m_comboStockStatus(nullptr)
    , m_tableProducts(nullptr)
    , m_lblSummary(nullptr)
{
    setupUI();
    reloadData();
}

void ProductsPage::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 10, 12, 10);
    mainLayout->setSpacing(8);

    setStyleSheet(QStringLiteral("QWidget { background-color: #D9E8F5; }"));

    // ==========================================
    // Page Header (Matches Reference Image Exactly)
    // Left: Box Icon 📦 + Title & Subtitle
    // Right: Category Combo + Stock Status Combo + Refresh Button
    // ==========================================
    QFrame *headerFrame = new QFrame(this);
    headerFrame->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 3px; padding: 4px; }"
    ));
    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(6, 2, 6, 2);

    // Left title group
    QHBoxLayout *titleGroup = new QHBoxLayout();
    titleGroup->setSpacing(8);

    QLabel *lblIcon = new QLabel(QStringLiteral("📦"), headerFrame);
    lblIcon->setStyleSheet(QStringLiteral("font-size: 22pt; padding: 0px; margin: 0px;"));
    titleGroup->addWidget(lblIcon);

    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setSpacing(1);
    QLabel *lblTitle = new QLabel(QStringLiteral("Item Master & Product Catalog"), headerFrame);
    lblTitle->setStyleSheet(QStringLiteral("font-size: 13pt; font-weight: 800; color: #0A294A; margin: 0;"));
    QLabel *lblSubtitle = new QLabel(QStringLiteral("Manage products, SKU barcodes, pricing tiers, units, and inventory threshold limits."), headerFrame);
    lblSubtitle->setStyleSheet(QStringLiteral("font-size: 8.5pt; color: #4A6B82; font-style: italic; margin: 0;"));
    textLayout->addWidget(lblTitle);
    textLayout->addWidget(lblSubtitle);
    titleGroup->addLayout(textLayout);

    headerLayout->addLayout(titleGroup);
    headerLayout->addStretch();

    // Right filter toolbar
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(6);

    // Hidden search edit to allow programmatic searching
    m_txtSearch = new QLineEdit(headerFrame);
    m_txtSearch->hide();

    QLabel *lblCat = new QLabel(QStringLiteral("Category:"), headerFrame);
    lblCat->setStyleSheet(QStringLiteral("font-weight: bold; color: #0A294A; font-size: 8.5pt;"));
    m_comboCategory = new QComboBox(headerFrame);
    m_comboCategory->setMinimumWidth(140);
    m_comboCategory->addItem(QStringLiteral("All Categories"), 0);

    QLabel *lblStock = new QLabel(QStringLiteral("Stock Status:"), headerFrame);
    lblStock->setStyleSheet(QStringLiteral("font-weight: bold; color: #0A294A; font-size: 8.5pt;"));
    m_comboStockStatus = new QComboBox(headerFrame);
    m_comboStockStatus->setMinimumWidth(140);
    m_comboStockStatus->addItem(QStringLiteral("All Stock Levels"), 0);
    m_comboStockStatus->addItem(QStringLiteral("In Stock (> 0)"), 1);
    m_comboStockStatus->addItem(QStringLiteral("Low Stock Alert"), 2);
    m_comboStockStatus->addItem(QStringLiteral("Out of Stock (= 0)"), 3);

    QPushButton *btnRefresh = new QPushButton(QStringLiteral("🔄 Refresh (F5)"), headerFrame);
    btnRefresh->setCursor(Qt::PointingHandCursor);
    btnRefresh->setShortcut(QKeySequence(Qt::Key_F5));
    btnRefresh->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; border-color: #4A82AD; }"
    ));

    filterLayout->addWidget(lblCat);
    filterLayout->addWidget(m_comboCategory);
    filterLayout->addWidget(lblStock);
    filterLayout->addWidget(m_comboStockStatus);
    filterLayout->addWidget(btnRefresh);

    headerLayout->addLayout(filterLayout);
    mainLayout->addWidget(headerFrame);

    // ==========================================
    // Data Table View (Native ERP Grid)
    // ==========================================
    m_tableProducts = new QTableWidget(this);
    m_tableProducts->setColumnCount(11);
    QStringList headers;
    headers << QStringLiteral("SKU ▾")
            << QStringLiteral("Barcode")
            << QStringLiteral("Product Name")
            << QStringLiteral("Category")
            << QStringLiteral("Unit")
            << QStringLiteral("Cost Price")
            << QStringLiteral("Selling Price")
            << QStringLiteral("Stock")
            << QStringLiteral("Reorder Level")
            << QStringLiteral("Status")
            << QStringLiteral("Actions");
    m_tableProducts->setHorizontalHeaderLabels(headers);
    m_tableProducts->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableProducts->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tableProducts->verticalHeader()->setVisible(false);
    m_tableProducts->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableProducts->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableProducts->setAlternatingRowColors(true);
    m_tableProducts->setStyleSheet(QStringLiteral(
        "QTableWidget { background-color: #FFFFFF; alternate-background-color: #F8FBFE; "
        "gridline-color: #C2D7EA; border: 1px solid #7A9EB9; font-size: 8.5pt; color: #0A294A; } "
        "QTableWidget::item { padding: 2px 4px; } "
        "QTableWidget::item:selected { background-color: #2D82D6; color: #FFFFFF; font-weight: bold; } "
        "QHeaderView::section { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B3D4F0, stop:1 #8DBDE0); "
        "color: #0A294A; font-weight: bold; font-size: 8pt; border: none; border-top: 1px solid #C2D7EA; "
        "border-bottom: 1px solid #6895BA; border-right: 1px solid #99BEDC; padding: 3px 5px; }"
    ));
    mainLayout->addWidget(m_tableProducts, 1);

    // ==========================================
    // Table Footer Summary & Pagination Bar
    // ==========================================
    QFrame *footerFrame = new QFrame(this);
    footerFrame->setFixedHeight(30);
    footerFrame->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; }"
    ));
    QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(6, 2, 6, 2);

    m_lblSummary = new QLabel(QStringLiteral("Showing 0 products | Total Inventory Stock Valuation: $0.00"), footerFrame);
    m_lblSummary->setStyleSheet(QStringLiteral("font-weight: bold; color: #000080; font-size: 8.5pt;"));
    footerLayout->addWidget(m_lblSummary);
    footerLayout->addStretch();

    // Pagination controls on the right (matches reference image)
    QLabel *lblPerPage = new QLabel(QStringLiteral("Items per page:"), footerFrame);
    lblPerPage->setStyleSheet(QStringLiteral("color: #0A294A; font-size: 8.5pt;"));
    QComboBox *comboPerPage = new QComboBox(footerFrame);
    comboPerPage->addItem(QStringLiteral("50"));
    comboPerPage->addItem(QStringLiteral("100"));
    comboPerPage->addItem(QStringLiteral("250"));
    comboPerPage->setStyleSheet(QStringLiteral(
        "QComboBox { background: #FFFFFF; border: 1px solid #7A9EB9; border-radius: 2px; padding: 1px 4px; font-size: 8.5pt; }"
    ));

    QLabel *lblPage = new QLabel(QStringLiteral(" | Page 1 of 1 | "), footerFrame);
    lblPage->setStyleSheet(QStringLiteral("color: #0A294A; font-size: 8.5pt;"));

    const QString pageBtnStyle = QStringLiteral(
        "QPushButton { background: #FFFFFF; color: #0A294A; border: 1px solid #7A9EB9; "
        "border-radius: 2px; padding: 1px 6px; font-size: 8pt; min-width: 18px; } "
        "QPushButton:hover { background: #EDF4FB; }"
    );

    QPushButton *btnFirst = new QPushButton(QStringLiteral("|<< (Alt+Home)"), footerFrame);
    btnFirst->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Home));
    btnFirst->setStyleSheet(pageBtnStyle);
    QPushButton *btnPrev = new QPushButton(QStringLiteral("< (Alt+Left)"), footerFrame);
    btnPrev->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Left));
    btnPrev->setStyleSheet(pageBtnStyle);
    QPushButton *btnNext = new QPushButton(QStringLiteral("> (Alt+Right)"), footerFrame);
    btnNext->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Right));
    btnNext->setStyleSheet(pageBtnStyle);
    QPushButton *btnLast = new QPushButton(QStringLiteral(">>| (Alt+End)"), footerFrame);
    btnLast->setShortcut(QKeySequence(Qt::ALT | Qt::Key_End));
    btnLast->setStyleSheet(pageBtnStyle);

    footerLayout->addWidget(lblPerPage);
    footerLayout->addWidget(comboPerPage);
    footerLayout->addWidget(lblPage);
    footerLayout->addWidget(btnFirst);
    footerLayout->addWidget(btnPrev);
    footerLayout->addWidget(btnNext);
    footerLayout->addWidget(btnLast);

    mainLayout->addWidget(footerFrame);

    // ==========================================
    // Bottom Function Key Bar (Matching Reference Image)
    // ==========================================
    QFrame *fnBar = new QFrame(this);
    fnBar->setFixedHeight(32);
    fnBar->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; }"
    ));
    QHBoxLayout *fnLayout = new QHBoxLayout(fnBar);
    fnLayout->setContentsMargins(4, 2, 4, 2);
    fnLayout->setSpacing(3);

    const QString fnBtnStyle = QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 2px 8px; font-size: 8.5pt; font-weight: bold; } "
        "QPushButton:hover { background: #EDF4FB; border-color: #4A82AD; }"
    );

    QPushButton *bF1 = new QPushButton(QStringLiteral("F1 - Help"), fnBar);
    bF1->setStyleSheet(fnBtnStyle);
    bF1->setShortcut(QKeySequence(Qt::Key_F1));

    QPushButton *bF3 = new QPushButton(QStringLiteral("F3 - New Product"), fnBar);
    bF3->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 2px 10px; font-size: 8.5pt; font-weight: bold; } "
        "QPushButton:hover { background: #C8E6C9; }"
    ));
    bF3->setShortcut(QKeySequence(Qt::Key_F3));
    connect(bF3, &QPushButton::clicked, this, &ProductsPage::onAddProductClicked);

    QPushButton *bF4 = new QPushButton(QStringLiteral("F4 - Edit Product"), fnBar);
    bF4->setStyleSheet(fnBtnStyle);
    bF4->setShortcut(QKeySequence(Qt::Key_F4));
    connect(bF4, &QPushButton::clicked, this, &ProductsPage::onEditProductClicked);

    QPushButton *bF5 = new QPushButton(QStringLiteral("F5 - Bulk Price"), fnBar);
    bF5->setStyleSheet(fnBtnStyle);
    bF5->setShortcut(QKeySequence(Qt::Key_F5));
    connect(bF5, &QPushButton::clicked, this, &ProductsPage::onBulkPriceUpdateClicked);

    QPushButton *bF6 = new QPushButton(QStringLiteral("F6 - Barcode Label"), fnBar);
    bF6->setStyleSheet(fnBtnStyle);
    bF6->setShortcut(QKeySequence(Qt::Key_F6));
    connect(bF6, &QPushButton::clicked, this, &ProductsPage::onPrintBarcodeClicked);

    QPushButton *bF7 = new QPushButton(QStringLiteral("F7 - Export CSV"), fnBar);
    bF7->setStyleSheet(fnBtnStyle);
    bF7->setShortcut(QKeySequence(Qt::Key_F7));
    connect(bF7, &QPushButton::clicked, this, &ProductsPage::onExportCsvClicked);

    QPushButton *bF8 = new QPushButton(QStringLiteral("F8 - Import CSV"), fnBar);
    bF8->setStyleSheet(fnBtnStyle);
    bF8->setShortcut(QKeySequence(Qt::Key_F8));
    connect(bF8, &QPushButton::clicked, this, &ProductsPage::onImportCsvClicked);

    QPushButton *bF9 = new QPushButton(QStringLiteral("F9 - Refresh"), fnBar);
    bF9->setStyleSheet(fnBtnStyle);
    bF9->setShortcut(QKeySequence(Qt::Key_F9));
    connect(bF9, &QPushButton::clicked, this, &ProductsPage::reloadData);

    QPushButton *bEsc = new QPushButton(QStringLiteral("Esc - Cancel"), fnBar);
    bEsc->setStyleSheet(fnBtnStyle);
    bEsc->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(bEsc, &QPushButton::clicked, this, [this]() {
      if (m_txtSearch) m_txtSearch->clear();
    });

    fnLayout->addWidget(bF1);
    fnLayout->addWidget(bF3);
    fnLayout->addWidget(bF4);
    fnLayout->addWidget(bF5);
    fnLayout->addWidget(bF6);
    fnLayout->addWidget(bF7);
    fnLayout->addWidget(bF8);
    fnLayout->addStretch();
    fnLayout->addWidget(bF9);
    fnLayout->addWidget(bEsc);

    mainLayout->addWidget(fnBar);

    // Event Connections
    connect(m_comboCategory, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProductsPage::onSearchChanged);
    connect(m_comboStockStatus, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ProductsPage::onSearchChanged);
    connect(btnRefresh, &QPushButton::clicked, this, &ProductsPage::reloadData);
    connect(m_tableProducts, &QTableWidget::cellDoubleClicked, this, &ProductsPage::onTableDoubleClicked);
}

void ProductsPage::reloadData() {
    DatabaseManager &db = DatabaseManager::instance();

    // Populate category dropdown
    m_comboCategory->blockSignals(true);
    int currentCat = m_comboCategory->currentData().toInt();
    m_comboCategory->clear();
    m_comboCategory->addItem(QStringLiteral("All Categories"), 0);
    auto catList = db.getCategoriesList();
    for (int i = 0; i < catList.size(); ++i) {
        m_comboCategory->addItem(catList[i], i + 1);
    }
    int idx = m_comboCategory->findData(currentCat);
    if (idx >= 0) m_comboCategory->setCurrentIndex(idx);
    m_comboCategory->blockSignals(false);

    onSearchChanged();
}

void ProductsPage::onSearchChanged() {
    QString text = m_txtSearch ? m_txtSearch->text().trimmed() : QString();
    int catId = m_comboCategory ? m_comboCategory->currentData().toInt() : 0;
    int stockFilter = m_comboStockStatus ? m_comboStockStatus->currentData().toInt() : 0;

    m_currentProducts = DatabaseManager::instance().searchProducts(text, catId, stockFilter);

    // Sort descending by SKU (SKU-1012 down to SKU-1001) as shown in reference image
    std::sort(m_currentProducts.begin(), m_currentProducts.end(), [](const RetailNext::ProductItem &a, const RetailNext::ProductItem &b) {
        return a.sku > b.sku;
    });

    m_tableProducts->setRowCount(m_currentProducts.size());

    double totalValuation = 0.0;
    QString cur = DatabaseManager::instance().currencySymbol();

    for (int r = 0; r < m_currentProducts.size(); ++r) {
        const auto& p = m_currentProducts[r];
        double val = p.currentStock * p.costPrice;
        totalValuation += val;

        m_tableProducts->setRowHeight(r, 22);

        // Column 0: SKU
        QTableWidgetItem *skuItem = new QTableWidgetItem(p.sku);
        skuItem->setTextAlignment(Qt::AlignCenter);
        m_tableProducts->setItem(r, 0, skuItem);

        // Column 1: Barcode
        QTableWidgetItem *barcodeItem = new QTableWidgetItem(p.barcode);
        barcodeItem->setTextAlignment(Qt::AlignCenter);
        m_tableProducts->setItem(r, 1, barcodeItem);

        // Column 2: Product Name
        QTableWidgetItem *nameItem = new QTableWidgetItem(p.name);
        nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_tableProducts->setItem(r, 2, nameItem);

        // Column 3: Category
        QTableWidgetItem *catItem = new QTableWidgetItem(p.categoryName);
        catItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_tableProducts->setItem(r, 3, catItem);

        // Column 4: Unit
        QTableWidgetItem *unitItem = new QTableWidgetItem(p.unitCode);
        unitItem->setTextAlignment(Qt::AlignCenter);
        m_tableProducts->setItem(r, 4, unitItem);

        // Column 5: Cost Price
        QTableWidgetItem *costItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(p.costPrice, 0, 'f', 2));
        costItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_tableProducts->setItem(r, 5, costItem);

        // Column 6: Selling Price (Bold Blue)
        QTableWidgetItem *priceItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(p.sellingPrice, 0, 'f', 2));
        priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        priceItem->setForeground(QBrush(QColor("#1565C0")));
        QFont priceFont = priceItem->font();
        priceFont.setBold(true);
        priceItem->setFont(priceFont);
        m_tableProducts->setItem(r, 6, priceItem);

        // Column 7: Stock (Red if 0, Orange if low, Normal if good)
        QTableWidgetItem *stockItem = new QTableWidgetItem(QString::number(p.currentStock, 'f', p.unitCode == "PCS" ? 0 : 2));
        stockItem->setTextAlignment(Qt::AlignCenter);
        QFont stockFont = stockItem->font();
        stockFont.setBold(true);
        stockItem->setFont(stockFont);
        if (p.currentStock <= 0.0) {
            stockItem->setForeground(QBrush(QColor("#D32F2F"))); // Red
        } else if (p.currentStock <= p.reorderLevel) {
            stockItem->setForeground(QBrush(QColor("#E65100"))); // Orange
        } else {
            stockItem->setForeground(QBrush(QColor("#1E293B")));
        }
        m_tableProducts->setItem(r, 7, stockItem);

        // Column 8: Reorder Level
        QTableWidgetItem *reorderItem = new QTableWidgetItem(QString::number(p.reorderLevel, 'f', 0));
        reorderItem->setTextAlignment(Qt::AlignCenter);
        m_tableProducts->setItem(r, 8, reorderItem);

        // Column 9: Status Pill Badge (Out of Stock, Low Stock, Active)
        QWidget *statusWidget = new QWidget(m_tableProducts);
        QHBoxLayout *statusLayout = new QHBoxLayout(statusWidget);
        statusLayout->setContentsMargins(6, 2, 6, 2);
        statusLayout->setAlignment(Qt::AlignCenter);

        QLabel *lblBadge = new QLabel(statusWidget);
        lblBadge->setAlignment(Qt::AlignCenter);

        if (p.currentStock <= 0.0) {
            lblBadge->setText(QStringLiteral("Out of Stock"));
            lblBadge->setStyleSheet(QStringLiteral(
                "QLabel { background-color: #FFEBEE; color: #C62828; border: 1px solid #FFCDD2; "
                "border-radius: 4px; padding: 2px 10px; font-weight: bold; font-size: 8.5pt; }"
            ));
        } else if (p.currentStock <= p.reorderLevel) {
            lblBadge->setText(QStringLiteral("Low Stock"));
            lblBadge->setStyleSheet(QStringLiteral(
                "QLabel { background-color: #FFF3E0; color: #E65100; border: 1px solid #FFE0B2; "
                "border-radius: 4px; padding: 2px 10px; font-weight: bold; font-size: 8.5pt; }"
            ));
        } else {
            lblBadge->setText(QStringLiteral("Active"));
            lblBadge->setStyleSheet(QStringLiteral(
                "QLabel { background-color: #E8F5E9; color: #2E7D32; border: 1px solid #C8E6C9; "
                "border-radius: 4px; padding: 2px 10px; font-weight: bold; font-size: 8.5pt; }"
            ));
        }
        statusLayout->addWidget(lblBadge);
        m_tableProducts->setCellWidget(r, 9, statusWidget);

        // Column 10: Actions (3 Icon Buttons: ✏️ Edit, 📋 Barcode Label, 🗑️ Delete)
        QWidget *actionWidget = new QWidget(m_tableProducts);
        QHBoxLayout *actLayout = new QHBoxLayout(actionWidget);
        actLayout->setContentsMargins(2, 2, 2, 2);
        actLayout->setSpacing(4);
        actLayout->setAlignment(Qt::AlignCenter);

        // Edit button (Orange)
        QPushButton *btnEdit = new QPushButton(QStringLiteral("✏️"), actionWidget);
        btnEdit->setToolTip(QStringLiteral("Edit Product"));
        btnEdit->setCursor(Qt::PointingHandCursor);
        btnEdit->setStyleSheet(QStringLiteral(
            "QPushButton { background: #FFFFFF; color: #E65100; border: 1px solid #FFE0B2; "
            "border-radius: 3px; min-width: 24px; max-width: 26px; min-height: 22px; max-height: 24px; padding: 0; font-size: 9pt; } "
            "QPushButton:hover { background: #FFF3E0; border-color: #FFB74D; }"
        ));

        // Barcode / Copy button (Blue)
        QPushButton *btnLabel = new QPushButton(QStringLiteral("📋"), actionWidget);
        btnLabel->setToolTip(QStringLiteral("Print Barcode Label"));
        btnLabel->setCursor(Qt::PointingHandCursor);
        btnLabel->setStyleSheet(QStringLiteral(
            "QPushButton { background: #FFFFFF; color: #1976D2; border: 1px solid #BBDEFB; "
            "border-radius: 3px; min-width: 24px; max-width: 26px; min-height: 22px; max-height: 24px; padding: 0; font-size: 9pt; } "
            "QPushButton:hover { background: #E3F2FD; border-color: #90CAF9; }"
        ));

        // Delete button (Red)
        QPushButton *btnDelete = new QPushButton(QStringLiteral("🗑️"), actionWidget);
        btnDelete->setToolTip(QStringLiteral("Deactivate Product"));
        btnDelete->setCursor(Qt::PointingHandCursor);
        btnDelete->setStyleSheet(QStringLiteral(
            "QPushButton { background: #FFFFFF; color: #D32F2F; border: 1px solid #FFCDD2; "
            "border-radius: 3px; min-width: 24px; max-width: 26px; min-height: 22px; max-height: 24px; padding: 0; font-size: 9pt; } "
            "QPushButton:hover { background: #FFEBEE; border-color: #EF9A9A; }"
        ));

        connect(btnEdit, &QPushButton::clicked, this, [this, r]() {
            if (r < m_currentProducts.size()) {
                ProductDialog dlg(m_currentProducts[r], false, this);
                if (dlg.exec() == QDialog::Accepted) {
                    reloadData();
                }
            }
        });

        connect(btnLabel, &QPushButton::clicked, this, [this, r]() {
            if (r < m_currentProducts.size()) {
                BarcodeLabelDialog dlg(m_currentProducts[r], this);
                dlg.exec();
            }
        });

        connect(btnDelete, &QPushButton::clicked, this, [this, r]() {
            if (r < m_currentProducts.size()) {
                auto reply = QMessageBox::question(this, QStringLiteral("Deactivate Product"),
                    QStringLiteral("Are you sure you want to deactivate '%1'?").arg(m_currentProducts[r].name),
                    QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                    DatabaseManager::instance().deleteProduct(m_currentProducts[r].id);
                    reloadData();
                }
            }
        });

        actLayout->addWidget(btnEdit);
        actLayout->addWidget(btnLabel);
        actLayout->addWidget(btnDelete);
        m_tableProducts->setCellWidget(r, 10, actionWidget);
    }

    m_lblSummary->setText(QStringLiteral("Showing %1 products  |  Total Inventory Stock Valuation: %2%3")
        .arg(m_currentProducts.size())
        .arg(cur)
        .arg(totalValuation, 0, 'f', 2));
}

void ProductsPage::onAddProductClicked() {
    RetailNext::ProductItem newProd;
    newProd.sku = QStringLiteral("SKU-%1").arg(QDateTime::currentMSecsSinceEpoch() % 10000);
    newProd.barcode = QStringLiteral("890103%1").arg(QDateTime::currentMSecsSinceEpoch() % 1000000, 6, 10, QChar('0'));
    newProd.costPrice = 10.0;
    newProd.sellingPrice = 15.0;
    newProd.mrp = 18.0;
    newProd.currentStock = 10.0;
    newProd.reorderLevel = 5.0;
    newProd.taxRate = 5.0;

    ProductDialog dlg(newProd, true, this);
    if (dlg.exec() == QDialog::Accepted) {
        reloadData();
    }
}

void ProductsPage::onEditProductClicked() {
    int row = m_tableProducts->currentRow();
    if (row < 0 || row >= m_currentProducts.size()) {
        QMessageBox::information(this, QStringLiteral("Edit Product"), QStringLiteral("Please select a product from the table first."));
        return;
    }
    ProductDialog dlg(m_currentProducts[row], false, this);
    if (dlg.exec() == QDialog::Accepted) {
        reloadData();
    }
}

void ProductsPage::onDeleteProductClicked() {
    int row = m_tableProducts->currentRow();
    if (row < 0 || row >= m_currentProducts.size()) {
        QMessageBox::information(this, QStringLiteral("Deactivate Product"), QStringLiteral("Please select a product from the table first."));
        return;
    }
    auto reply = QMessageBox::question(this, QStringLiteral("Deactivate Product"),
        QStringLiteral("Are you sure you want to deactivate '%1'?").arg(m_currentProducts[row].name),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        DatabaseManager::instance().deleteProduct(m_currentProducts[row].id);
        reloadData();
    }
}

void ProductsPage::onBulkPriceUpdateClicked() {
    bool ok;
    double percentage = QInputDialog::getDouble(this, QStringLiteral("Bulk Price Adjustment"),
        QStringLiteral("Adjust all product selling prices by percentage (+/- %):\n(e.g., enter 10 for a 10% increase, -5 for a 5% discount)"),
        0.0, -90.0, 500.0, 2, &ok);

    if (ok && percentage != 0.0) {
        if (DatabaseManager::instance().bulkUpdatePrices(0, percentage)) {
            QMessageBox::information(this, QStringLiteral("Bulk Update"),
                QStringLiteral("Selling prices successfully updated by %1% across the catalog.").arg(percentage));
            reloadData();
        } else {
            QMessageBox::critical(this, QStringLiteral("Update Failed"),
                QStringLiteral("Failed to apply bulk price update."));
        }
    }
}

void ProductsPage::onPrintBarcodeClicked() {
    int row = m_tableProducts->currentRow();
    if (row < 0 || row >= m_currentProducts.size()) {
        if (!m_currentProducts.isEmpty()) {
            row = 0;
        } else {
            QMessageBox::information(this, QStringLiteral("Barcode Label"), QStringLiteral("No product available to generate label."));
            return;
        }
    }
    BarcodeLabelDialog dlg(m_currentProducts[row], this);
    dlg.exec();
}

void ProductsPage::onExportCsvClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, QStringLiteral("Export Products Catalog"),
        QStringLiteral("Products_Export.csv"), QStringLiteral("CSV Files (*.csv)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, QStringLiteral("Export Error"), QStringLiteral("Cannot open file for writing."));
        return;
    }

    QTextStream out(&file);
    out << "SKU,Barcode,Name,Category,Unit,CostPrice,SellingPrice,Stock,ReorderLevel,TaxRate,Active\n";
    for (const auto &p : m_currentProducts) {
        out << QStringLiteral("\"%1\",\"%2\",\"%3\",\"%4\",\"%5\",%6,%7,%8,%9,%10,%11\n")
            .arg(p.sku)
            .arg(p.barcode)
            .arg(p.name)
            .arg(p.categoryName)
            .arg(p.unitCode)
            .arg(p.costPrice)
            .arg(p.sellingPrice)
            .arg(p.currentStock)
            .arg(p.reorderLevel)
            .arg(p.taxRate)
            .arg(p.isActive ? 1 : 0);
    }
    file.close();
    QMessageBox::information(this, QStringLiteral("Export Complete"), QStringLiteral("Exported %1 products to CSV.").arg(m_currentProducts.size()));
}

void ProductsPage::onImportCsvClicked() {
    QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("Import Products CSV"),
        QString(), QStringLiteral("CSV Files (*.csv);;All Files (*.*)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, QStringLiteral("Import Error"), QStringLiteral("Cannot open CSV file for reading."));
        return;
    }

    QTextStream in(&file);
    int imported = 0;
    bool isHeader = true;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        if (isHeader) { isHeader = false; continue; }

        QStringList parts = line.split(',');
        if (parts.size() >= 7) {
            RetailNext::ProductItem p;
            p.sku = parts[0].trimmed().remove('"');
            p.barcode = parts[1].trimmed().remove('"');
            p.name = parts[2].trimmed().remove('"');
            p.costPrice = parts[5].trimmed().toDouble();
            p.sellingPrice = parts[6].trimmed().toDouble();
            p.mrp = p.sellingPrice * 1.1;
            p.currentStock = parts.size() > 7 ? parts[7].trimmed().toDouble() : 10.0;
            p.reorderLevel = parts.size() > 8 ? parts[8].trimmed().toDouble() : 5.0;
            p.taxRate = 5.0;
            p.isActive = true;

            DatabaseManager::instance().saveProduct(p);
            imported++;
        }
    }
    file.close();
    reloadData();
    QMessageBox::information(this, QStringLiteral("Import Finished"), QStringLiteral("Successfully imported %1 products.").arg(imported));
}

void ProductsPage::filterByCategory(int catId) {
    int idx = m_comboCategory->findData(catId);
    if (idx >= 0) {
        m_comboCategory->setCurrentIndex(idx);
    }
}

void ProductsPage::onTableDoubleClicked(int row, int col) {
    Q_UNUSED(col);
    if (row >= 0 && row < m_currentProducts.size()) {
        ProductDialog dlg(m_currentProducts[row], false, this);
        if (dlg.exec() == QDialog::Accepted) {
            reloadData();
        }
    }
}
