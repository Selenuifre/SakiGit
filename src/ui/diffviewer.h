#ifndef DIFFVIEWER_H
#define DIFFVIEWER_H

#include <QWidget>

class QAbstractItemModel;
class QLabel;
class QTableView;

// Diff 查看器——将 DiffLineModel 以表格形式展示，自动适配列宽并着色。
// 规范接口：setModel(DiffLineModel*)
class DiffViewer : public QWidget
{
    Q_OBJECT

public:
    explicit DiffViewer(QWidget* parent = nullptr);

    // 设置 diff 数据源（DiffLineModel，继承自 QAbstractTableModel）
    void setModel(QAbstractItemModel* model);

    // 返回当前模型
    QAbstractItemModel* model() const;

    // 返回内部 QTableView，供外部做样式定制
    QTableView* tableView() const;

    // 清空 diff 显示
    void clear();

    // 设置空数据提示文本
    void setPlaceholderText(const QString& text);

    // 刷新视图状态（强制刷新 QTableView 可见性）
    void updateViewState();

private:

    QLabel* m_placeholderLabel;
    QTableView* m_tableView;
};

#endif // DIFFVIEWER_H
