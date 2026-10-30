#ifndef PLACEHOLDERPAGE_H
#define PLACEHOLDERPAGE_H

#include <QWidget>
#include <QString>

class QLabel;

class PlaceholderPage : public QWidget {
    Q_OBJECT

public:
    explicit PlaceholderPage(const QString& title,
                            const QString& icon,
                            const QString& description,
                            QWidget *parent = nullptr);

    void setDetails(const QStringList& featureList);

private:
    QLabel *m_titleLabel;
    QLabel *m_iconLabel;
    QLabel *m_descLabel;
};

#endif // PLACEHOLDERPAGE_H
