#ifndef GITSERVICE_H
#define GITSERVICE_H

#include "domain/branch.h"
#include "domain/commit.h"
#include "domain/ConflictFile.h"
#include "domain/diff.h"
#include "domain/filechange.h"
#include "domain/GitOperationResult.h"
#include "domain/repository.h"
#include "domain/Remote.h"
#include "domain/Stash.h"
#include "domain/Tag.h"
#include "infrastructure/gitcommandexecutor.h"
#include "infrastructure/result.h"

#include <QList>
#include <QString>
#include <QStringList>

class GitService
{
public:
    // 创建 Git 服务对象，并持有一个内部 GitCommandExecutor
    GitService();

    // 返回当前 git 可执行文件路径
    QString gitExecutablePath() const;

    // 设置 git 可执行文件路径
    void setGitExecutablePath(const QString& gitExecutablePath);

    // 返回 Git 命令超时时间，单位毫秒
    int timeoutMs() const;

    // 设置 Git 命令超时时间，单位毫秒
    void setTimeoutMs(int timeoutMs);

    // 检查当前环境是否可以执行 git 命令
    Result<void> checkGitAvailable() const;

    // 打开并读取一个本地 Git 仓库的基础信息
    Result<Repository> openRepository(const QString& repositoryPath) const;

    // 读取仓库当前工作区文件变更
    Result<QList<FileChange>> status(const QString& repositoryPath) const;

    // 读取指定文件或整个仓库的原始 diff 文本
    Result<QString> rawDiff(const QString& repositoryPath,
                            const QString& filePath = QString(),
                            bool staged = false) const;

    Result<QString> rawCommitDiff(const QString& repositoryPath,
                                  const QString& commitHash,
                                  const QString& filePath = QString()) const;

    // 读取指定文件或整个仓库的结构化 diff 对象
    Result<Diff> diff(const QString& repositoryPath,
                      const QString& filePath = QString(),
                      bool staged = false) const;

    // 读取提交历史列表
    Result<QList<Commit>> commitHistory(const QString& repositoryPath,
                                        int maxCount = 100) const;

    // 读取全部分支的拓扑顺序提交列表（用于提交树形图）
    Result<QList<Commit>> commitGraph(const QString& repositoryPath,
                                      int maxCount = 500) const;

    // 读取所有 tag 列表
    Result<QList<Tag>> tags(const QString& repositoryPath) const;

    // 读取当前 HEAD 指向的提交哈希
    Result<QString> currentHEAD(const QString& repositoryPath) const;

    Result<void> reset(const QString& repoPath, const QString& hash, const QString& mode = QStringLiteral("soft")) const;
    Result<void> cherryPick(const QString& repoPath, const QString& hash) const;
    Result<void> checkoutCommit(const QString& repoPath, const QString& hash) const;

    // 读取本地分支和远程分支列表
    Result<QList<Branch>> branches(const QString& repositoryPath) const;

    // 读取当前分支名称
    Result<QString> currentBranch(const QString& repositoryPath) const;

    // 暂存指定文件
    Result<void> stageFile(const QString& repositoryPath,
                           const QString& filePath) const;

    // 取消暂存指定文件
    Result<void> unstageFile(const QString& repositoryPath,
                             const QString& filePath) const;

    // 丢弃工作区文件的未暂存更改（git restore <file>）
    Result<void> restoreFile(const QString& repositoryPath,
                             const QString& filePath) const;

    // 重命名/移动已跟踪文件（git mv <oldPath> <newPath>）
    Result<void> renameFile(const QString& repositoryPath,
                            const QString& oldPath,
                            const QString& newPath) const;

    // 将文件模式追加到仓库根目录的 .gitignore 文件中
    Result<void> addToGitignore(const QString& repositoryPath,
                                const QString& pattern) const;

    // 创建一次提交
    Result<void> commit(const QString& repositoryPath,
                        const QString& message) const;

    // 从远程仓库获取最新信息
    Result<void> fetch(const QString& repositoryPath,
                       const QString& remoteName = QStringLiteral("origin")) const;

    // 拉取当前分支的远程更新
    // useRebase=true 时使用 git pull --rebase（推荐，避免多余的 merge commit）
    Result<void> pull(const QString& repositoryPath,
                     const QString& remoteName = QString(),
                     bool useRebase = true) const;

    // 推送当前分支或指定分支到远程仓库
    Result<void> push(const QString& repositoryPath,
                      const QString& remoteName = QString(),
                      const QString& branchName = QString()) const;

    // --- 仓库初始化与克隆 ---

    // 在指定路径初始化一个新的 Git 仓库
    Result<void> init(const QString& path) const;

    // 克隆远程仓库到本地目标路径
    Result<void> clone(const QString& url,
                       const QString& targetPath) const;

    // 判断指定路径是否为 Git 仓库
    Result<bool> isGitRepository(const QString& path) const;

    // --- 分支管理 ---

    // 在指定仓库中创建新分支
    Result<void> createBranch(const QString& repoPath,
                              const QString& branchName) const;

    // 在指定提交处创建分支
    Result<void> createBranchAt(const QString& repoPath,
                                const QString& branchName,
                                const QString& commitHash) const;

    // 切换到指定分支
    Result<void> checkoutBranch(const QString& repoPath,
                                const QString& branchName) const;

    // 删除指定分支
    Result<void> deleteBranch(const QString& repoPath,
                              const QString& branchName,
                              bool force = false) const;

    // 将指定分支合并到当前分支（基础版，不返回详细结果）
    Result<void> merge(const QString& repoPath,
                       const QString& branchName) const;

    // 将指定分支合并到当前分支（增强版，返回含冲突信息的操作结果）
    Result<GitOperationResult> mergeBranch(const QString& repoPath,
                                           const QString& branchName) const;

    // 将当前分支变基到目标分支
    Result<GitOperationResult> rebaseBranch(const QString& repoPath,
                                            const QString& branchName) const;

    // 中止正在进行的合并
    Result<void> abortMerge(const QString& repoPath) const;

    // 继续变基（冲突解决后）
    Result<void> continueRebase(const QString& repoPath) const;

    // 中止正在进行的变基
    Result<void> abortRebase(const QString& repoPath) const;

    // --- 仓库状态检测 ---

    // 检查是否处于合并状态（.git/MERGE_HEAD 存在）
    Result<bool> isMerging(const QString& repoPath) const;

    // 检查是否处于变基状态
    Result<bool> isRebasing(const QString& repoPath) const;

    // 检查工作区是否干净
    Result<bool> isWorkingTreeClean(const QString& repoPath) const;

    // --- 冲突检测（Phase 3） ---

    Result<bool> hasConflicts(const QString& repoPath) const;

    Result<std::vector<ConflictFile>> listConflictFiles(
        const QString& repoPath) const;

    Result<void> markResolved(const QString& repoPath,
                              const QString& filePath) const;

    Result<void> checkoutOurs(const QString& repoPath,
                              const QString& filePath) const;

    Result<void> checkoutTheirs(const QString& repoPath,
                                const QString& filePath) const;

    // --- 三方合并工具（Phase 4） ---

    Result<QString> readOursFile(const QString& repoPath,
                                  const QString& filePath) const;

    Result<QString> readTheirsFile(const QString& repoPath,
                                    const QString& filePath) const;

    Result<QString> readBaseFile(const QString& repoPath,
                                  const QString& filePath) const;

    Result<void> writeResolvedFile(const QString& repoPath,
                                    const QString& filePath,
                                    const QString& content) const;

    // 重命名分支
    Result<void> renameBranch(const QString& repoPath,
                              const QString& oldName,
                              const QString& newName) const;

    // --- Remote 管理 ---

    // 列出指定仓库的所有 remote 名称
    Result<QStringList> listRemotes(const QString& repoPath) const;

    // 添加一个 remote
    Result<void> addRemote(const QString& repoPath,
                           const QString& name,
                           const QString& url) const;

    // 移除一个 remote
    Result<void> removeRemote(const QString& repoPath,
                              const QString& name) const;

    // 获取 remote 详细信息列表（名称、fetch URL、push URL）
    Result<std::vector<Remote>> remoteDetails(const QString& repoPath) const;

    // 重命名 remote
    Result<void> renameRemote(const QString& repoPath,
                              const QString& oldName,
                              const QString& newName) const;

    // 修改 remote URL
    Result<void> setRemoteUrl(const QString& repoPath,
                              const QString& name,
                              const QString& url) const;

    // 读取 Git 配置值（默认读取全局配置）
    Result<QString> readConfig(const QString& key,
                               const QString& repoPath = QString(),
                               bool global = true) const;

    // 写入 Git 配置值（默认写入全局配置）
    Result<void> writeConfig(const QString& key,
                             const QString& value,
                             const QString& repoPath = QString(),
                             bool global = true) const;

    // --- Stash 管理 ---

    // 列出指定仓库的所有 stash
    Result<std::vector<Stash>> listStashes(const QString& repoPath) const;

    // 创建一个新的 stash
    Result<void> createStash(const QString& repoPath, const QString& message) const;

    // 应用指定索引的 stash
    Result<void> applyStash(const QString& repoPath, int index) const;

    // 删除指定索引的 stash
    Result<void> dropStash(const QString& repoPath, int index) const;

    // 显示指定索引的 stash diff
    Result<Diff> showStashDiff(const QString& repoPath, int index) const;

    // --- AI Commit Message（Phase 5） ---

    // 获取暂存区原始 diff（等价于 rawDiff(repoPath, QString(), true)）
    Result<QString> stagedDiffRaw(const QString& repoPath) const;

    // 返回底层 GitCommandExecutor 引用（供 TerminalService 执行用户命令）
    GitCommandExecutor& executor() { return m_executor; }
    const GitCommandExecutor& executor() const { return m_executor; }

    // 代理注册命令日志回调到 GitCommandExecutor
    void setCommandLogCallback(CommandLogCallback callback);

private:
    // 底层 Git 命令执行器
    GitCommandExecutor m_executor;
};

#endif // GITSERVICE_H
