#include "GlobalSearchDialog.h"
#include "../database/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>

GlobalSearchDialog::GlobalSearchDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("RetailNext Global Search (Products, Invoices, Customers)"));
    resize(720, 500);

    setStyleSheet(QStringLiteral(
        "QDialog { background-color: #D9E8F5; } "
        "QLabel { color: #0A294A; } "
        "QLineEdit { background: #FFFFFF; border: 1px solid #7A9EB9; border-radius: 2px; padding: 4px 8px; color: #000080; font-weight: bold; font-size: 10pt; } "
        "QLineEdit:focus { border: 1px solid #0055A5; background-color: #FFFFF2; } "
        "QTableWidget { background-color: #FFFFFF; alternate-background-color: #F8FBFE; "
        "gridline-color: #C2D7EA; border: 1px solid #7A9EB9; font-size: 8.5pt; color: #0A294A; } "
        "QTableWidget::item { padding: 2px 4px; } "
        "QTableWidget::item:selected { background-color: #2D82D6; color: #FFFFFF; font-weight: bold; } "
        "QHeaderView::section { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B3D4F0, stop:1 #8DBDE0); "
        "color: #0A294A; font-weight: bold; font-size: 8pt; border: none; border-top: 1px solid #C2D7EA; "
        "border-bottom: 1px solid #6895BA; border-right: 1px solid #99BEDC; padding: 3px 5px; } "
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 4px 16px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; }"
    ));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    QLabel *hdr = new QLabel(QStringLiteral("<b>Global Search:</b> Type SKU, barcode, name, invoice #, phone, or customer name"), this);
    hdr->setStyleSheet(QStringLiteral("color: #0A294A; font-size: 9.5pt;"));
    mainLayout->addWidget(hdr);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(QStringLiteral("Start typing to search across all records..."));
    mainLayout->addWidget(m_searchEdit);

    m_resultsTable = new QTableWidget(this);
    m_resultsTable->setColumnCount(4);
    m_resultsTable->setHorizontalHeaderLabels(QStringList() << QStringLiteral("Type") << QStringLiteral("Reference / Code") << QStringLiteral("Title / Name") << QStringLiteral("Details"));
    m_resultsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_resultsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_resultsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_resultsTable->verticalHeader()->setVisible(false);
    m_resultsTable->verticalHeader()->setDefaultSectionSize(22);
    m_resultsTable->setAlternatingRowColors(true);
    m_resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(m_resultsTable);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QLabel *tip = new QLabel(QStringLiteral("Double-click any item to inspect details"), this);
    tip->setStyleSheet(QStringLiteral("color: #4A6B82; font-size: 8.5pt; font-style: italic;"));
    QPushButton *btnClose = new QPushButton(QStringLiteral("❌ Close (Esc)"), this);
    btnClose->setShortcut(QKeySequence(Qt::Key_Escape));
    btnLayout->addWidget(tip);
    btnLayout->addStretch();
    btnLayout->addWidget(btnClose);
    mainLayout->addLayout(btnLayout);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &GlobalSearchDialog::onSearchTextChanged);
    connect(m_resultsTable, &QTableWidget::cellDoubleClicked, this, &GlobalSearchDialog::onItemDoubleClicked);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);

    onSearchTextChanged(QString());
}

void GlobalSearchDialog::onSearchTextChanged(const QString& text) {
    m_resultsTable->setRowCount(0);
    DatabaseManager &db = DatabaseManager::instance();

    int row = 0;

    // Search Products
    auto prods = db.searchProducts(text);
    for (int i = 0; i < qMin(10, prods.size()); ++i) {
        const auto& p = prods[i];
        m_resultsTable->insertRow(row);
        m_resultsTable->setItem(row, 0, new QTableWidgetItem(QStringLiteral("📦 Product")));
        QTableWidgetItem *refItem = new QTableWidgetItem(p.sku);
        refItem->setForeground(QBrush(QColor("#000080")));
        refItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_resultsTable->setItem(row, 1, refItem);
        m_resultsTable->setItem(row, 2, new QTableWidgetItem(p.name));
        m_resultsTable->setItem(row, 3, new QTableWidgetItem(QStringLiteral("Stock: %1 %2 | Price: %3%4").arg(p.currentStock).arg(p.unitCode).arg(db.currencySymbol()).arg(p.sellingPrice, 0, 'f', 2)));
        m_resultsTable->item(row, 0)->setData(Qt::UserRole, 1); // 1 = Product
        m_resultsTable->item(row, 0)->setData(Qt::UserRole + 1, p.id);
        row++;
    }

    // Search Invoices
    auto invs = db.searchInvoices(text);
    for (int i = 0; i < qMin(10, invs.size()); ++i) {
        const auto& inv = invs[i];
        m_resultsTable->insertRow(row);
        m_resultsTable->setItem(row, 0, new QTableWidgetItem(QStringLiteral("🧾 Invoice")));
        QTableWidgetItem *invRef = new QTableWidgetItem(inv.invoiceNo);
        invRef->setForeground(QBrush(QColor("#000080")));
        invRef->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_resultsTable->setItem(row, 1, invRef);
        m_resultsTable->setItem(row, 2, new QTableWidgetItem(inv.customerName));
        m_resultsTable->setItem(row, 3, new QTableWidgetItem(QStringLiteral("Total: %1%2 | Date: %3").arg(db.currencySymbol()).arg(inv.netPayable, 0, 'f', 2).arg(inv.createdAt)));
        m_resultsTable->item(row, 0)->setData(Qt::UserRole, 2); // 2 = Invoice
        m_resultsTable->item(row, 0)->setData(Qt::UserRole + 1, inv.id);
        row++;
    }

    // Search Customers
    auto custs = db.searchCustomers(text);
    for (int i = 0; i < qMin(10, custs.size()); ++i) {
        const auto& c = custs[i];
        m_resultsTable->insertRow(row);
        m_resultsTable->setItem(row, 0, new QTableWidgetItem(QStringLiteral("👤 Customer")));
        QTableWidgetItem *custRef = new QTableWidgetItem(c.code);
        custRef->setForeground(QBrush(QColor("#000080")));
        custRef->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_resultsTable->setItem(row, 1, custRef);
        m_resultsTable->setItem(row, 2, new QTableWidgetItem(c.name));
        m_resultsTable->setItem(row, 3, new QTableWidgetItem(QStringLiteral("Phone: %1 | Points: %2").arg(c.phone).arg(c.loyaltyPoints)));
        m_resultsTable->item(row, 0)->setData(Qt::UserRole, 3); // 3 = Customer
        m_resultsTable->item(row, 0)->setData(Qt::UserRole + 1, c.id);
        row++;
    }
}

void GlobalSearchDialog::onItemDoubleClicked(int row, int /*col*/) {
    if (row < 0 || row >= m_resultsTable->rowCount()) return;
    int type = m_resultsTable->item(row, 0)->data(Qt::UserRole).toInt();
    int id = m_resultsTable->item(row, 0)->data(Qt::UserRole + 1).toInt();

    if (type == 1) {
        emit productSelected(id);
    } else if (type == 2) {
        emit invoiceSelected(id);
    } else if (type == 3) {
        emit customerSelected(id);
    }
    accept();
}

