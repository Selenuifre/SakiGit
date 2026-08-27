#include "gitcommandexecutor.h"
#include "infrastructure/PathUtils.h"

#include <QDir>
#include <QProcess>
#include <QProcessEnvironment>

namespace {
QString commandLineText(const QStringList& arguments)
{
    return arguments.join(QStringLiteral(" "));
}

QStringList withStableGitEncoding(const QStringList& arguments)
{
    QStringList effectiveArguments;
    effectiveArguments << QStringLiteral("-c")
                       << QStringLiteral("core.quotepath=false")
                       << QStringLiteral("-c")
                       << QStringLiteral("i18n.logOutputEncoding=utf-8")
                       << QStringLiteral("-c")
                       << QStringLiteral("i18n.commitEncoding=utf-8");
    effectiveArguments << arguments;
    return effectiveArguments;
}

QProcessEnvironment utf8ProcessEnvironment()
{
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("LANG"), QStringLiteral("C.UTF-8"));
    environment.insert(QStringLiteral("LC_ALL"), QStringLiteral("C.UTF-8"));
    // 禁止 Git 通过终端交互式询问用户名/密码。
    // QProcess 没有分配终端，如果 Git 尝试交互式提示会导致进程挂起直到超时（默认 30s）。
    // 设置此变量后，Git 会立即失败并返回错误，而不是阻塞。
    environment.insert(QStringLiteral("GIT_TERMINAL_PROMPT"), QStringLiteral("0"));
    return environment;
}

Error processError(Error::Code code,
                   const QString& message,
                   const QString& detail,
                   const QString& executablePath,
                   const QStringList& arguments,
                   const QString& workingDirectory)
{
    return Error(code, message, detail)
        .withContextValue(QStringLiteral("executable"), executablePath)
        .withContextValue(QStringLiteral("arguments"), commandLineText(arguments))
        .withContextValue(QStringLiteral("workingDirectory"), cleanWorkingDirectory(workingDirectory));
}

Error gitCommandError(const GitCommandExecutor::CommandResult& result,
                      const QString& operation,
                      const QString& repositoryPath,
                      const QStringList& arguments)
{
    return Error::gitError(result.message())
        .withContextValue(QStringLiteral("operation"), operation)
        .withContextValue(QStringLiteral("repositoryPath"), cleanWorkingDirectory(repositoryPath))
        .withContextValue(QStringLiteral("arguments"), commandLineText(arguments))
        .withContextValue(QStringLiteral("exitCode"), QString::number(result.exitCode));
}
}

GitCommandExecutor::GitCommandExecutor()
    : m_gitExecutablePath(QStringLiteral("git")),
    m_timeoutMs(120000)
{
}

QString GitCommandExecutor::gitExecutablePath() const
{
    return m_gitExecutablePath;
}

void GitCommandExecutor::setGitExecutablePath(const QString& gitExecutablePath)
{
    const QString cleanPath = gitExecutablePath.trimmed();

    if (!cleanPath.isEmpty()) {
        m_gitExecutablePath = cleanPath;
    }
}

int GitCommandExecutor::timeoutMs() const
{
    return m_timeoutMs;
}

void GitCommandExecutor::setTimeoutMs(int timeoutMs)
{
    if (timeoutMs > 0) {
        m_timeoutMs = timeoutMs;
    }
}

Result<void> GitCommandExecutor::checkGitAvailable() const
{
    const Result<CommandResult> result = run(QStringList() << QStringLiteral("--version"));

    if (result.isFailure()) {
        return Result<void>::failure(result.error());
    }

    if (!result.value().isSuccess()) {
        return Result<void>::failure(
            gitCommandError(result.value(),
                            QStringLiteral("checkGitAvailable"),
                            QString(),
                            QStringList() << QStringLiteral("--version")));
    }

    return Result<void>::success();
}

Result<GitCommandExecutor::CommandResult> GitCommandExecutor::run(
    const QStringList& arguments,
    const QString& workingDirectory) const
{
    QProcess process;
    const QStringList effectiveArguments = withStableGitEncoding(arguments);
    process.setProcessEnvironment(utf8ProcessEnvironment());

    if (!workingDirectory.trimmed().isEmpty()) {
        process.setWorkingDirectory(cleanWorkingDirectory(workingDirectory));
    }

    process.start(m_gitExecutablePath, effectiveArguments);

    if (!process.waitForStarted(m_timeoutMs)) {
        return Result<CommandResult>::failure(
            processError(Error::Code::ProcessFailed,
                         QStringLiteral("Failed to start git process."),
                         process.errorString(),
                         m_gitExecutablePath,
                         effectiveArguments,
                         workingDirectory));
    }

    if (!process.waitForFinished(m_timeoutMs)) {
        process.kill();
        process.waitForFinished();

        return Result<CommandResult>::failure(
            processError(Error::Code::Timeout,
                         QStringLiteral("Git command timed out."),
                         QStringLiteral("The git process exceeded the configured timeout."),
                         m_gitExecutablePath,
                         effectiveArguments,
                         workingDirectory)
                .withContextValue(QStringLiteral("timeoutMs"), QString::number(m_timeoutMs)));
    }

    CommandResult commandResult;
    commandResult.exitCode = process.exitCode();
    commandResult.standardOutput = QString::fromUtf8(process.readAllStandardOutput());
    commandResult.standardError = QString::fromUtf8(process.readAllStandardError());

    return Result<CommandResult>::success(commandResult);
}

void GitCommandExecutor::setCommandLogCallback(CommandLogCallback callback)
{
    m_commandLogCallback = std::move(callback);
}

void GitCommandExecutor::setSilentMode(bool silent)
{
    m_silentMode = silent;
}

bool GitCommandExecutor::isSilentMode() const
{
    return m_silentMode;
}

namespace {
void invokeCommandLog(const CommandLogCallback& callback,
                      const QStringList& arguments,
                      const QString& repositoryPath,
                      const GitCommandExecutor::CommandResult& result,
                      bool silent)
{
    if (!callback || silent)
        return;
    const QString cmdLine = QStringLiteral("git ") + arguments.join(QLatin1Char(' '));
    const int exitCode = result.exitCode;
    const QString output = result.standardOutput.isEmpty()
        ? result.standardError
        : (result.standardError.isEmpty()
            ? result.standardOutput
            : result.standardOutput + QLatin1Char('\n') + result.standardError);
    callback(cmdLine, repositoryPath, exitCode, output);
}
} // anonymous namespace

// --- 私有辅助方法：消除每个方法中 run → check failure → check exit code 的样板 ---

Result<QString> GitCommandExecutor::executeAndCheck(
    const QStringList& arguments,
    const QString& repositoryPath,
    const QString& operationName) const
{
    const Result<CommandResult> result = run(arguments, repositoryPath);
    if (!result.isFailure())
        invokeCommandLog(m_commandLogCallback, arguments, repositoryPath, result.value(), m_silentMode);

    if (result.isFailure()) {
        return Result<QString>::failure(result.error());
    }

    if (!result.value().isSuccess()) {
        return Result<QString>::failure(
            gitCommandError(result.value(), operationName, repositoryPath, arguments));
    }

    return Result<QString>::success(result.value().standardOutput);
}

Result<void> GitCommandExecutor::executeAndCheckVoid(
    const QStringList& arguments,
    const QString& repositoryPath,
    const QString& operationName) const
{
    const Result<CommandResult> result = run(arguments, repositoryPath);
    if (!result.isFailure())
        invokeCommandLog(m_commandLogCallback, arguments, repositoryPath, result.value(), m_silentMode);

    if (result.isFailure()) {
        return Result<void>::failure(result.error());
    }

    if (!result.value().isSuccess()) {
        return Result<void>::failure(
            gitCommandError(result.value(), operationName, repositoryPath, arguments));
    }

    return Result<void>::success();
}

Result<bool> GitCommandExecutor::isRepository(const QString& repositoryPath) const
{
    const QStringList arguments =
        QStringList() << QStringLiteral("rev-parse") << QStringLiteral("--is-inside-work-tree");
    const Result<CommandResult> result = run(
        arguments,
        repositoryPath);

    if (result.isFailure()) {
        return Result<bool>::failure(result.error());
    }

    if (!result.value().isSuccess()) {
        return Result<bool>::success(false);
    }

    return Result<bool>::success(result.value().standardOutput.trimmed() == QStringLiteral("true"));
}

Result<QString> GitCommandExecutor::currentBranch(const QString& repositoryPath) const
{
    const QStringList arguments =
        QStringList() << QStringLiteral("branch") << QStringLiteral("--show-current");
    const Result<QString> result = executeAndCheck(
        arguments, repositoryPath, QStringLiteral("currentBranch"));
    if (result.isFailure()) return result;
    return Result<QString>::success(result.value().trimmed());
}

Result<QString> GitCommandExecutor::repositoryRoot(const QString& repositoryPath) const
{
    const QStringList arguments =
        QStringList() << QStringLiteral("rev-parse") << QStringLiteral("--show-toplevel");
    const Result<QString> result = executeAndCheck(
        arguments, repositoryPath, QStringLiteral("repositoryRoot"));
    if (result.isFailure()) return result;
    return Result<QString>::success(QDir::cleanPath(result.value().trimmed()));
}

Result<QString> GitCommandExecutor::remoteUrl(const QString& repositoryPath,
                                              const QString& remoteName) const
{
    const QStringList arguments =
        QStringList() << QStringLiteral("remote")
                      << QStringLiteral("get-url")
                      << remoteName;
    const Result<QString> result = executeAndCheck(
        arguments, repositoryPath, QStringLiteral("remoteUrl"));
    if (result.isFailure())
        return Result<QString>::failure(
            result.error().withContextValue(QStringLiteral("remoteName"), remoteName));
    return Result<QString>::success(result.value().trimmed());
}

Result<QString> GitCommandExecutor::statusPorcelain(const QString& repositoryPath) const
{
    const QStringList arguments =
        QStringList() << QStringLiteral("status") << QStringLiteral("--porcelain");
    return executeAndCheck(arguments, repositoryPath, QStringLiteral("statusPorcelain"));
}

Result<QString> GitCommandExecutor::diff(const QString& repositoryPath,
                                         const QString& filePath,
                                         bool cached) const
{
    QStringList arguments;
    arguments << QStringLiteral("diff");
    if (cached) {
        arguments << QStringLiteral("--cached");
    }
    if (!filePath.trimmed().isEmpty()) {
        arguments << QStringLiteral("--") << filePath;
    }

    const Result<QString> result = executeAndCheck(
        arguments, repositoryPath, QStringLiteral("diff"));
    if (result.isFailure())
        return Result<QString>::failure(
            result.error().withContextValue(QStringLiteral("filePath"), filePath));
    return result;
}

Result<QString> GitCommandExecutor::commitDiff(const QString& repositoryPath,
                                               const QString& commitHash,
                                               const QString& filePath) const
{
    const QString cleanHash = commitHash.trimmed();
    if (cleanHash.isEmpty()) {
        return Result<QString>::failure(
            Error::invalidArgument(QStringLiteral("Commit hash cannot be empty."))
                .withContextValue(QStringLiteral("repositoryPath"),
                                  cleanWorkingDirectory(repositoryPath)));
    }

    QStringList arguments = QStringList()
        << QStringLiteral("show") << QStringLiteral("--format=") << cleanHash;
    if (!filePath.trimmed().isEmpty())
        arguments << QStringLiteral("--") << filePath.trimmed();
    const Result<QString> result = executeAndCheck(
        arguments, repositoryPath, QStringLiteral("commitDiff"));
    if (result.isFailure())
        return Result<QString>::failure(
            result.error().withContextValue(QStringLiteral("commitHash"), cleanHash));
    return result;
}

Result<QString> GitCommandExecutor::log(const QString& repositoryPath, int maxCount) const
{
    if (maxCount <= 0) {
        maxCount = 100;
    }

    const QString prettyFormat = QStringLiteral(
        "--pretty=format:%H%x1f%P%x1f%an%x1f%ae%x1f%ad%x1f%s%x1e");

    const QStringList arguments =
        QStringList() << QStringLiteral("log")
                      << QStringLiteral("--date=iso-strict")
                      << prettyFormat
                      << QStringLiteral("--name-only")
                      << QStringLiteral("-n")
                      << QString::number(maxCount);
    const Result<QString> result = executeAndCheck(
        arguments, repositoryPath, QStringLiteral("log"));
    if (result.isFailure())
        return Result<QString>::failure(
            result.error().withContextValue(QStringLiteral("maxCount"), QString::number(maxCount)));
    return result;
}

Result<void> GitCommandExecutor::stageFile(const QString& repositoryPath,
                                           const QString& filePath) const
{
    const QStringList arguments =
        QStringList() << QStringLiteral("add") << QStringLiteral("--") << filePath;
    const Result<void> result = executeAndCheckVoid(
        arguments, repositoryPath, QStringLiteral("stageFile"));
    if (result.isFailure())
        return Result<void>::failure(
            result.error().withContextValue(QStringLiteral("filePath"), filePath));
    return Result<void>::success();
}

Result<void> GitCommandExecutor::unstageFile(const QString& repositoryPath,
                                             const QString& filePath) const
{
    const QStringList arguments =
        QStringList() << QStringLiteral("restore")
                      << QStringLiteral("--staged")
                      << QStringLiteral("--")
                      << filePath;
    const Result<void> result = executeAndCheckVoid(
        arguments, repositoryPath, QStringLiteral("unstageFile"));
    if (result.isFailure())
        return Result<void>::failure(
            result.error().withContextValue(QStringLiteral("filePath"), filePath));
    return Result<void>::success();
}

Result<void> GitCommandExecutor::restoreFile(const QString& repositoryPath,
                                            const QString& filePath) const
{
    const QStringList arguments =
        QStringList() << QStringLiteral("restore") << QStringLiteral("--") << filePath;
    const Result<void> result = executeAndCheckVoid(
        arguments, repositoryPath, QStringLiteral("restoreFile"));
    if (result.isFailure())
        return Result<void>::failure(
            result.error().withContextValue(QStringLiteral("filePath"), filePath));
    return Result<void>::success();
}

Result<void> GitCommandExecutor::mvFile(const QString& repositoryPath,
                                        const QString& oldPath,
                                        const QString& newPath) const
{
    const QString cleanOld = oldPath.trimmed();
    const QString cleanNew = newPath.trimmed();
    if (cleanOld.isEmpty() || cleanNew.isEmpty()) {
        return Result<void>::failure(
            Error::invalidArgument(QStringLiteral("Source and destination paths cannot be empty.")));
    }
    const QStringList arguments =
        QStringList() << QStringLiteral("mv") << cleanOld << cleanNew;
    const Result<void> result = executeAndCheckVoid(
        arguments, repositoryPath, QStringLiteral("mvFile"));
    if (result.isFailure())
        return Result<void>::failure(
            result.error().withContextValue(QStringLiteral("oldPath"), cleanOld)
                .withContextValue(QStringLiteral("newPath"), cleanNew));
    return Result<void>::success();
}

Result<void> GitCommandExecutor::commit(const QString& repositoryPath,
                                        const QString& message) const
{
    const QString cleanMessage = message.trimmed();

    if (cleanMessage.isEmpty()) {
        return Result<void>::failure(
            Error::invalidArgument(QStringLiteral("Commit message cannot be empty."))
                .withContextValue(QStringLiteral("repositoryPath"),
                                  cleanWorkingDirectory(repositoryPath)));
    }

    const QStringList arguments =
        QStringList() << QStringLiteral("commit")
                      << QStringLiteral("-m")
                      << cleanMessage;
    return executeAndCheckVoid(arguments, repositoryPath, QStringLiteral("commit"));
}

Result<QString> GitCommandExecutor::stashList(const QString& repositoryPath) const
{
    const QStringList arguments =
        QStringList() << QStringLiteral("stash") << QStringLiteral("list");
    return executeAndCheck(arguments, repositoryPath, QStringLiteral("stashList"));
}

Result<void> GitCommandExecutor::stashPush(const QString& repositoryPath,
                                           const QString& message) const
{
    const QString cleanMessage = message.trimmed();
    QStringList arguments;
    arguments << QStringLiteral("stash") << QStringLiteral("push");
    if (!cleanMessage.isEmpty()) {
        arguments << QStringLiteral("-m") << cleanMessage;
    }

    return executeAndCheckVoid(arguments, repositoryPath, QStringLiteral("stashPush"));
}

Result<void> GitCommandExecutor::stashApply(const QString& repositoryPath, int index) const
{
    const QString stashRef = QStringLiteral("stash@{%1}").arg(index);
    const QStringList arguments =
        QStringList() << QStringLiteral("stash") << QStringLiteral("apply") << stashRef;
    const Result<void> result = executeAndCheckVoid(
        arguments, repositoryPath, QStringLiteral("stashApply"));
    if (result.isFailure())
        return Result<void>::failure(
            result.error().withContextValue(QStringLiteral("stashIndex"), QString::number(index)));
    return Result<void>::success();
}

Result<void> GitCommandExecutor::stashDrop(const QString& repositoryPath, int index) const
{
    const QString stashRef = QStringLiteral("stash@{%1}").arg(index);
    const QStringList arguments =
        QStringList() << QStringLiteral("stash") << QStringLiteral("drop") << stashRef;
    const Result<void> result = executeAndCheckVoid(
        arguments, repositoryPath, QStringLiteral("stashDrop"));
    if (result.isFailure())
        return Result<void>::failure(
            result.error().withContextValue(QStringLiteral("stashIndex"), QString::number(index)));
    return Result<void>::success();
}

Result<QString> GitCommandExecutor::stashDiff(const QString& repositoryPath, int index) const
{
    const QString stashRef = QStringLiteral("stash@{%1}").arg(index);
    const QStringList arguments =
        QStringList() << QStringLiteral("stash") << QStringLiteral("show")
                      << QStringLiteral("-p") << stashRef;
    const Result<QString> result = executeAndCheck(
        arguments, repositoryPath, QStringLiteral("stashDiff"));
    if (result.isFailure())
        return Result<QString>::failure(
            result.error().withContextValue(QStringLiteral("stashIndex"), QString::number(index)));
    return result;
}

Result<QString> GitCommandExecutor::remoteVerbose(const QString& repositoryPath) const
{
    const QStringList arguments =
        QStringList() << QStringLiteral("remote") << QStringLiteral("-v");
    const Result<CommandResult> result = run(arguments, repositoryPath);

    if (result.isFailure()) {
        return Result<QString>::failure(result.error());
    }

    if (!result.value().isSuccess()) {
        return Result<QString>::failure(
            gitCommandError(result.value(),
                            QStringLiteral("remoteVerbose"),
                            repositoryPath,
                            arguments));
    }

    return Result<QString>::success(result.value().standardOutput);
}

Result<void> GitCommandExecutor::remoteRename(const QString& repositoryPath,
                                              const QString& oldName,
                                              const QString& newName) const
{
    const QStringList arguments =
        QStringList() << QStringLiteral("remote") << QStringLiteral("rename")
                      << oldName.trimmed() << newName.trimmed();
    const Result<CommandResult> result = run(arguments, repositoryPath);

    if (result.isFailure()) {
        return Result<void>::failure(result.error());
    }

    if (!result.value().isSuccess()) {
        return Result<void>::failure(
            gitCommandError(result.value(),
                            QStringLiteral("remoteRename"),
                            repositoryPath,
                            arguments)
                .withContextValue(QStringLiteral("oldName"), oldName.trimmed())
                .withContextValue(QStringLiteral("newName"), newName.trimmed()));
    }

    return Result<void>::success();
}

Result<void> GitCommandExecutor::remoteSetUrl(const QString& repositoryPath,
                                              const QString& name,
                                              const QString& url) const
{
    const QStringList arguments =
        QStringList() << QStringLiteral("remote") << QStringLiteral("set-url")
                      << name.trimmed() << url.trimmed();
    const Result<CommandResult> result = run(arguments, repositoryPath);

    if (result.isFailure()) {
        return Result<void>::failure(result.error());
    }

    if (!result.value().isSuccess()) {
        return Result<void>::failure(
            gitCommandError(result.value(),
                            QStringLiteral("remoteSetUrl"),
                            repositoryPath,
                            arguments)
                .withContextValue(QStringLiteral("remoteName"), name.trimmed()));
    }

    return Result<void>::success();
}

Result<QString> GitCommandExecutor::readConfig(const QString& key,
                                                const QString& workingDirectory,
                                                bool global) const
{
    const QString cleanKey = key.trimmed();
    if (cleanKey.isEmpty()) {
        return Result<QString>::failure(
            Error::invalidArgument(QStringLiteral("Config key cannot be empty.")));
    }

    QStringList arguments;
    arguments << QStringLiteral("config");
    if (global) {
        arguments << QStringLiteral("--global");
    }
    arguments << cleanKey;

    const Result<QString> result = executeAndCheck(
        arguments, workingDirectory, QStringLiteral("readConfig"));
    if (result.isFailure())
        return Result<QString>::failure(
            result.error().withContextValue(QStringLiteral("configKey"), cleanKey));
    return Result<QString>::success(result.value().trimmed());
}

Result<void> GitCommandExecutor::writeConfig(const QString& key,
                                              const QString& value,
                                              const QString& workingDirectory,
                                              bool global) const
{
    const QString cleanKey = key.trimmed();
    if (cleanKey.isEmpty()) {
        return Result<void>::failure(
            Error::invalidArgument(QStringLiteral("Config key cannot be empty.")));
    }

    QStringList arguments;
    arguments << QStringLiteral("config");
    if (global) {
        arguments << QStringLiteral("--global");
    }
    arguments << cleanKey << value;

    return executeAndCheckVoid(arguments, workingDirectory, QStringLiteral("writeConfig"));
}
