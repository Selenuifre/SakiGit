#include "PageHeaderWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QStyle>

PageHeaderWidget::PageHeaderWidget(const QString& title,
                                     bool showStatusLabel,
                                     QWidget* parent)
    : QWidget(parent),
      m_titleLabel(nullptr),
      m_statusLabel(nullptr),
      m_refreshButton(nullptr)
{
    setupUi(title, showStatusLabel);
}

void PageHeaderWidget::setStatusText(const QString& text)
{
    if (m_statusLabel) {
        m_statusLabel->setText(text);
    }
}

QString PageHeaderWidget::statusText() const
{
    return m_statusLabel ? m_statusLabel->text() : QString();
}

void PageHeaderWidget::setStatusStyleSheet(const QString& styleSheet)
{
    if (m_statusLabel) {
        m_statusLabel->setStyleSheet(styleSheet);
    }
}

QPushButton* PageHeaderWidget::refreshButton() const
{
    return m_refreshButton;
}

QLabel* PageHeaderWidget::titleLabel() const
{
    return m_titleLabel;
}

QLabel* PageHeaderWidget::statusLabel() const
{
    return m_statusLabel;
}

void PageHeaderWidget::setupUi(const QString& title, bool showStatusLabel)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName(QStringLiteral("sectionLabel"));
    m_titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    if (showStatusLabel) {
        m_statusLabel = new QLabel(this);
        m_statusLabel->setStyleSheet(
            QStringLiteral("color: #F0883E; font-weight: bold;"));
        layout->addWidget(m_titleLabel);
        layout->addWidget(m_statusLabel, 1);
    } else {
        layout->addWidget(m_titleLabel, 1);
    }

    m_refreshButton = new QPushButton(tr("Refresh"), this);
    m_refreshButton->setToolTip(tr("Refresh"));
    layout->addWidget(m_refreshButton);

    connect(m_refreshButton, &QPushButton::clicked,
            this, &PageHeaderWidget::refreshRequested);
}
