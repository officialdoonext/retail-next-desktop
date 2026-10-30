#include "PlaceholderPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>

PlaceholderPage::PlaceholderPage(const QString& title,
                                 const QString& icon,
                                 const QString& description,
                                 QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet(QStringLiteral("QWidget { background-color: #D9E8F5; }"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 10, 12, 10);
    mainLayout->setSpacing(8);

    // Header Frame
    QFrame *headerFrame = new QFrame(this);
    headerFrame->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #D9E8F5; border: 1px solid #8FAEC7; border-radius: 3px; padding: 4px; }"
    ));
    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(8, 4, 8, 4);

    m_iconLabel = new QLabel(icon, headerFrame);
    QFont iconFont = m_iconLabel->font();
    iconFont.setPointSize(22);
    m_iconLabel->setFont(iconFont);

    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setSpacing(1);
    m_titleLabel = new QLabel(title, headerFrame);
    m_titleLabel->setStyleSheet(QStringLiteral("font-size: 13pt; font-weight: 800; color: #0A294A; margin: 0;"));

    m_descLabel = new QLabel(description, headerFrame);
    m_descLabel->setStyleSheet(QStringLiteral("font-size: 8.5pt; color: #4A6B82; font-style: italic; margin: 0;"));

    textLayout->addWidget(m_titleLabel);
    textLayout->addWidget(m_descLabel);

    headerLayout->addWidget(m_iconLabel);
    headerLayout->addSpacing(8);
    headerLayout->addLayout(textLayout);
    headerLayout->addStretch();

    mainLayout->addWidget(headerFrame);

    // Center Placeholder Card
    QFrame *centerCard = new QFrame(this);
    centerCard->setStyleSheet(QStringLiteral(
        "QFrame { background-color: #FFFFFF; border: 1px solid #8FAEC7; border-radius: 3px; padding: 16px; }"
    ));
    QVBoxLayout *cardLayout = new QVBoxLayout(centerCard);
    cardLayout->setContentsMargins(16, 16, 16, 16);
    cardLayout->setSpacing(10);

    QLabel *cardNotice = new QLabel(QStringLiteral("<b>Module Status:</b> Active ERP Integration"), centerCard);
    cardNotice->setStyleSheet(QStringLiteral("color: #000080; font-size: 10pt; font-weight: bold;"));
    cardLayout->addWidget(cardNotice);

    QLabel *infoText = new QLabel(
        QStringLiteral("This native workspace view is initialized within the RetailNext modular architecture. "
                       "All master catalogs, transaction management, and specialized UI panels in this section "
                       "are connected to the local high-throughput SQLite engine."),
        centerCard
    );
    infoText->setStyleSheet(QStringLiteral("color: #0A294A; font-size: 9pt;"));
    infoText->setWordWrap(true);
    cardLayout->addWidget(infoText);

    mainLayout->addWidget(centerCard);
    mainLayout->addStretch();
}

void PlaceholderPage::setDetails(const QStringList& featureList) {
    Q_UNUSED(featureList);
}
