#ifndef TOASTMANAGER_H
#define TOASTMANAGER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QWidget>

class QLabel;
class QVBoxLayout;

// 轻量级消息提示管理器——在父窗口底部显示自动消失的通知。
// 用法：ToastManager::showMessage(parent, "操作成功", 3000);
class ToastManager : public QObject
{
    Q_OBJECT

public:
    // 在 parent 右下角显示一条消息，timeoutMs 毫秒后自动消失。
    // 传入 0 则使用默认时长（3000ms）。
    static void showMessage(QWidget* parent, const QString& message, int timeoutMs = 3000);

    // 显示错误消息（红色背景，默认显示 5000ms）
    static void showError(QWidget* parent, const QString& message, int timeoutMs = 5000);

    // 显示成功消息（绿色背景）
    static void showSuccess(QWidget* parent, const QString& message, int timeoutMs = 3000);

private:
    explicit ToastManager(QWidget* parent);
    void show(const QString& message, const QString& styleClass, int timeoutMs);

    QLabel* m_label;
    QTimer m_timer;
};

#endif // TOASTMANAGER_H
