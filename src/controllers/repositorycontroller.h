#ifndef REPOSITORYCONTROLLER_H
#define REPOSITORYCONTROLLER_H

#include "domain/repository.h"
#include "infrastructure/result.h"

#include "BaseController.h"
#include <QString>

class GitTaskRunner;
class RepositoryListModel;
class RepositoryService;

class RepositoryController : public BaseController
{
    Q_OBJECT

public:
    // 使用仓库服务和仓库列表模型构造控制器
    explicit RepositoryController(RepositoryService* repositoryService,
                                  RepositoryListModel* repositoryListModel,
                                  QObject* parent = nullptr);

    // 注入异步任务执行器，使 openRepository 不阻塞 UI
    void setGitTaskRunner(GitTaskRunner* taskRunner);

    // 打开本地仓库
    void openRepository(const QString& path);

    // 初始化新仓库
    void initRepository(const QString& path);

    // 克隆远程仓库
    void cloneRepository(const QString& url, const QString& targetPath);

    // 关闭仓库
    Result<void> closeRepository(const QString& path);

    // 加载最近仓库列表到模型
    void loadRecentRepositories();

    // 添加仓库到最近列表
    void addRecentRepository(const Repository& repository);

    // 从最近列表移除仓库
    void removeRecentRepository(const QString& path);

    // 清空最近仓库列表
    void clearRecentRepositories();

    // 判断指定路径是否为 Git 仓库
    Result<bool> isGitRepository(const QString& path) const;

    // 返回仓库列表模型
    RepositoryListModel* repositoryListModel() const;

signals:
    // 仓库打开成功
    void repositoryOpened(const Repository& repository);

    // 仓库打开失败
    void openFailed(const QString& path, const QString& errorMessage);

    // 仓库初始化成功
    void repositoryInitialized(const Repository& repository);

    // 仓库克隆成功
    void repositoryCloned(const Repository& repository);

    // 仓库克隆失败
    void cloneFailed(const QString& url, const QString& errorMessage);

    // 最近仓库列表加载完成
    void recentRepositoriesLoaded();

    // 操作发生错误

private slots:
    // GitTaskRunner 异步打开仓库完成后的回调
    void onRepositoryOpened(bool success,
                            const QString& repositoryPath,
                            const Repository& repository,
                            const QString& errorMessage);

    // GitTaskRunner 异步初始化/克隆完成后的回调
    void onRepositoryInitialized(bool success, const QString& path, const Repository& repository, const QString& errorMessage);
    void onRepositoryCloned(bool success, const QString& url, const Repository& repository, const QString& errorMessage);

private:
    // 仓库业务服务
    RepositoryService* m_repositoryService;

    // 仓库列表模型
    RepositoryListModel* m_repositoryListModel;

    // 异步任务执行器（可选注入，用于非阻塞 openRepository）
    GitTaskRunner* m_taskRunner = nullptr;
};

#endif // REPOSITORYCONTROLLER_H
