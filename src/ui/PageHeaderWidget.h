#ifndef PAGEHEADERWIDGET_H
#define PAGEHEADERWIDGET_H

#include <QWidget>

class QHBoxLayout;
class QLabel;
class QPushButton;

// 可复用的页面标题栏组件：标题 + 可选状态标签 + 刷新按钮。
// BranchesPage、StashPage、ConflictPage 共用此模式。

class PageHeaderWidget : public QWidget
{
    Q_OBJECT

public:
    // title: 页面标题文本
    // showStatusLabel: 是否显示状态标签（ConflictPage 需要）
    explicit PageHeaderWidget(const QString& title,
                               bool showStatusLabel = false,
                               QWidget* parent = nullptr);

    // 设置/获取状态标签文本
    void setStatusText(const QString& text);
    QString statusText() const;

    // 设置/获取样式表（用于状态标签颜色）
    void setStatusStyleSheet(const QString& styleSheet);

    // 获取刷新按钮和标题标签
    QPushButton* refreshButton() const;
    QLabel* titleLabel() const;
    QLabel* statusLabel() const;

signals:
    void refreshRequested();

private:
    void setupUi(const QString& title, bool showStatusLabel);

    QLabel* m_titleLabel;
    QLabel* m_statusLabel;
    QPushButton* m_refreshButton;
};

#endif // PAGEHEADERWIDGET_H
