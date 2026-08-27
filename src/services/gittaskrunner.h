#ifndef GITTASKRUNNER_H
#define GITTASKRUNNER_H

#include "services/gitservice.h"

#include <QList>
#include <QObject>
#include <QString>

class GitTaskRunner : public QObject
{
    Q_OBJECT

public:
    // 创建 Git 异步任务执行器
    explicit GitTaskRunner(QObject* parent = nullptr);

    // 返回当前 git 可执行文件路径
    QString gitExecutablePath() const;

    // 设置 git 可执行文件路径
    void setGitExecutablePath(const QString& gitExecutablePath);

    // 返回 Git 命令超时时间，单位毫秒
    int timeoutMs() const;

    // 设置 Git 命令超时时间，单位毫秒
    void setTimeoutMs(int timeoutMs);

    // 判断当前是否有后台任务正在运行
    bool isBusy() const;

    // 返回当前正在运行的后台任务数量
    int activeTaskCount() const;

public slots:
    // 异步检查 Git 是否可用
    void checkGitAvailable();

    // 异步打开仓库并读取基础信息
    void openRepository(const QString& repositoryPath);

    // 异步读取工作区文件变更
    void loadStatus(const QString& repositoryPath);

    // 异步读取原始 diff 文本
    void loadRawDiff(const QString& repositoryPath, const QString& filePath = QString());

    // 异步读取结构化 diff 对象
    void loadDiff(const QString& repositoryPath, const QString& filePath = QString(), bool staged = false);

    // 异步读取提交历史
    void loadCommitHistory(const QString& repositoryPath, int maxCount = 100);

    // 异步读取指定提交的 diff
    void loadCommitDiff(const QString& repositoryPath, const QString& commitHash, const QString& filePath = QString());

    // 异步读取分支列表
    void loadBranches(const QString& repositoryPath);

    // 异步读取当前分支名
    void loadCurrentBranch(const QString& repositoryPath);

    // 异步暂存指定文件
    void stageFile(const QString& repositoryPath, const QString& filePath);

    // 异步暂存全部未暂存文件
    void stageAllFiles(const QString& repositoryPath);

    // 异步取消暂存指定文件
    void unstageFile(const QString& repositoryPath, const QString& filePath);

    // 异步创建提交
    void commit(const QString& repositoryPath, const QString& message);

    // 异步执行 fetch
    void fetch(const QString& repositoryPath, const QString& remoteName = QStringLiteral("origin"));

    // 异步执行 pull（默认使用 --rebase，避免多余 merge commit）
    void pull(const QString& repositoryPath,
              const QString& remoteName = QString(),
              bool useRebase = true);

    // 异步执行 push
    void push(const QString& repositoryPath,
              const QString& remoteName = QString(),
              const QString& branchName = QString());

    // 异步初始化新仓库
    void initRepository(const QString& path);

    // 异步克隆远程仓库
    void cloneRepository(const QString& url, const QString& targetPath);

    // 异步创建分支
    void createBranch(const QString& repoPath, const QString& branchName);

    // 异步切换到指定分支
    void checkoutBranch(const QString& repoPath, const QString& branchName);

    // 异步删除指定分支
    void deleteBranch(const QString& repoPath, const QString& branchName, bool force = false);

    // 异步创建 stash
    void createStash(const QString& repoPath, const QString& message);

    // 异步应用 stash
    void applyStash(const QString& repoPath, int index);

    // 异步删除 stash
    void dropStash(const QString& repoPath, int index);

    // 异步显示 stash diff
    void showStashDiff(const QString& repoPath, int index);

signals:
    // 后台任务开始时发出
    void taskStarted(const QString& taskName);

    // 后台任务结束时发出
    void taskFinished(const QString& taskName);

    // 忙碌状态变化时发出
    void busyChanged(bool busy);

    // Git 可用性检查完成时发出
    void gitAvailableChecked(bool success, const QString& errorMessage);

    // 仓库打开完成时发出
    void repositoryOpened(bool success,
                          const QString& repositoryPath,
                          const Repository& repository,
                          const QString& errorMessage);

    // 工作区状态加载完成时发出
    void statusLoaded(bool success,
                      const QString& repositoryPath,
                      const QList<FileChange>& changes,
                      const QString& errorMessage);

    // 原始 diff 加载完成时发出
    void rawDiffLoaded(bool success,
                       const QString& repositoryPath,
                       const QString& filePath,
                       const QString& rawText,
                       const QString& errorMessage);

    // 结构化 diff 加载完成时发出
    void diffLoaded(bool success,
                    const QString& repositoryPath,
                    const QString& filePath,
                    const Diff& diff,
                    const QString& errorMessage);

    // 提交 diff 加载完成时发出
    void commitDiffLoaded(bool success,
                          const QString& repositoryPath,
                          const QString& commitHash,
                          const QString& filePath,
                          const QString& rawText,
                          const QString& errorMessage);

    // 提交历史加载完成时发出
    void commitHistoryLoaded(bool success,
                             const QString& repositoryPath,
                             const QList<Commit>& commits,
                             const QString& errorMessage);

    // 分支列表加载完成时发出
    void branchesLoaded(bool success,
                        const QString& repositoryPath,
                        const QList<Branch>& branches,
                        const QString& errorMessage);

    // 当前分支加载完成时发出
    void currentBranchLoaded(bool success,
                             const QString& repositoryPath,
                             const QString& branchName,
                             const QString& errorMessage);

    // 文件暂存完成时发出
    void fileStaged(bool success,
                    const QString& repositoryPath,
                    const QString& filePath,
                    const QString& errorMessage);

    // 全部文件暂存完成时发出
    void allFilesStaged(bool success,
                        const QString& repositoryPath,
                        const QString& errorMessage);

    // 文件取消暂存完成时发出
    void fileUnstaged(bool success,
                      const QString& repositoryPath,
                      const QString& filePath,
                      const QString& errorMessage);

    // 提交完成时发出
    void commitFinished(bool success,
                        const QString& repositoryPath,
                        const QString& errorMessage);

    // fetch 完成时发出
    void fetchFinished(bool success,
                       const QString& repositoryPath,
                       const QString& remoteName,
                       const QString& errorMessage);

    // pull 完成时发出
    void pullFinished(bool success,
                      const QString& repositoryPath,
                      const QString& errorMessage);

    // push 完成时发出
    void pushFinished(bool success,
                      const QString& repositoryPath,
                      const QString& remoteName,
                      const QString& branchName,
                      const QString& errorMessage);

    // 仓库初始化完成时发出
    void repositoryInitialized(bool success,
                               const QString& path,
                               const Repository& repository,
                               const QString& errorMessage);

    // 仓库克隆完成时发出
    void repositoryCloned(bool success,
                          const QString& url,
                          const Repository& repository,
                          const QString& errorMessage);

    // 分支创建完成时发出
    void branchCreated(bool success,
                       const QString& repoPath,
                       const QString& branchName,
                       const QString& errorMessage);

    // 分支切换完成时发出
    void branchCheckedOut(bool success,
                          const QString& repoPath,
                          const QString& branchName,
                          const QString& errorMessage);

    // 分支删除完成时发出
    void branchDeleted(bool success,
                       const QString& repoPath,
                       const QString& branchName,
                       const QString& errorMessage);

    // Stash 创建完成时发出
    void stashCreated(bool success,
                      const QString& repoPath,
                      const QString& message,
                      const QString& errorMessage);

    // Stash 应用完成时发出
    void stashApplied(bool success,
                      const QString& repoPath,
                      int index,
                      const QString& errorMessage);

    // Stash 删除完成时发出
    void stashDropped(bool success,
                      const QString& repoPath,
                      int index,
                      const QString& errorMessage);

    // Stash diff 加载完成时发出
    void stashDiffLoaded(bool success,
                         const QString& repoPath,
                         int index,
                         const Diff& diff,
                         const QString& errorMessage);

private:
    // 标记一个后台任务开始
    void startTask(const QString& taskName);

    // 标记一个后台任务结束
    void finishTask(const QString& taskName);

private:
    // 实际执行 Git 业务逻辑的服务对象
    GitService m_service;

    // 当前正在运行的任务数量
    int m_activeTaskCount;
};

#endif // GITTASKRUNNER_H
