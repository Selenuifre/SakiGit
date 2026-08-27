#ifndef DIFFLINEMODEL_H
#define DIFFLINEMODEL_H

#include "domain/diff.h"

#include <QAbstractTableModel>
#include <QHash>
#include <QModelIndex>
#include <QVariant>
#include <vector>
#include <QColor>

// Diff 行显示模型——将 std::vector<DiffLine> 包装为 QAbstractTableModel，
// 供 DiffViewer 以四列表格形式展示 diff 内容。
// 支持新增/删除行的前景色与背景色区分。
class DiffLineModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    // 视图层可读取的自定义数据角色
    enum Role {
        OldLineNumberRole = Qt::UserRole + 1, // 旧文件行号 (int)
        NewLineNumberRole,                    // 新文件行号 (int)
        ContentRole,                          // 行文本内容 (QString)
        LineTypeRole                          // 行类型枚举值 (int, DiffLine::Type)
    };

    // 表格四列定义
    enum Column {
        OldLineNumberColumn = 0, // 旧文件行号列
        NewLineNumberColumn,     // 新文件行号列
        ContentColumn,           // diff 内容列
        ColumnCount              // 总列数
    };

    explicit DiffLineModel(QObject* parent = nullptr);

    // QAbstractTableModel 接口
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 从 Diff 对象中提取所有文件的所有 hunk 行并显示
    void setDiff(const Diff& diff);

    // 直接设置 diff 行列表
    void setLines(const std::vector<DiffLine>& lines);

    // 获取指定行的 DiffLine；行无效时返回空对象
    DiffLine lineAt(int row) const;

    // 清空 diff 行列表
    void clear();

private:
    std::vector<DiffLine> m_lines; // 当前持有的 diff 行列表
};

#endif // DIFFLINEMODEL_H
