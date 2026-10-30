#include "SalesPage.h"
#include "../../ui/ReceiptDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QTextStream>
#include <QShortcut>

SalesPage::SalesPage(QWidget *parent)
    : QWidget(parent)
    , m_tabs(nullptr)
    , m_txtSearch(nullptr)
    , m_tableInvoices(nullptr)
    , m_tableHeld(nullptr)
    , m_lblInvoicesSummary(nullptr)
{
    setupUI();
    reloadData();
}

void SalesPage::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 10, 12, 10);
    mainLayout->setSpacing(8);

    setStyleSheet(QStringLiteral("QWidget { background-color: #D9E8F5; }"));

    // ==========================================
    // Header Panel
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

    QLabel *lblIcon = new QLabel(QStringLiteral("🧾"), headerFrame);
    lblIcon->setStyleSheet(QStringLiteral("font-size: 22pt; padding: 0px; margin: 0px;"));
    titleGroup->addWidget(lblIcon);

    QVBoxLayout *titleText = new QVBoxLayout();
    titleText->setSpacing(1);
    QLabel *lblTitle = new QLabel(QStringLiteral("Sales Invoices & Order History"), headerFrame);
    lblTitle->setStyleSheet(QStringLiteral("font-size: 13pt; font-weight: 800; color: #0A294A; margin: 0;"));
    QLabel *lblSubtitle = new QLabel(QStringLiteral("Audit sales transactions, view invoice breakdowns, reprint receipts, and process customer returns."), headerFrame);
    lblSubtitle->setStyleSheet(QStringLiteral("font-size: 8.5pt; color: #4A6B82; font-style: italic; margin: 0;"));
    titleText->addWidget(lblTitle);
    titleText->addWidget(lblSubtitle);
    titleGroup->addLayout(titleText);
    headerLayout->addLayout(titleGroup);
    headerLayout->addStretch();

    // Search input inside header
    QLabel *lblSearch = new QLabel(QStringLiteral("Search:"), headerFrame);
    lblSearch->setStyleSheet(QStringLiteral("font-weight: bold; color: #0A294A; font-size: 8.5pt;"));
    m_txtSearch = new QLineEdit(headerFrame);
    m_txtSearch->setPlaceholderText(QStringLiteral("Invoice # or customer..."));
    m_txtSearch->setMinimumWidth(200);
    m_txtSearch->setStyleSheet(QStringLiteral(
        "QLineEdit { background: #FFFFFF; border: 1px solid #7A9EB9; border-radius: 2px; "
        "padding: 2px 6px; color: #000080; font-weight: bold; font-size: 9pt; } "
        "QLineEdit:focus { border: 1px solid #0055A5; background-color: #FFFFF2; }"
    ));

    const QString erpBtnStyle = QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; border-color: #4A82AD; }"
    );

    QPushButton *btnDetails = new QPushButton(QStringLiteral("🧾 View Details & Reprint (F5)"), headerFrame);
    btnDetails->setShortcut(QKeySequence(Qt::Key_F5));
    btnDetails->setStyleSheet(erpBtnStyle);

    QPushButton *btnReturn = new QPushButton(QStringLiteral("↩️ Process Return (F7)"), headerFrame);
    btnReturn->setShortcut(QKeySequence(Qt::Key_F7));
    btnReturn->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFEBEE, stop:1 #FFCDD2); "
        "color: #B71C1C; border: 1px solid #EF9A9A; border-radius: 2px; padding: 3px 10px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #FFCDD2; }"
    ));

    QPushButton *btnRefresh = new QPushButton(QStringLiteral("🔄 Refresh (F9)"), headerFrame);
    btnRefresh->setShortcut(QKeySequence(Qt::Key_F9));
    btnRefresh->setStyleSheet(erpBtnStyle);

    headerLayout->addWidget(lblSearch);
    headerLayout->addWidget(m_txtSearch);
    headerLayout->addWidget(btnDetails);
    headerLayout->addWidget(btnReturn);
    headerLayout->addWidget(btnRefresh);
    mainLayout->addWidget(headerFrame);

    // ==========================================
    // Sub-tabs
    // ==========================================
    m_tabs = new QTabWidget(this);
    m_tabs->setStyleSheet(QStringLiteral(
        "QTabWidget::pane { border: 1px solid #8FAEC7; background-color: #D9E8F5; top: -1px; } "
        "QTabBar::tab { background-color: #D0E2F2; color: #0A294A; border: 1px solid #8FAEC7; border-bottom: none; "
        "border-top-left-radius: 3px; border-top-right-radius: 3px; padding: 3px 14px; font-size: 8.5pt; font-weight: 600; margin-right: 2px; } "
        "QTabBar::tab:selected { background-color: #FFFFFF; color: #000080; border-bottom: 2px solid #0055A5; font-weight: bold; }"
    ));

    const QString tableStyle = QStringLiteral(
        "QTableWidget { background-color: #FFFFFF; alternate-background-color: #F8FBFE; "
        "gridline-color: #C2D7EA; border: 1px solid #7A9EB9; font-size: 8.5pt; color: #0A294A; } "
        "QTableWidget::item { padding: 2px 4px; } "
        "QTableWidget::item:selected { background-color: #2D82D6; color: #FFFFFF; font-weight: bold; } "
        "QHeaderView::section { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B3D4F0, stop:1 #8DBDE0); "
        "color: #0A294A; font-weight: bold; font-size: 8pt; border: none; border-top: 1px solid #C2D7EA; "
        "border-bottom: 1px solid #6895BA; border-right: 1px solid #99BEDC; padding: 3px 5px; }"
    );

    // Tab 1: Completed Sales Invoices
    QWidget *tabInvoices = new QWidget(m_tabs);
    QVBoxLayout *l1 = new QVBoxLayout(tabInvoices);
    l1->setContentsMargins(6, 6, 6, 6);
    l1->setSpacing(4);

    m_tableInvoices = new QTableWidget(tabInvoices);
    m_tableInvoices->setColumnCount(9);
    m_tableInvoices->setHorizontalHeaderLabels(QStringList()
        << QStringLiteral("Invoice #")
        << QStringLiteral("Date / Time")
        << QStringLiteral("Customer")
        << QStringLiteral("Cashier")
        << QStringLiteral("Subtotal")
        << QStringLiteral("Discount")
        << QStringLiteral("Net Payable")
        << QStringLiteral("Payment Mode")
        << QStringLiteral("Status"));
    m_tableInvoices->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableInvoices->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tableInvoices->verticalHeader()->setVisible(false);
    m_tableInvoices->verticalHeader()->setDefaultSectionSize(22);
    m_tableInvoices->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableInvoices->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableInvoices->setAlternatingRowColors(true);
    m_tableInvoices->setStyleSheet(tableStyle);
    l1->addWidget(m_tableInvoices);

    // Footer for Tab 1
    QFrame *footerInvoices = new QFrame(tabInvoices);
    footerInvoices->setFixedHeight(28);
    footerInvoices->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 2px; }"
    ));
    QHBoxLayout *footerInvLayout = new QHBoxLayout(footerInvoices);
    footerInvLayout->setContentsMargins(6, 1, 6, 1);
    m_lblInvoicesSummary = new QLabel(footerInvoices);
    m_lblInvoicesSummary->setStyleSheet(QStringLiteral("font-weight: bold; color: #000080; font-size: 8.5pt;"));
    footerInvLayout->addWidget(m_lblInvoicesSummary);
    footerInvLayout->addStretch();
    l1->addWidget(footerInvoices);

    m_tabs->addTab(tabInvoices, QStringLiteral("🧾 1. Completed Sales Invoices"));

    // Tab 2: Held Bills
    QWidget *tabHeld = new QWidget(m_tabs);
    QVBoxLayout *l2 = new QVBoxLayout(tabHeld);
    l2->setContentsMargins(6, 6, 6, 6);
    l2->setSpacing(4);

    m_tableHeld = new QTableWidget(tabHeld);
    m_tableHeld->setColumnCount(4);
    m_tableHeld->setHorizontalHeaderLabels(QStringList()
        << QStringLiteral("Hold ID")
        << QStringLiteral("Customer / Description")
        << QStringLiteral("Total Items")
        << QStringLiteral("Status"));
    m_tableHeld->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableHeld->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableHeld->verticalHeader()->setVisible(false);
    m_tableHeld->verticalHeader()->setDefaultSectionSize(22);
    m_tableHeld->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableHeld->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableHeld->setAlternatingRowColors(true);
    m_tableHeld->setStyleSheet(tableStyle);
    l2->addWidget(m_tableHeld);

    m_tabs->addTab(tabHeld, QStringLiteral("⏸️ 2. Held Bills"));

    mainLayout->addWidget(m_tabs, 1);

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

    QPushButton *bF2 = new QPushButton(QStringLiteral("F2 - Search"), fnBar);
    bF2->setStyleSheet(fnBtnStyle);
    bF2->setShortcut(QKeySequence(Qt::Key_F2));
    connect(bF2, &QPushButton::clicked, this, [this]() {
        if (m_txtSearch) {
            m_txtSearch->setFocus();
            m_txtSearch->selectAll();
        }
    });

    QPushButton *bF5 = new QPushButton(QStringLiteral("F5 - View Details"), fnBar);
    bF5->setStyleSheet(fnBtnStyle);
    bF5->setShortcut(QKeySequence(Qt::Key_F5));
    connect(bF5, &QPushButton::clicked, this, &SalesPage::onViewInvoiceDetailsClicked);

    QPushButton *bF7 = new QPushButton(QStringLiteral("F7 - Return / Refund"), fnBar);
    bF7->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFEBEE, stop:1 #FFCDD2); "
        "color: #B71C1C; border: 1px solid #EF9A9A; border-radius: 2px; padding: 2px 10px; font-size: 8.5pt; font-weight: bold; } "
        "QPushButton:hover { background: #FFCDD2; }"
    ));
    bF7->setShortcut(QKeySequence(Qt::Key_F7));
    connect(bF7, &QPushButton::clicked, this, &SalesPage::onProcessReturnClicked);

    QPushButton *bF9 = new QPushButton(QStringLiteral("F9 - Export CSV"), fnBar);
    bF9->setStyleSheet(fnBtnStyle);
    bF9->setShortcut(QKeySequence(Qt::Key_F9));
    connect(bF9, &QPushButton::clicked, this, &SalesPage::onExportCsvClicked);

    QPushButton *bEsc = new QPushButton(QStringLiteral("Esc - Cancel"), fnBar);
    bEsc->setStyleSheet(fnBtnStyle);
    bEsc->setShortcut(QKeySequence(Qt::Key_Escape));
    connect(bEsc, &QPushButton::clicked, this, [this]() {
        if (m_txtSearch) m_txtSearch->clear();
    });

    fnLayout->addWidget(bF1);
    fnLayout->addWidget(bF2);
    fnLayout->addWidget(bF5);
    fnLayout->addWidget(bF7);
    fnLayout->addWidget(bF9);
    fnLayout->addStretch();
    fnLayout->addWidget(bEsc);

    mainLayout->addWidget(fnBar);

    connect(m_txtSearch, &QLineEdit::textChanged, this, &SalesPage::onSearchChanged);
    connect(btnDetails, &QPushButton::clicked, this, &SalesPage::onViewInvoiceDetailsClicked);
    connect(btnReturn, &QPushButton::clicked, this, &SalesPage::onProcessReturnClicked);
    connect(btnRefresh, &QPushButton::clicked, this, &SalesPage::reloadData);
    connect(m_tableInvoices, &QTableWidget::cellDoubleClicked, this, &SalesPage::onViewInvoiceDetailsClicked);
}

void SalesPage::switchToTab(int tabIndex) {
    if (m_tabs && tabIndex >= 0 && tabIndex < m_tabs->count()) {
        m_tabs->setCurrentIndex(tabIndex);
    }
}

void SalesPage::reloadData() {
    onSearchChanged();
    loadHeldBills();
}

void SalesPage::onSearchChanged() {
    QString q = m_txtSearch->text().trimmed();
    m_currentInvoices = DatabaseManager::instance().searchInvoices(q);
    m_tableInvoices->setRowCount(m_currentInvoices.size());

    double netSum = 0.0;
    QString cur = DatabaseManager::instance().currencySymbol();

    for (int r = 0; r < m_currentInvoices.size(); ++r) {
        const auto& inv = m_currentInvoices[r];
        netSum += inv.netPayable;

        QTableWidgetItem *invItem = new QTableWidgetItem(inv.invoiceNo);
        invItem->setForeground(QBrush(QColor("#000080")));
        invItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_tableInvoices->setItem(r, 0, invItem);

        m_tableInvoices->setItem(r, 1, new QTableWidgetItem(inv.createdAt));
        m_tableInvoices->setItem(r, 2, new QTableWidgetItem(inv.customerName));
        m_tableInvoices->setItem(r, 3, new QTableWidgetItem(inv.cashierName));

        QTableWidgetItem *subItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(inv.subtotal, 0, 'f', 2));
        subItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_tableInvoices->setItem(r, 4, subItem);

        QTableWidgetItem *discItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(inv.discountAmount, 0, 'f', 2));
        discItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_tableInvoices->setItem(r, 5, discItem);

        QTableWidgetItem *netItem = new QTableWidgetItem(QStringLiteral("%1%2").arg(cur).arg(inv.netPayable, 0, 'f', 2));
        netItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        netItem->setForeground(QBrush(QColor("#000080")));
        netItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_tableInvoices->setItem(r, 6, netItem);

        QTableWidgetItem *modeItem = new QTableWidgetItem(inv.paymentMode);
        modeItem->setTextAlignment(Qt::AlignCenter);
        m_tableInvoices->setItem(r, 7, modeItem);

        QTableWidgetItem *statItem = new QTableWidgetItem(inv.paymentStatus);
        statItem->setTextAlignment(Qt::AlignCenter);
        if (inv.paymentStatus == "Returned") {
            statItem->setForeground(QBrush(QColor("#B71C1C")));
            statItem->setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::Bold));
        } else {
            statItem->setForeground(QBrush(QColor("#1B5E20")));
            statItem->setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::Bold));
        }
        m_tableInvoices->setItem(r, 8, statItem);
    }

    m_lblInvoicesSummary->setText(QStringLiteral("Total Invoices: %1 | Net Sales Volume: %2%3")
        .arg(m_currentInvoices.size())
        .arg(cur)
        .arg(netSum, 0, 'f', 2));
}

void SalesPage::loadHeldBills() {
    auto held = DatabaseManager::instance().getHeldBills();
    m_tableHeld->setRowCount(held.size());

    for (int r = 0; r < held.size(); ++r) {
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(held[r].first));
        idItem->setForeground(QBrush(QColor("#000080")));
        idItem->setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        m_tableHeld->setItem(r, 0, idItem);

        m_tableHeld->setItem(r, 1, new QTableWidgetItem(held[r].second));
        m_tableHeld->setItem(r, 2, new QTableWidgetItem(QStringLiteral("Held Cart")));

        QTableWidgetItem *statItem = new QTableWidgetItem(QStringLiteral("Active"));
        statItem->setTextAlignment(Qt::AlignCenter);
        statItem->setForeground(QBrush(QColor("#E65100")));
        m_tableHeld->setItem(r, 3, statItem);
    }
}

void SalesPage::onViewInvoiceDetailsClicked() {
    int row = m_tableInvoices->currentRow();
    if (row < 0 || row >= m_currentInvoices.size()) {
        QMessageBox::warning(this, QStringLiteral("Select Invoice"), QStringLiteral("Please select an invoice from the table to inspect."));
        return;
    }

    const auto& inv = m_currentInvoices[row];
    auto items = DatabaseManager::instance().getInvoiceItems(inv.id);

    ReceiptDialog receipt(
        inv.invoiceNo,
        inv.customerName,
        items,
        inv.subtotal,
        inv.discountAmount,
        inv.taxAmount,
        inv.roundOff,
        inv.netPayable,
        inv.tenderedAmount,
        inv.changeDue,
        inv.paymentMode,
        this
    );
    receipt.exec();
}

void SalesPage::onProcessReturnClicked() {
    int row = m_tableInvoices->currentRow();
    if (row < 0 || row >= m_currentInvoices.size()) {
        QMessageBox::warning(this, QStringLiteral("Select Invoice"), QStringLiteral("Please select an invoice to return."));
        return;
    }

    const auto& inv = m_currentInvoices[row];
    if (inv.paymentStatus == "Returned") {
        QMessageBox::information(this, QStringLiteral("Already Returned"), QStringLiteral("This invoice has already been returned and refunded."));
        return;
    }

    bool ok = false;
    QString reason = QInputDialog::getItem(
        this,
        QStringLiteral("Sales Return - %1").arg(inv.invoiceNo),
        QStringLiteral("Select return reason:"),
        QStringList() << QStringLiteral("Customer Changed Mind")
                      << QStringLiteral("Wrong Item / Size")
                      << QStringLiteral("Defective / Damaged Item")
                      << QStringLiteral("Billing Correction"),
        0,
        false,
        &ok
    );
    if (!ok) return;

    QString refundMode = QInputDialog::getItem(
        this,
        QStringLiteral("Refund Method"),
        QStringLiteral("Select refund tender:"),
        QStringList() << QStringLiteral("Cash") << QStringLiteral("Store Credit") << QStringLiteral("Original Tender"),
        0,
        false,
        &ok
    );
    if (!ok) return;

    if (DatabaseManager::instance().processSalesReturn(inv.id, inv.invoiceNo, 1, inv.netPayable, reason, refundMode)) {
        QMessageBox::information(this, QStringLiteral("Return Processed"), QStringLiteral("Invoice %1 returned successfully. Stock restored and refund of $%2 issued via %3.").arg(inv.invoiceNo).arg(inv.netPayable, 0, 'f', 2).arg(refundMode));
        reloadData();
    } else {
        QMessageBox::critical(this, QStringLiteral("Error"), QStringLiteral("Failed to process sales return."));
    }
}

void SalesPage::onExportCsvClicked() {
    if (m_tableInvoices->rowCount() == 0) {
        QMessageBox::information(this, QStringLiteral("Export CSV"), QStringLiteral("No sales invoices available to export."));
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(
        this, QStringLiteral("Export Sales Invoices to CSV"), QStringLiteral("SalesInvoices.csv"),
        QStringLiteral("CSV Files (*.csv)")
    );
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, QStringLiteral("Export Failed"), QStringLiteral("Cannot open file for writing."));
        return;
    }

    QTextStream out(&file);
    QStringList headerLabels;
    for (int c = 0; c < m_tableInvoices->columnCount(); ++c) {
        headerLabels << QStringLiteral("\"%1\"").arg(m_tableInvoices->horizontalHeaderItem(c)->text());
    }
    out << headerLabels.join(QStringLiteral(",")) << "\n";

    for (int r = 0; r < m_tableInvoices->rowCount(); ++r) {
        QStringList rowValues;
        for (int c = 0; c < m_tableInvoices->columnCount(); ++c) {
            QTableWidgetItem *item = m_tableInvoices->item(r, c);
            QString text = item ? item->text() : QString();
            text.replace(QStringLiteral("\""), QStringLiteral("\"\""));
            rowValues << QStringLiteral("\"%1\"").arg(text);
        }
        out << rowValues.join(QStringLiteral(",")) << "\n";
    }

    file.close();
    QMessageBox::information(this, QStringLiteral("Export Completed"),
        QStringLiteral("Successfully exported %1 sales invoices to:\n%2")
        .arg(m_tableInvoices->rowCount())
        .arg(filePath));
}
