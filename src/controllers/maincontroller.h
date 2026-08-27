#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QString>

class BranchController;
class ChangesController;
class ConflictController;
class PullRequestController;
class GitService;
class GitTaskRunner;
class HistoryController;
class RemoteController;
class RepositoryController;
class SettingsService;
class StashController;
class SyncController;

class MainController : public QObject
{
    Q_OBJECT

public:
    // 创建主控制器，通过 setter 注入所有依赖
    explicit MainController(QObject* parent = nullptr);

    // -- 依赖注入 --
    void setRepositoryController(RepositoryController* controller);
    void setChangesController(ChangesController* controller);
    void setHistoryController(HistoryController* controller);
    void setBranchController(BranchController* controller);
    void setStashController(StashController* controller);
    void setRemoteController(RemoteController* controller);
    void setSyncController(SyncController* controller);
    void setConflictController(ConflictController* controller);
    void setPullRequestController(PullRequestController* controller);
    void setGitService(GitService* gitService);
    void setSettingsService(SettingsService* settingsService);
    void setGitTaskRunner(GitTaskRunner* taskRunner);

    // -- 操作 --
    // 切换仓库并刷新全部数据
    void switchToRepository(const QString& repoPath);

    // 委托给 RepositoryController 打开仓库
    void openRepository(const QString& path);

    // 关闭仓库并清空所有 Model
    void closeCurrentRepository();

    // 刷新当前仓库全部数据
    void refreshAll();

    // -- 查询 --
    QString currentRepositoryPath() const;
    RepositoryController* repositoryController() const;
    ChangesController* changesController() const;
    HistoryController* historyController() const;
    BranchController* branchController() const;
    StashController* stashController() const;
    RemoteController* remoteController() const;
    SyncController* syncController() const;
    ConflictController* conflictController() const;
    PullRequestController* pullRequestController() const;

signals:
    // 仓库切换完成
    void repositorySwitched(const QString& repoPath);

    // 仓库已关闭
    void repositoryClosed();

    // 全量刷新完成
    void refreshCompleted();

    // 全局错误
    void globalError(const QString& title, const QString& message);

private slots:
    // 转发子控制器的 errorOccurred 信号为全局错误
    void onSubControllerError(const QString& operation, const QString& errorMessage);

private:
    // 连接子控制器的错误信号到全局错误转发
    void connectErrorSignals();

    // 仓库管理控制器
    RepositoryController* m_repositoryController;

    // 文件变更控制器
    ChangesController* m_changesController;

    // 提交历史控制器
    HistoryController* m_historyController;

    // 分支管理控制器
    BranchController* m_branchController;

    // Stash 管理控制器
    StashController* m_stashController;

    // Remote 管理控制器
    RemoteController* m_remoteController;

    // Pull Request 控制器
    PullRequestController* m_pullRequestController;

    // 远程同步控制器
    SyncController* m_syncController;

    // 冲突管理控制器
    ConflictController* m_conflictController;

    // Git 核心服务
    GitService* m_gitService;

    // 用户配置服务
    SettingsService* m_settingsService;

    // 异步任务执行器
    GitTaskRunner* m_gitTaskRunner = nullptr;

    // 当前仓库路径
    QString m_currentRepoPath;
};

#endif // MAINCONTROLLER_H
