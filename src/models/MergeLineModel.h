#ifndef MERGELINEMODEL_H
#define MERGELINEMODEL_H

#include "BaseListModel.h"

#include <QList>
#include <QString>

// 三方合并行数据
enum class MergeLineSource {
    Base = 0,
    Ours,
    Theirs,
    Resolved
};

struct MergeLineData {
    QString text;
    MergeLineSource source = MergeLineSource::Base;
    bool isConflict = false;
};

// 三方合并行模型，每行来自 base/ours/theirs 其中一版
class MergeLineModel : public BaseListModel<MergeLineData, QList<MergeLineData>>
{
    Q_OBJECT

public:
    enum Role {
        TextRole = Qt::UserRole + 1,
        SourceRole,
        IsConflictRole
    };

    explicit MergeLineModel(QObject* parent = nullptr)
        : BaseListModel(parent) {}

    // ---- BaseListModel 接口实现 ----

    QHash<int, QByteArray> roleNames() const override
    {
        QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
        roles.insert(TextRole, "text");
        roles.insert(SourceRole, "source");
        roles.insert(IsConflictRole, "isConflict");
        return roles;
    }

    QVariant dataForRole(const MergeLineData& line, int role) const override
    {
        switch (role) {
        case Qt::DisplayRole:
        case TextRole:
            return line.text;
        case SourceRole:
            return static_cast<int>(line.source);
        case IsConflictRole:
            return line.isConflict;
        default:
            return {};
        }
    }

    // ---- 类型安全的便捷访问器 ----

    void setLines(const QList<MergeLineData>& lines) { setItems(lines); }
    MergeLineData lineAt(int row) const { return itemAt(row); }
};

#endif // MERGELINEMODEL_H
