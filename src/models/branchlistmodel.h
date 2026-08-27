#ifndef BRANCHLISTMODEL_H
#define BRANCHLISTMODEL_H

#include "BaseListModel.h"
#include "domain/branch.h"

#include <QHash>
#include <vector>

// 分支列表模型——将 std::vector<Branch> 包装为 QAbstractListModel，
// 供 BranchesPage / BranchController 直接绑定显示。
class BranchListModel : public BaseListModel<Branch>
{
    Q_OBJECT

public:
    // 视图层可读取的自定义数据角色
    enum Role {
        NameRole = Qt::UserRole + 1, // 分支名称（DisplayRole 同）
        IsCurrentRole,               // 是否为当前分支 (bool)
        IsRemoteRole                 // 是否为远程分支 (bool)
    };

    explicit BranchListModel(QObject* parent = nullptr)
        : BaseListModel(parent) {}

    // ---- BaseListModel 接口实现 ----

    QHash<int, QByteArray> roleNames() const override
    {
        QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
        roles.insert(NameRole, "name");
        roles.insert(IsCurrentRole, "isCurrent");
        roles.insert(IsRemoteRole, "isRemote");
        return roles;
    }

    QVariant dataForRole(const Branch& branch, int role) const override
    {
        switch (role) {
        case Qt::DisplayRole:
        case NameRole:
            return branch.displayName();
        case Qt::ToolTipRole:
            return branch.fullName().isEmpty() ? branch.name() : branch.fullName();
        case IsCurrentRole:
            return branch.isCurrent();
        case IsRemoteRole:
            return branch.isRemote();
        default:
            return {};
        }
    }

    // ---- 类型安全的便捷访问器 ----

    void setBranches(const std::vector<Branch>& branches) { setItems(branches); }
    Branch branchAt(int row) const { return itemAt(row); }
    QString branchNameAt(int row) const { return branchAt(row).name(); }
};

#endif // BRANCHLISTMODEL_H
