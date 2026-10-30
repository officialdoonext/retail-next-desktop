#ifndef GLOBALSEARCHDIALOG_H
#define GLOBALSEARCHDIALOG_H

#include <QDialog>

class QLineEdit;
class QTableWidget;

class GlobalSearchDialog : public QDialog {
    Q_OBJECT

public:
    explicit GlobalSearchDialog(QWidget *parent = nullptr);

signals:
    void productSelected(int productId);
    void invoiceSelected(int invoiceId);
    void customerSelected(int customerId);

private slots:
    void onSearchTextChanged(const QString& text);
    void onItemDoubleClicked(int row, int col);

private:
    QLineEdit *m_searchEdit;
    QTableWidget *m_resultsTable;
};

#endif // GLOBALSEARCHDIALOG_H

