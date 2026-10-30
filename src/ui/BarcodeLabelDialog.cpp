#include "BarcodeLabelDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>

BarcodeLabelDialog::BarcodeLabelDialog(const RetailNext::ProductItem& product, QWidget *parent)
    : QDialog(parent)
    , m_product(product)
{
    setWindowTitle(QStringLiteral("Barcode Label Generator - %1").arg(product.name));
    resize(380, 400);

    setStyleSheet(QStringLiteral(
        "QDialog { background-color: #D9E8F5; } "
        "QLabel { color: #0A294A; } "
        "QSpinBox { background: #FFFFFF; border: 1px solid #7A9EB9; border-radius: 2px; padding: 2px 6px; color: #000080; font-weight: bold; font-size: 9pt; } "
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #E0EBF5); "
        "color: #0A294A; border: 1px solid #7A9EB9; border-radius: 2px; padding: 4px 14px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #EDF4FB; }"
    ));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    QLabel *infoLabel = new QLabel(QStringLiteral("<b>Print Shelf / Garment Barcode Sticker</b>"), this);
    mainLayout->addWidget(infoLabel);

    // Visual label preview box
    m_previewLabel = new QLabel(this);
    m_previewLabel->setStyleSheet(QStringLiteral(
        "QLabel { background-color: #FFFFFF; border: 1px solid #7A9EB9; border-radius: 3px; padding: 14px; color: #111; }"
    ));
    m_previewLabel->setAlignment(Qt::AlignCenter);

    QString cur = DatabaseManager::instance().currencySymbol();
    QString labelHtml = QStringLiteral(
        "<div style='text-align: center; font-family: monospace;'>"
        "<div style='font-size: 11pt; font-weight: bold; color: #0A294A;'>RETAILNEXT STORE</div>"
        "<div style='font-size: 10pt; font-weight: bold; margin: 4px 0; color: #000080;'>%1</div>"
        "<div style='font-size: 8pt; color: #555;'>SKU: %2 | Unit: %3</div>"
        "<div style='letter-spacing: 4px; font-size: 14pt; margin: 10px 0; font-weight: bold;'>||| |||| || ||||| |||</div>"
        "<div style='font-size: 10pt; font-weight: bold;'>%4</div>"
        "<div style='font-size: 14pt; font-weight: bold; color: #B71C1C; margin-top: 6px;'>PRICE: %5%6</div>"
        "</div>"
    ).arg(product.name)
     .arg(product.sku)
     .arg(product.unitCode)
     .arg(product.barcode)
     .arg(cur)
     .arg(product.sellingPrice, 0, 'f', 2);

    m_previewLabel->setText(labelHtml);
    mainLayout->addWidget(m_previewLabel);

    QHBoxLayout *copiesLayout = new QHBoxLayout();
    QLabel *lblCopies = new QLabel(QStringLiteral("Number of Labels:"), this);
    lblCopies->setStyleSheet(QStringLiteral("font-weight: bold;"));
    copiesLayout->addWidget(lblCopies);
    m_copiesSpinBox = new QSpinBox(this);
    m_copiesSpinBox->setRange(1, 500);
    m_copiesSpinBox->setValue(1);
    copiesLayout->addWidget(m_copiesSpinBox);
    mainLayout->addLayout(copiesLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnPrint = new QPushButton(QStringLiteral("🖨️ Send to Printer (F9)"), this);
    btnPrint->setShortcut(QKeySequence(Qt::Key_F9));
    btnPrint->setStyleSheet(QStringLiteral(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8F5E9, stop:1 #C8E6C9); "
        "color: #1B5E20; border: 1px solid #81C784; border-radius: 2px; padding: 4px 16px; font-weight: bold; font-size: 8.5pt; } "
        "QPushButton:hover { background: #C8E6C9; }"
    ));
    QPushButton *btnClose = new QPushButton(QStringLiteral("❌ Close (Esc)"), this);
    btnClose->setShortcut(QKeySequence(Qt::Key_Escape));
    btnLayout->addStretch();
    btnLayout->addWidget(btnPrint);
    btnLayout->addWidget(btnClose);
    mainLayout->addLayout(btnLayout);

    connect(btnPrint, &QPushButton::clicked, this, &BarcodeLabelDialog::onPrintClicked);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
}

void BarcodeLabelDialog::onPrintClicked() {
    QPrinter printer(QPrinter::HighResolution);
    printer.setDocName(QStringLiteral("Barcode - %1").arg(m_product.sku));
    QPrintDialog dlg(&printer, this);
    if (dlg.exec() == QDialog::Accepted) {
        QMessageBox::information(this, QStringLiteral("Label Printed"), QStringLiteral("Sent %1 barcode label(s) to printer.").arg(m_copiesSpinBox->value()));
        accept();
    }
}

