#ifndef PRODUCTSPAGE_H
#define PRODUCTSPAGE_H

#include <QWidget>
#include "../../database/DatabaseManager.h"

class QLineEdit;
class QComboBox;
class QTableWidget;
class QLabel;

class ProductsPage : public QWidget {
    Q_OBJECT

public:
    explicit ProductsPage(QWidget *parent = nullptr);

public slots:
    void reloadData();
    void onAddProductClicked();
    void onEditProductClicked();
    void onDeleteProductClicked();
    void onBulkPriceUpdateClicked();
    void onPrintBarcodeClicked();
    void onExportCsvClicked();
    void onImportCsvClicked();
    void filterByCategory(int catId);

private slots:
    void onSearchChanged();
    void onTableDoubleClicked(int row, int col);

private:
    void setupUI();

    QLineEdit *m_txtSearch;
    QComboBox *m_comboCategory;
    QComboBox *m_comboStockStatus;
    QTableWidget *m_tableProducts;
    QLabel *m_lblSummary;

    QList<RetailNext::ProductItem> m_currentProducts;
};

#endif // PRODUCTSPAGE_H

