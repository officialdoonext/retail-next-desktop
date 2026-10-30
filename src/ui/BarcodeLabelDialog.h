#ifndef BARCODELABELDIALOG_H
#define BARCODELABELDIALOG_H

#include <QDialog>
#include "../database/DatabaseManager.h"

class QLabel;
class QSpinBox;
class QPushButton;

class BarcodeLabelDialog : public QDialog {
    Q_OBJECT

public:
    explicit BarcodeLabelDialog(const RetailNext::ProductItem& product, QWidget *parent = nullptr);

private slots:
    void onPrintClicked();

private:
    RetailNext::ProductItem m_product;
    QSpinBox *m_copiesSpinBox;
    QLabel *m_previewLabel;
};

#endif // BARCODELABELDIALOG_H

