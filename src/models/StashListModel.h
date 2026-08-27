#ifndef STASHLISTMODEL_H
#define STASHLISTMODEL_H

#include "BaseListModel.h"
#include "domain/Stash.h"

#include <QHash>
#include <vector>

class StashListModel : public BaseListModel<Stash>
{
    Q_OBJECT

public:
    enum Role {
        IndexRole = Qt::UserRole + 1,
        NameRole,
        MessageRole,
        BranchRole
    };

    explicit StashListModel(QObject* parent = nullptr)
        : BaseListModel(parent) {}

    // ---- BaseListModel 接口实现 ----

    QHash<int, QByteArray> roleNames() const override
    {
        QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
        roles.insert(IndexRole, "stashIndex");
        roles.insert(NameRole, "name");
        roles.insert(MessageRole, "message");
        roles.insert(BranchRole, "branch");
        return roles;
    }

    QVariant dataForRole(const Stash& stash, int role) const override
    {
        switch (role) {
        case Qt::DisplayRole:
        case NameRole:
            return stash.displayName();
        case IndexRole:
            return stash.index();
        case MessageRole:
            return stash.message();
        case BranchRole:
            return stash.displayBranch();
        case Qt::ToolTipRole:
            return stash.message();
        default:
            return {};
        }
    }

    // ---- 类型安全的便捷访问器 ----

    void setStashes(const std::vector<Stash>& stashes) { setItems(stashes); }
    Stash stashAt(int row) const { return itemAt(row); }
    int stashIndexAt(int row) const { return stashAt(row).index(); }
};

#endif // STASHLISTMODEL_H
