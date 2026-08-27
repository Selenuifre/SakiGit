#ifndef REPOSITORYSERVICE_H
#define REPOSITORYSERVICE_H

#include "domain/repository.h"
#include "infrastructure/result.h"

#include <QList>
#include <QString>
#include <QStringList>

#include <optional>

class GitService;
class SettingsService;

class RepositoryService
{
public:
    // 创建仓库服务，依赖 GitService 和 SettingsService
    RepositoryService(GitService* gitService,
                      SettingsService* settingsService);

    // 打开一个本地 Git 仓库
    Result<Repository> openRepository(const QString& path);

    // 初始化一个新的 Git 仓库
    Result<Repository> initRepository(const QString& path);

    // 克隆远程仓库到本地
    Result<Repository> cloneRepository(const QString& url,
                                       const QString& targetPath);

    // 关闭当前仓库
    Result<void> closeRepository(const QString& path);

    // 返回当前打开的仓库
    Result<Repository> currentRepository() const;

    // 返回指定仓库的当前分支
    Result<QString> currentBranch(const QString& repoPath) const;

    // 判断指定路径是否为 Git 仓库
    Result<bool> isGitRepository(const QString& path) const;

    // 返回最近打开的仓库路径列表
    QStringList recentRepositoryPaths() const;

    // 返回最近打开的仓库列表
    QList<Repository> recentRepositories() const;

    // 添加一个仓库到最近列表
    void addRecentRepository(const Repository& repository);

    // 移除一个仓库路径
    void removeRecentRepository(const QString& path);

private:
    // Git 核心服务
    GitService* m_gitService;

    // 用户设置服务
    SettingsService* m_settingsService;

    // 当前打开的仓库，没有打开仓库时值为 nullopt
    std::optional<Repository> m_currentRepository;
};

#endif // REPOSITORYSERVICE_H
