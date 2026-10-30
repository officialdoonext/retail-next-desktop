#include "ReceiptDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QDateTime>
#include <QPrinter>
#include <QPrintDialog>
#include <QMessageBox>

ReceiptDialog::ReceiptDialog(
    const QString& invoiceNo,
    const QString& customerName,
    const QList<RetailNext::CartItem>& items,
    double subtotal,
    double discount,
    double tax,
    double roundOff,
    double netPayable,
    double tendered,
    double change,
    const QString& paymentMode,
    QWidget *parent
) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Thermal Receipt Preview - %1").arg(invoiceNo));
    resize(420, 620);

    setStyleSheet(QStringLiteral(
        "QDialog { background-color: #D9E8F5; } "
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 4px 14px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; }"
    ));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    m_textBrowser = new QTextBrowser(this);
    m_textBrowser->setStyleSheet(QStringLiteral(
        "QTextBrowser { background-color: #FFFFFF; border: 1px solid #7A9EB9; font-family: 'Consolas', 'Courier New', monospace; font-size: 10pt; padding: 10px; color: #111; }"
    ));
    mainLayout->addWidget(m_textBrowser);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_btnPrint = new QPushButton(QStringLiteral("🖨️ Print Receipt (Ctrl+P)"), this);
    m_btnPrint->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_P));
    m_btnPrint->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 4px 16px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #C8E6C9; }"
    ));
    m_btnClose = new QPushButton(QStringLiteral("❌ Close (Esc)"), this);
    m_btnClose->setShortcut(QKeySequence(Qt::Key_Escape));

    btnLayout->addStretch();
    btnLayout->addWidget(m_btnPrint);
    btnLayout->addWidget(m_btnClose);
    mainLayout->addLayout(btnLayout);

    connect(m_btnPrint, &QPushButton::clicked, this, &ReceiptDialog::onPrintClicked);
    connect(m_btnClose, &QPushButton::clicked, this, &QDialog::accept);

    generateReceiptHTML(invoiceNo, customerName, items, subtotal, discount, tax, roundOff, netPayable, tendered, change, paymentMode);
}

void ReceiptDialog::generateReceiptHTML(
    const QString& invoiceNo,
    const QString& customerName,
    const QList<RetailNext::CartItem>& items,
    double subtotal,
    double discount,
    double tax,
    double roundOff,
    double netPayable,
    double tendered,
    double change,
    const QString& paymentMode
) {
    DatabaseManager &db = DatabaseManager::instance();
    QString store = db.storeName();
    QString cur = db.currencySymbol();
    QString header = db.getBusinessSetting(QStringLiteral("receipt_header"), QStringLiteral("RetailNext Flagship Store"));
    QString footer = db.getBusinessSetting(QStringLiteral("receipt_footer"), QStringLiteral("Thank you for shopping with us!"));
    QString dateStr = QDateTime::currentDateTime().toString(QStringLiteral("dd-MMM-yyyy hh:mm AP"));

    header.replace(QStringLiteral("\n"), QStringLiteral("<br>"));
    footer.replace(QStringLiteral("\n"), QStringLiteral("<br>"));

    QString html;
    html += QStringLiteral(
        "<html><body style='text-align: center; color: #111; font-family: monospace;'>"
        "<h2 style='margin: 0;'>%1</h2>"
        "<div style='font-size: 9pt; margin-bottom: 8px;'>%2</div>"
        "<div style='border-top: 1px dashed #444; border-bottom: 1px dashed #444; padding: 6px 0; text-align: left; font-size: 9.5pt;'>"
        "<b>Invoice:</b> %3<br>"
        "<b>Date:</b> %4<br>"
        "<b>Customer:</b> %5<br>"
        "<b>Cashier:</b> %6 | <b>Terminal:</b> %7"
        "</div>"
        "<table width='100%' style='margin-top: 8px; border-collapse: collapse; font-size: 9.5pt; text-align: left;'>"
        "<tr style='border-bottom: 1px solid #444;'>"
        "<th>Item</th><th style='text-align: center;'>Qty</th><th style='text-align: right;'>Price</th><th style='text-align: right;'>Total</th>"
        "</tr>"
    ).arg(store, header, invoiceNo, dateStr, customerName, db.currentUser(), db.registerCode());

    for (const auto& item : items) {
        html += QStringLiteral(
            "<tr>"
            "<td style='padding: 3px 0;'>%1</td>"
            "<td style='text-align: center;'>%2</td>"
            "<td style='text-align: right;'>%3%4</td>"
            "<td style='text-align: right;'>%3%5</td>"
            "</tr>"
        ).arg(item.name)
         .arg(item.qty, 0, 'f', item.unit == "PCS" ? 0 : 2)
         .arg(cur)
         .arg(item.unitPrice, 0, 'f', 2)
         .arg(item.lineTotal, 0, 'f', 2);
    }

    html += QStringLiteral(
        "</table>"
        "<div style='border-top: 1px dashed #444; margin-top: 8px; padding-top: 6px; font-size: 9.5pt; text-align: right;'>"
        "<div>Subtotal: %1%2</div>"
    ).arg(cur).arg(subtotal, 0, 'f', 2);

    if (discount > 0.0) {
        html += QStringLiteral("<div>Discount: -%1%2</div>").arg(cur).arg(discount, 0, 'f', 2);
    }
    if (tax > 0.0) {
        html += QStringLiteral("<div>Tax: %1%2</div>").arg(cur).arg(tax, 0, 'f', 2);
    }
    if (qAbs(roundOff) > 0.001) {
        html += QStringLiteral("<div>Round-off: %1%2</div>").arg(cur).arg(roundOff, 0, 'f', 2);
    }

    html += QStringLiteral(
        "<div style='font-size: 12pt; font-weight: bold; margin: 6px 0;'>TOTAL PAYABLE: %1%2</div>"
        "<div style='border-top: 1px dashed #888; padding-top: 4px;'>"
        "Payment Mode: %3<br>"
        "Amount Tendered: %1%4<br>"
        "Change Due: %1%5"
        "</div>"
        "</div>"
        "<div style='margin-top: 16px; border-top: 1px dashed #444; padding-top: 10px; font-size: 9pt;'>"
        "%6<br><br>"
        "<b>* * * * RETAILNEXT POS * * * *</b>"
        "</div></body></html>"
    ).arg(cur)
     .arg(netPayable, 0, 'f', 2)
     .arg(paymentMode)
     .arg(tendered, 0, 'f', 2)
     .arg(change, 0, 'f', 2)
     .arg(footer);

    m_htmlContent = html;
    m_textBrowser->setHtml(html);
}

void ReceiptDialog::onPrintClicked() {
    QPrinter printer(QPrinter::HighResolution);
    printer.setDocName(windowTitle());
    printer.setPageSize(QPageSize(QPageSize::Letter));

    QPrintDialog printDlg(&printer, this);
    if (printDlg.exec() == QDialog::Accepted) {
        m_textBrowser->print(&printer);
    }
}

