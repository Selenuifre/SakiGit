#ifndef SYNCSERVICE_H
#define SYNCSERVICE_H

#include "infrastructure/result.h"
#include "BaseService.h"

#include <QString>
#include <QStringList>

class GitService;

class SyncService : public BaseService
{
public:
    // 创建同步服务，依赖 GitService
    explicit SyncService(GitService* gitService);

    // 从远程仓库获取最新信息
    Result<void> fetch(const QString& repoPath) const;

    // 拉取当前分支的远程更新
    Result<void> pull(const QString& repoPath) const;

    // 推送到远程仓库
    Result<void> push(const QString& repoPath) const;

    // 推送指定分支到指定远程
    Result<void> push(const QString& repoPath,
                      const QString& remote,
                      const QString& branch) const;

    // 列出所有 remote 名称
    Result<QStringList> listRemotes(const QString& repoPath) const;

    // 添加一个 remote
    Result<void> addRemote(const QString& repoPath,
                           const QString& name,
                           const QString& url) const;

    // 移除一个 remote
    Result<void> removeRemote(const QString& repoPath,
                              const QString& name) const;

};

#endif // SYNCSERVICE_H
