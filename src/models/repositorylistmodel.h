#ifndef REPOSITORYLISTMODEL_H
#define REPOSITORYLISTMODEL_H

#include "domain/repository.h"

#include <QAbstractListModel>
#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QVariant>

// 仓库列表模型——将 QList<Repository> 包装为 QAbstractListModel，
// 供 RepositorySidebar 显示仓库名、路径和当前分支。
class RepositoryListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // 规范要求的自定义数据角色
    enum Role {
        NameRole = Qt::UserRole + 1, // 仓库名称
        PathRole,                    // 本地仓库路径
        CurrentBranchRole,           // 当前分支名

        // ---- 扩展角色 ----
        RepositoryRole,      // 完整 Repository 对象
        IdRole,              // 仓库唯一标识
        DisplayNameRole,     // 用于界面展示的名称
        DefaultBranchRole,   // 默认分支
        RemoteNameRole,      // 默认远程名
        RemoteUrlRole,       // 默认远程地址
        ProviderRole,        // 托管平台枚举值 (int)
        ProviderTextRole,    // 托管平台文本
        StateRole,           // 仓库状态枚举值 (int)
        StateTextRole,       // 仓库状态文本
        LastOpenedAtRole,    // 最近打开时间 (QDateTime)
        ValidRole,           // 是否为可用仓库 (bool)
        MissingRole,         // 是否路径缺失 (bool)
        HasRemoteRole        // 是否配置远程仓库 (bool)
    };

public:
    explicit RepositoryListModel(QObject* parent = nullptr);

    // ---- QAbstractListModel 接口 ----
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QHash<int, QByteArray> roleNames() const override;

    // ---- 规范要求的方法 ----

    // 整体替换仓库列表
    void setRepositories(const QList<Repository>& repositories);

    // 添加仓库；路径重复则更新
    void addRepository(const Repository& repository);

    // 按本地路径移除仓库
    void removeRepository(const QString& path);

    // 获取指定行的仓库；行无效时返回空对象
    Repository repositoryAt(int row) const;

    // 清空仓库列表
    void clear();

    // ---- 扩展方法 ----

    QList<Repository> repositories() const; // 返回当前所有仓库

    bool isEmpty() const;  // 判断列表是否为空
    int count() const;     // 返回仓库数量

    Repository repositoryForPath(const QString& localPath) const; // 按路径查找仓库
    QModelIndex indexForPath(const QString& localPath) const;     // 按路径获取模型索引
    int indexOfPath(const QString& localPath) const;              // 按路径获取行号
    bool containsPath(const QString& localPath) const;            // 是否包含指定路径

    bool updateRepository(const Repository& repository);   // 按路径更新
    bool upsertRepository(const Repository& repository);   // 新增或更新，更新返回 true

    bool removeRepositoryAt(int row); // 按行号移除

signals:
    void repositoriesChanged();                              // 列表结构或内容变化
    void repositoryAdded(const Repository& repository);      // 新仓库添加
    void repositoryUpdated(const Repository& repository);    // 仓库内容更新
    void repositoryRemoved(const QString& localPath);        // 仓库移除

private:
    bool isValidRow(int row) const;             // 判断行号是否有效
    void emitRepositoryChanged(int row);        // 通知指定行数据已变化

    static QString normalizedPath(const QString& localPath); // 规范化仓库路径

private:
    QList<Repository> m_repositories; // 当前持有的仓库列表
};

#endif // REPOSITORYLISTMODEL_H
