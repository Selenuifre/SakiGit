#include "toastmanager.h"

#include <QLabel>
#include <QVBoxLayout>

ToastManager::ToastManager(QWidget* parent)
    : QObject(parent),
    m_label(new QLabel(parent))
{
    m_label->setObjectName(QStringLiteral("toastLabel"));
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setWordWrap(true);
    m_label->setMinimumWidth(260);
    m_label->setMaximumWidth(480);
    m_label->hide();

    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, m_label, &QLabel::hide);
}

void ToastManager::showMessage(QWidget* parent, const QString& message, int timeoutMs)
{
    auto* mgr = new ToastManager(parent);
    mgr->show(message, QStringLiteral("default"), timeoutMs <= 0 ? 3000 : timeoutMs);
}

void ToastManager::showError(QWidget* parent, const QString& message, int timeoutMs)
{
    auto* mgr = new ToastManager(parent);
    mgr->show(message, QStringLiteral("error"), timeoutMs <= 0 ? 5000 : timeoutMs);
}

void ToastManager::showSuccess(QWidget* parent, const QString& message, int timeoutMs)
{
    auto* mgr = new ToastManager(parent);
    mgr->show(message, QStringLiteral("success"), timeoutMs <= 0 ? 3000 : timeoutMs);
}

void ToastManager::show(const QString& message, const QString& styleClass, int timeoutMs)
{
    QWidget* parent = qobject_cast<QWidget*>(this->parent());
    if (!parent) {
        deleteLater();
        return;
    }

    m_label->setText(message);

    QString style;
    if (styleClass == QStringLiteral("error")) {
        style = QStringLiteral(
            "QLabel { background-color: #3D1F1F; color: #F47067; border: 1px solid #F47067; "
            "border-radius: 6px; padding: 10px 18px; font-size: 13px; }");
    } else if (styleClass == QStringLiteral("success")) {
        style = QStringLiteral(
            "QLabel { background-color: #1F3D1F; color: #57AB5A; border: 1px solid #2DA44E; "
            "border-radius: 6px; padding: 10px 18px; font-size: 13px; }");
    } else {
        style = QStringLiteral(
            "QLabel { background-color: #1F2E3D; color: #79C0FF; border: 1px solid #539BF5; "
            "border-radius: 6px; padding: 10px 18px; font-size: 13px; }");
    }
    m_label->setStyleSheet(style);

    // 定位在父窗口底部居中
    m_label->adjustSize();
    const int x = (parent->width() - m_label->width()) / 2;
    const int y = parent->height() - m_label->height() - 30;
    m_label->move(qMax(x, 20), qMax(y, 20));
    m_label->raise();
    m_label->show();

    m_timer.start(timeoutMs);

    // 定时器触发后 self-destruct
    connect(&m_timer, &QTimer::timeout, this, &QObject::deleteLater);
}
