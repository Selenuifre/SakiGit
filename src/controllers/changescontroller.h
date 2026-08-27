#ifndef CHANGESCONTROLLER_H
#define CHANGESCONTROLLER_H

#include "infrastructure/result.h"

#include "BaseController.h"
#include <QString>

#include "domain/diff.h"

class DiffLineModel;
class FileChangeModel;
class GitService;
class GitTaskRunner;

class ChangesController : public BaseController
{
    Q_OBJECT

public:
    // 使用 Git 服务和文件变更/diff 模型构造控制器
    explicit ChangesController(GitService* gitService,
                               FileChangeModel* fileChangeModel,
                               DiffLineModel* diffLineModel,
                               QObject* parent = nullptr);

    // 注入异步任务执行器
    void setGitTaskRunner(GitTaskRunner* taskRunner);

    // 刷新工作区文件变更列表
    void refreshChanges(const QString& repoPath);

    // 加载指定文件的 diff；staged=true 时使用 git diff --cached
    void loadDiff(const QString& repoPath, const QString& filePath,
                  bool staged = false);

    // 暂存指定文件
    Result<void> stageFile(const QString& repoPath, const QString& filePath);

    // 暂存全部文件
    Result<void> stageAllFiles(const QString& repoPath);

    // 取消暂存指定文件
    Result<void> unstageFile(const QString& repoPath, const QString& filePath);

    // 取消暂存全部文件
    Result<void> unstageAllFiles(const QString& repoPath);

    // 提交暂存区
    Result<void> commit(const QString& repoPath, const QString& message);

    // 丢弃指定文件的变更
    Result<void> discardChanges(const QString& repoPath, const QString& filePath);

    // 将指定文件添加到 .gitignore
    Result<void> ignoreFile(const QString& repoPath, const QString& filePath);

    // 重命名指定的已跟踪文件
    Result<void> renameFile(const QString& repoPath, const QString& oldPath, const QString& newPath);

    // 返回文件变更模型
    FileChangeModel* fileChangeModel() const;

    // 返回 diff 行模型
    DiffLineModel* diffLineModel() const;

    // 清空所有模型数据
    void clear();

signals:
    // 文件变更刷新完成
    void changesRefreshed(bool success, const QString& errorMessage);

    // diff 加载完成
    void diffLoaded(bool success, const QString& filePath, const QString& errorMessage);

    // 文件暂存完成
    void fileStaged(bool success, const QString& filePath, const QString& errorMessage);

    // 文件取消暂存完成
    void fileUnstaged(bool success, const QString& filePath, const QString& errorMessage);

    // 提交完成
    void commitFinished(bool success, const QString& errorMessage);

    // 操作发生错误

private slots:
    void onFileStaged(bool success, const QString& repoPath, const QString& filePath, const QString& errorMessage);
    void onFileUnstaged(bool success, const QString& repoPath, const QString& filePath, const QString& errorMessage);
    void onCommitFinished(bool success, const QString& repoPath, const QString& errorMessage);
    void onDiffLoaded(bool success, const QString& repoPath, const QString& filePath, const Diff& diff, const QString& errorMessage);
    void onAllFilesStaged(bool success, const QString& repoPath, const QString& errorMessage);

private:
    // Git 核心服务（回退用）
    GitService* m_gitService;

    // 文件变更模型
    FileChangeModel* m_fileChangeModel;

    // diff 行显示模型
    DiffLineModel* m_diffLineModel;

    // 异步任务执行器
    GitTaskRunner* m_taskRunner = nullptr;

    // 当前操作的仓库路径（用于异步回调后刷新）
    QString m_pendingRepoPath;
};

#endif // CHANGESCONTROLLER_H
