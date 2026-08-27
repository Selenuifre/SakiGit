#ifndef SYNCCONTROLLER_H
#define SYNCCONTROLLER_H

#include "infrastructure/result.h"

#include "BaseController.h"
#include <QString>
#include <QStringList>

class GitTaskRunner;
class SyncService;

class SyncController : public BaseController
{
    Q_OBJECT

public:
    // 使用同步服务构造控制器
    explicit SyncController(SyncService* syncService, QObject* parent = nullptr);

    // 注入异步任务执行器
    void setGitTaskRunner(GitTaskRunner* taskRunner);

    // 从远程仓库获取最新信息
    void fetch(const QString& repoPath, const QString& remoteName = QStringLiteral("origin"));

    // 拉取当前分支的远程更新
    void pull(const QString& repoPath);

    // 推送当前分支到远程
    void push(const QString& repoPath);

    // 推送指定分支到指定远程
    void push(const QString& repoPath, const QString& remote, const QString& branch);

    // 列出所有 remote 名称
    Result<QStringList> listRemotes(const QString& repoPath) const;

    // 添加一个 remote
    Result<void> addRemote(const QString& repoPath, const QString& name, const QString& url);

    // 移除一个 remote
    Result<void> removeRemote(const QString& repoPath, const QString& name);

signals:
    // fetch 完成
    void fetchFinished(bool success, const QString& errorMessage);

    // pull 完成
    void pullFinished(bool success, const QString& errorMessage);

    // push 完成
    void pushFinished(bool success, const QString& errorMessage);

    // 远程操作完成
    void remoteOperationFinished(const QString& operation, bool success, const QString& errorMessage);

    // 操作发生错误

private slots:
    void onFetchFinished(bool success, const QString& repoPath, const QString& remoteName, const QString& errorMessage);
    void onPullFinished(bool success, const QString& repoPath, const QString& errorMessage);
    void onPushFinished(bool success, const QString& repoPath, const QString& remoteName, const QString& branchName, const QString& errorMessage);

private:
    // 同步业务服务（回退用）
    SyncService* m_syncService;

    // 异步任务执行器
    GitTaskRunner* m_taskRunner = nullptr;
};

#endif // SYNCCONTROLLER_H
