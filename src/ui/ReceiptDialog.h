#ifndef RECEIPTDIALOG_H
#define RECEIPTDIALOG_H

#include <QDialog>
#include <QList>
#include "../database/DatabaseManager.h"

class QTextBrowser;
class QPushButton;

class ReceiptDialog : public QDialog {
    Q_OBJECT

public:
    explicit ReceiptDialog(
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
        QWidget *parent = nullptr
    );

private slots:
    void onPrintClicked();

private:
    void generateReceiptHTML(
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
    );

    QTextBrowser *m_textBrowser;
    QPushButton *m_btnPrint;
    QPushButton *m_btnClose;
    QString m_htmlContent;
};

#endif // RECEIPTDIALOG_H

