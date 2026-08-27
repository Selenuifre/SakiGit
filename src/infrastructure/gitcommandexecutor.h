#ifndef GITCOMMANDEXECUTOR_H
#define GITCOMMANDEXECUTOR_H

#include "result.h"

#include <QString>
#include <QStringList>
#include <functional>

// 命令日志回调：在每次 git 命令执行后调用
// 参数：命令行文本, 工作目录, 退出码, 输出文本(stdout+stderr)
using CommandLogCallback = std::function<void(
    const QString& commandLine, const QString& workingDir,
    int exitCode, const QString& output)>;

class GitCommandExecutor
{
public:
    // 保存一次 Git 命令执行后的完整结果
    struct CommandResult {
        // Git 进程退出码，0 通常表示成功
        int exitCode = -1;

        // 标准输出内容，例如 git status 的正常输出
        QString standardOutput;

        // 标准错误内容，例如 Git 报错信息
        QString standardError;

        // 判断命令是否执行成功
        bool isSuccess() const
        {
            return exitCode == 0;
        }

        // 返回优先用于展示的消息
        QString message() const
        {
            if (!standardError.trimmed().isEmpty()) {
                return standardError.trimmed();
            }

            return standardOutput.trimmed();
        }
    };

public:
    // 创建 Git 命令执行器，默认使用系统 PATH 中的 git
    GitCommandExecutor();

    // 返回当前使用的 git 可执行文件路径
    QString gitExecutablePath() const;

    // 设置 git 可执行文件路径，例如 "git" 或 "C:/Program Files/Git/bin/git.exe"
    void setGitExecutablePath(const QString& gitExecutablePath);

    // 返回命令超时时间，单位毫秒
    int timeoutMs() const;

    // 设置命令超时时间，单位毫秒
    void setTimeoutMs(int timeoutMs);

    // 检查当前机器是否可以执行 git 命令；失败时 Result::error() 包含结构化错误信息
    Result<void> checkGitAvailable() const;

    // 在指定工作目录执行 git 命令；进程启动失败或超时时返回结构化错误
    Result<CommandResult> run(const QStringList& arguments,
                              const QString& workingDirectory = QString()) const;

    // 判断指定路径是否是 Git 仓库
    Result<bool> isRepository(const QString& repositoryPath) const;

    // 获取当前分支名；detached HEAD 时可能返回空字符串
    Result<QString> currentBranch(const QString& repositoryPath) const;

    // 获取仓库根目录路径
    Result<QString> repositoryRoot(const QString& repositoryPath) const;

    // 获取指定 remote 的 URL，默认 remote 是 origin
    Result<QString> remoteUrl(const QString& repositoryPath,
                              const QString& remoteName = QStringLiteral("origin")) const;

    // 获取工作区状态的 porcelain 输出
    Result<QString> statusPorcelain(const QString& repositoryPath) const;

    // 获取指定文件或整个仓库的 diff 文本
    // cached=true 时使用 --cached（比对暂存区与 HEAD，适用于 staged 文件）
    Result<QString> diff(const QString& repositoryPath,
                         const QString& filePath = QString(),
                         bool cached = false) const;

    Result<QString> commitDiff(const QString& repositoryPath,
                               const QString& commitHash,
                               const QString& filePath = QString()) const;

    // 获取提交历史文本输出
    Result<QString> log(const QString& repositoryPath,
                        int maxCount = 100) const;

    // 暂存一个文件
    Result<void> stageFile(const QString& repositoryPath,
                           const QString& filePath) const;

    // 取消暂存一个文件
    Result<void> unstageFile(const QString& repositoryPath,
                             const QString& filePath) const;

    // 丢弃工作区文件的未暂存更改（git restore <file>）
    Result<void> restoreFile(const QString& repositoryPath,
                             const QString& filePath) const;

    // 重命名/移动一个已跟踪文件（git mv <oldPath> <newPath>）
    Result<void> mvFile(const QString& repositoryPath,
                        const QString& oldPath,
                        const QString& newPath) const;

    // 创建一次提交
    Result<void> commit(const QString& repositoryPath,
                        const QString& message) const;

    // 获取 stash 列表原始文本输出
    Result<QString> stashList(const QString& repositoryPath) const;

    // 创建一个新的 stash
    Result<void> stashPush(const QString& repositoryPath,
                           const QString& message) const;

    // 应用指定索引的 stash
    Result<void> stashApply(const QString& repositoryPath, int index) const;

    // 删除指定索引的 stash
    Result<void> stashDrop(const QString& repositoryPath, int index) const;

    // 显示指定索引的 stash diff 原始文本
    Result<QString> stashDiff(const QString& repositoryPath, int index) const;

    // 获取 remote 详细列表原始文本（git remote -v）
    Result<QString> remoteVerbose(const QString& repositoryPath) const;

    // 重命名 remote（git remote rename <old> <new>）
    Result<void> remoteRename(const QString& repositoryPath,
                              const QString& oldName,
                              const QString& newName) const;

    // 修改 remote URL（git remote set-url <name> <url>）
    Result<void> remoteSetUrl(const QString& repositoryPath,
                              const QString& name,
                              const QString& url) const;

    // 读取 Git 配置值（可选 --global / --local，默认 --global）
    // workingDirectory 为空时使用全局配置，非空时读取当前仓库的本地配置
    Result<QString> readConfig(const QString& key,
                               const QString& workingDirectory = QString(),
                               bool global = true) const;

    // 写入 Git 配置值（可选 --global / --local，默认 --global）
    Result<void> writeConfig(const QString& key,
                             const QString& value,
                             const QString& workingDirectory = QString(),
                             bool global = true) const;

    // 注册命令日志回调（供终端窗口使用）
    void setCommandLogCallback(CommandLogCallback callback);

    // 静默模式：为 true 时跳过命令日志回调（用于系统自动刷新操作）
    void setSilentMode(bool silent);
    bool isSilentMode() const;

private:
    // 执行 git 命令并统一处理 run() → 检查失败 → 检查退出码 的样板流程。
    // 返回 Result<QString>，包含 stdout；失败时包含 gitCommandError。
    Result<QString> executeAndCheck(const QStringList& arguments,
                                     const QString& repositoryPath,
                                     const QString& operationName) const;

    // 同上，但返回 Result<void>（适用于不关心 stdout 的操作）。
    Result<void> executeAndCheckVoid(const QStringList& arguments,
                                      const QString& repositoryPath,
                                      const QString& operationName) const;

    // git 可执行文件路径
    QString m_gitExecutablePath;

    // 命令超时时间，单位毫秒
    int m_timeoutMs;

    // 命令日志回调
    CommandLogCallback m_commandLogCallback;

    // 静默模式：跳过终端日志
    bool m_silentMode = false;
};

#endif // GITCOMMANDEXECUTOR_H
