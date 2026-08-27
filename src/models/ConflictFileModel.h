#ifndef CONFLICTFILEMODEL_H
#define CONFLICTFILEMODEL_H

#include "BaseListModel.h"
#include "domain/ConflictFile.h"

#include <QHash>
#include <QStringList>
#include <vector>

// 冲突文件列表模型，供 ConflictPage 展示冲突文件清单
class ConflictFileModel : public BaseListModel<ConflictFile>
{
    Q_OBJECT

public:
    enum Role {
        FilePathRole = Qt::UserRole + 1,
        ConflictTypeRole,
        ConflictFileRole
    };

    explicit ConflictFileModel(QObject* parent = nullptr)
        : BaseListModel(parent) {}

    // ---- BaseListModel 接口实现 ----

    QHash<int, QByteArray> roleNames() const override
    {
        QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
        roles.insert(FilePathRole, "filePath");
        roles.insert(ConflictTypeRole, "conflictType");
        roles.insert(ConflictFileRole, "conflictFile");
        return roles;
    }

    QVariant dataForRole(const ConflictFile& cf, int role) const override
    {
        switch (role) {
        case Qt::DisplayRole:
        case FilePathRole:
            return cf.path();
        case ConflictTypeRole:
            return cf.conflictType();
        case ConflictFileRole:
            return QVariant::fromValue(cf);
        default:
            return {};
        }
    }

    // ---- 类型安全的便捷访问器 ----

    void setConflictFiles(const std::vector<ConflictFile>& files)
    {
        // 过滤无效条目后设置
        beginResetModel();
        m_items.clear();
        for (const auto& f : files) {
            if (f.isValid()) m_items.push_back(f);
        }
        endResetModel();
        onItemsChanged();
    }

    ConflictFile conflictFileAt(int row) const { return itemAt(row); }
    QString filePathAt(int row) const
    {
        const ConflictFile cf = itemAt(row);
        return cf.isValid() ? cf.path() : QString();
    }

    // 返回冲突文件路径列表
    QStringList filePaths() const
    {
        QStringList paths;
        for (const auto& f : m_items)
            paths.append(f.path());
        return paths;
    }

signals:
    void conflictFilesChanged();

protected:
    void onItemsChanged() override { emit conflictFilesChanged(); }
};

#endif // CONFLICTFILEMODEL_H
