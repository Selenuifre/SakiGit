#include "gitservice.h"

#include "domain/Stash.h"
#include "infrastructure/ResultUtils.h"

#include <QDateTime>
#include <QDir>
#include <QRegularExpression>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QTextStream>
#include "domain/Tag.h"
#include <utility>
namespace {
const QChar FieldSeparator(0x1f);
const QChar RecordSeparator(0x1e);

// ---- 提交树形图辅助函数 ----

QDateTime parseDateTime(const QString& value)
{
    return QDateTime::fromString(value.trimmed(), Qt::ISODate);
}

struct ParsedRefNames
{
    QStringList branches;
    QStringList tags;
    bool isHEAD = false;
};

ParsedRefNames parseRefNames(const QString& refString)
{
    ParsedRefNames result;
    if (refString.trimmed().isEmpty()) return result;
    const QStringList parts = refString.split(QStringLiteral(", "), Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        QString p = part.trimmed();
        if (p.startsWith(QStringLiteral("HEAD -> "))) {
            result.isHEAD = true;
            p = p.mid(8);
        }
        if (p.startsWith(QStringLiteral("tag: "))) {
            result.tags.append(p.mid(5).trimmed());
        } else if (!p.isEmpty() && p != QStringLiteral("HEAD")) {
            result.branches.append(p);
        }
    }
    return result;
}

// 扩展版 log 解析——额外解析 %D ref names 字段（字段 10）
QList<Commit> parseGraphLogOutput(const QString& output,
                                  QHash<QString, ParsedRefNames>& outRefNames)
{
    QList<Commit> commits;
    const QStringList records = output.split(RecordSeparator, Qt::SkipEmptyParts);
    for (const QString& record : records) {
        QString cleanRecord = record.trimmed();
        if (cleanRecord.isEmpty()) continue;
        QStringList lines = cleanRecord.split(QLatin1Char('\n'));
        const QString header = lines.takeFirst().trimmed();
        const QStringList fields = header.split(FieldSeparator, Qt::KeepEmptyParts);
        if (fields.size() < 11) continue;
        Commit commit;
        commit.setHash(fields.at(0));
        commit.setParentHashes(fields.at(1).split(QLatin1Char(' '), Qt::SkipEmptyParts));
        commit.setAuthorName(fields.at(2));
        commit.setAuthorEmail(fields.at(3));
        commit.setAuthorDate(parseDateTime(fields.at(4)));
        commit.setSummary(fields.at(5));
        commit.setBody(fields.at(6));
        commit.setCommitterName(fields.at(7));
        commit.setCommitterEmail(fields.at(8));
        commit.setCommitterDate(parseDateTime(fields.at(9)));
        outRefNames.insert(commit.hash(), parseRefNames(fields.at(10)));
        for (const QString& filePath : std::as_const(lines)) {
            const QString cleanPath = filePath.trimmed();
            if (!cleanPath.isEmpty()) commit.addChangedFile(cleanPath);
        }
        if (commit.isValid()) commits.append(commit);
    }
    return commits;
}

// for-each-ref --format 不支持 %xNN hex 编码，用 tab 作为字段分隔符
// （refname、objectname、subject 等 git 输出字段不会包含 tab）
const QChar RefFieldSeparator = QLatin1Char('\t');

// 解析 merge/rebase 命令输出，构造 GitOperationResult
GitOperationResult parseMergeRebaseOutput(
    const GitCommandExecutor::CommandResult& cmdResult,
    const QString& operationType,
    const QString& targetBranch)
{
    GitOperationResult opResult;
    opResult.operationType = operationType;
    opResult.targetBranch = targetBranch;
    opResult.rawOutput = cmdResult.standardOutput;
    opResult.rawError = cmdResult.standardError;

    if (cmdResult.isSuccess()) {
        opResult.success = true;
        opResult.hasConflicts = false;

        // 提取摘要信息：取第一行非空输出作为 message
        const QStringList lines = cmdResult.standardOutput.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        if (!lines.isEmpty()) {
            opResult.message = lines.first().trimmed();
        }
    } else {
        // 检查是否因冲突而失败
        const QString combined = cmdResult.standardOutput + QLatin1Char('\n') + cmdResult.standardError;
        if (combined.contains(QStringLiteral("CONFLICT"), Qt::CaseInsensitive)
            || combined.contains(QStringLiteral("conflict"), Qt::CaseInsensitive)) {
            opResult.success = false;
            opResult.hasConflicts = true;
            opResult.message = QStringLiteral("CONFLICT: 合并冲突，需要手动解决");
        } else if (combined.contains(QStringLiteral("Automatic merge failed"), Qt::CaseInsensitive)) {
            opResult.success = false;
            opResult.hasConflicts = true;
            opResult.message = QStringLiteral("自动合并失败，请手动解决冲突");
        } else {
            opResult.success = false;
            opResult.hasConflicts = false;
            // 提取错误信息
            if (!cmdResult.standardError.trimmed().isEmpty()) {
                opResult.message = cmdResult.standardError.trimmed();
            } else {
                opResult.message = cmdResult.standardOutput.trimmed();
            }
        }
    }

    return opResult;
}

// 获取冲突文件列表（通过 git diff --name-only --diff-filter=U）
QStringList fetchConflictFiles(const GitCommandExecutor& executor,
                                const QString& repoPath)
{
    const Result<GitCommandExecutor::CommandResult> result = executor.run(
        QStringList() << QStringLiteral("diff")
                      << QStringLiteral("--name-only")
                      << QStringLiteral("--diff-filter=U"),
        repoPath);

    if (result.isFailure() || !result.value().isSuccess()) {
        return {};
    }

    return result.value().standardOutput
        .split(QLatin1Char('\n'), Qt::SkipEmptyParts);
}

QList<FileChange> parseStatusOutput(const QString& output)
{
    QList<FileChange> changes;
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        if (line.length() < 3) {
            continue;
        }

        const QString code = line.left(2);
        QString pathText = line.mid(3).trimmed();

        FileChange change;
        change.setIndexStatus(code.left(1));
        change.setWorktreeStatus(code.mid(1, 1));
        change.setStatus(GitTypes::fileStatusFromPorcelain(code));
        change.setStageState(GitTypes::stageStateFromPorcelain(code));

        const int renameIndex = pathText.indexOf(QStringLiteral(" -> "));
        if (renameIndex >= 0) {
            change.setOldPath(pathText.left(renameIndex));
            change.setPath(pathText.mid(renameIndex + 4));
        } else {
            change.setPath(pathText);
        }

        if (change.isValid()) {
            changes.append(change);
        }
    }

    return changes;
}

QList<Commit> parseLogOutput(const QString& output)
{
    QList<Commit> commits;
    const QStringList records = output.split(RecordSeparator, Qt::SkipEmptyParts);

    for (const QString& record : records) {
        QString cleanRecord = record.trimmed();

        if (cleanRecord.isEmpty()) {
            continue;
        }

        QStringList lines = cleanRecord.split(QLatin1Char('\n'));
        const QString header = lines.takeFirst().trimmed();
        const QStringList fields = header.split(FieldSeparator, Qt::KeepEmptyParts);

        if (fields.size() < 10) {
            continue;
        }

        Commit commit;
        commit.setHash(fields.at(0));
        commit.setParentHashes(fields.at(1).split(QLatin1Char(' '), Qt::SkipEmptyParts));
        commit.setAuthorName(fields.at(2));
        commit.setAuthorEmail(fields.at(3));
        commit.setAuthorDate(parseDateTime(fields.at(4)));
        commit.setCommitterName(fields.at(5));
        commit.setCommitterEmail(fields.at(6));
        commit.setCommitterDate(parseDateTime(fields.at(7)));
        commit.setSummary(fields.at(8));
        commit.setBody(fields.at(9));

        for (const QString& filePath : std::as_const(lines)) {
            const QString cleanPath = filePath.trimmed();

            if (!cleanPath.isEmpty()) {
                commit.addChangedFile(cleanPath);
            }
        }

        if (commit.isValid()) {
            commits.append(commit);
        }
    }

    return commits;
}

QString readDefaultBranch(const GitCommandExecutor& executor, const QString& repositoryPath)
{
    const Result<GitCommandExecutor::CommandResult> result = executor.run(
        QStringList() << QStringLiteral("symbolic-ref")
                      << QStringLiteral("refs/remotes/origin/HEAD")
                      << QStringLiteral("--short"),
        repositoryPath);

    if (result.isFailure() || !result.value().isSuccess()) {
        return QString();
    }

    return GitTypes::localNameFromRemoteBranchName(result.value().standardOutput.trimmed());
}

void fillAheadBehind(const GitCommandExecutor& executor, const QString& repositoryPath, Branch& branch)
{
    if (!branch.hasUpstream()) {
        return;
    }

    const QString range = branch.name() + QStringLiteral("...") + branch.upstreamName();

    const Result<GitCommandExecutor::CommandResult> result = executor.run(
        QStringList() << QStringLiteral("rev-list")
                      << QStringLiteral("--left-right")
                      << QStringLiteral("--count")
                      << range,
        repositoryPath);

    if (result.isFailure() || !result.value().isSuccess()) {
        return;
    }

    const QStringList parts = result.value().standardOutput
                                  .simplified()
                                  .split(QLatin1Char(' '), Qt::SkipEmptyParts);

    if (parts.size() >= 2) {
        branch.setAheadCount(parts.at(0).toInt());
        branch.setBehindCount(parts.at(1).toInt());
    }
}

QList<Branch> parseBranchOutput(const QString& output,
                                const QString& currentBranchName,
                                const GitCommandExecutor& executor,
                                const QString& repositoryPath)
{
    QList<Branch> branches;
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        const QStringList fields = line.split(RefFieldSeparator, Qt::KeepEmptyParts);

        if (fields.size() < 6) {
            continue;
        }

        const QString fullName = fields.at(0).trimmed();
        const QString shortName = fields.at(1).trimmed();

        if (shortName.endsWith(QStringLiteral("/HEAD"))) {
            continue;
        }

        const GitTypes::BranchType type = fullName.startsWith(QStringLiteral("refs/remotes/"))
                                      ? GitTypes::BranchType::Remote
                                      : GitTypes::BranchType::Local;

        Branch branch(shortName, type);
        branch.setFullName(fullName);
        branch.setHeadCommitHash(fields.at(2));
        branch.setLastCommitSummary(fields.at(3));
        branch.setLastCommitDate(parseDateTime(fields.at(4)));
        branch.setUpstreamName(fields.at(5));

        if (branch.isRemote()) {
            branch.setRemoteName(GitTypes::remoteNameFromBranchName(branch.name()));
        } else if (branch.hasUpstream()) {
            branch.setRemoteName(GitTypes::remoteNameFromBranchName(branch.upstreamName()));
            fillAheadBehind(executor, repositoryPath, branch);
        }

        branch.setCurrent(branch.isLocal() && branch.name() == currentBranchName);

        if (branch.isValid()) {
            branches.append(branch);
        }
    }

    return branches;
}

std::vector<Stash> parseStashListOutput(const QString& output)
{
    std::vector<Stash> stashes;
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        const QString trimmed = line.trimmed();

        if (trimmed.isEmpty()) {
            continue;
        }

        // 格式: "stash@{N}: <message>"
        const QRegularExpression indexRegex(QStringLiteral("^stash@\\{(\\d+)\\}:\\s*(.*)$"));
        const QRegularExpressionMatch match = indexRegex.match(trimmed);

        if (!match.hasMatch()) {
            continue;
        }

        Stash stash;
        stash.setIndex(match.captured(1).toInt());

        const QString fullMessage = match.captured(2).trimmed();
        stash.setMessage(fullMessage);

        // 尝试从消息中提取分支名和提交哈希
        // 格式: "WIP on <branch>: <hash> <summary>" 或 "On <branch>: <hash> <summary>"
        const QRegularExpression branchRegex(
            QStringLiteral("^(?:WIP\\s+)?[Oo]n\\s+([^:]+):\\s*([0-9a-fA-F]+)(?:\\s+(.*))?$"));
        const QRegularExpressionMatch branchMatch = branchRegex.match(fullMessage);

        if (branchMatch.hasMatch()) {
            stash.setBranchName(branchMatch.captured(1).trimmed());
            stash.setCommitHash(branchMatch.captured(2));
        }

        stashes.push_back(stash);
    }

    return stashes;
}

}

GitService::GitService()
{
}

QString GitService::gitExecutablePath() const
{
    return m_executor.gitExecutablePath();
}

void GitService::setGitExecutablePath(const QString& gitExecutablePath)
{
    m_executor.setGitExecutablePath(gitExecutablePath);
}

int GitService::timeoutMs() const
{
    return m_executor.timeoutMs();
}

void GitService::setTimeoutMs(int timeoutMs)
{
    m_executor.setTimeoutMs(timeoutMs);
}

Result<void> GitService::checkGitAvailable() const
{
    return m_executor.checkGitAvailable();
}

Result<Repository> GitService::openRepository(const QString& repositoryPath) const
{
    const QString cleanPath = Repository::resolveAbsolutePath(repositoryPath);

    if (cleanPath.isEmpty()) {
        return Result<Repository>::failure(QStringLiteral("Repository path cannot be empty."));
    }

    // 优化: 用单次 rev-parse --show-toplevel 同时完成"是否为仓库"和"获取根路径"两个检查
    const Result<QString> rootResult = m_executor.repositoryRoot(cleanPath);

    if (rootResult.isFailure()) {
        return Result<Repository>::failure(rootResult.errorMessage());
    }

    Repository repository(rootResult.value());
    repository.setState(GitTypes::RepositoryState::Ready);
    repository.setLastOpenedAt(QDateTime::currentDateTime());

    // 仅获取关键路径信息（当前分支名）
    const Result<QString> branchResult = m_executor.currentBranch(rootResult.value());
    if (branchResult.isSuccess()) {
        repository.setCurrentBranch(branchResult.value());
    }

    // 非关键信息设置为异步获取的占位值，避免阻塞
    // defaultBranch 和 remoteUrl 延迟到后续异步填充
    repository.setDefaultBranch(branchResult.isSuccess()
        ? branchResult.value() : QString());

    return Result<Repository>::success(repository);
}

Result<QList<FileChange>> GitService::status(const QString& repositoryPath) const
{
    const Result<QString> result = m_executor.statusPorcelain(repositoryPath);

    if (result.isFailure()) {
        return Result<QList<FileChange>>::failure(result.errorMessage());
    }

    return Result<QList<FileChange>>::success(parseStatusOutput(result.value()));
}

Result<QString> GitService::rawDiff(const QString& repositoryPath, const QString& filePath, bool staged) const
{
    return m_executor.diff(repositoryPath, filePath, staged);
}

Result<QString> GitService::rawCommitDiff(const QString& repositoryPath, const QString& commitHash, const QString& filePath) const
{
    return m_executor.commitDiff(repositoryPath, commitHash, filePath);
}

Result<Diff> GitService::diff(const QString& repositoryPath, const QString& filePath, bool staged) const
{
    const Result<QString> result = rawDiff(repositoryPath, filePath, staged);

    if (result.isFailure()) {
        return Result<Diff>::failure(result.errorMessage());
    }

    return Result<Diff>::success(Diff::fromUnifiedDiff(result.value()));
}

Result<QList<Commit>> GitService::commitHistory(const QString& repositoryPath, int maxCount) const
{
    if (maxCount <= 0) {
        maxCount = 100;
    }

    const QString prettyFormat = QStringLiteral(
        "--pretty=format:%x1e%H%x1f%P%x1f%an%x1f%ae%x1f%ad%x1f%cn%x1f%ce%x1f%cd%x1f%s%x1f%b");

    const Result<GitCommandExecutor::CommandResult> result = m_executor.run(
        QStringList() << QStringLiteral("log")
                      << QStringLiteral("--date=iso-strict")
                      << prettyFormat
                      << QStringLiteral("--name-only")
                      << QStringLiteral("-n")
                      << QString::number(maxCount),
        repositoryPath);

    if (result.isFailure()) {
        return Result<QList<Commit>>::failure(result.errorMessage());
    }

    if (!result.value().isSuccess()) {
        return Result<QList<Commit>>::failure(result.value().message());
    }

    return Result<QList<Commit>>::success(parseLogOutput(result.value().standardOutput));
}

Result<QList<Commit>> GitService::commitGraph(const QString& repositoryPath,
                                               int maxCount) const
{
    if (maxCount <= 0) maxCount = 500;

    // --all --topo-order 获取全部分支的拓扑顺序提交
    // %D 输出 ref names（如 "HEAD -> main, origin/main, tag: v1.0"）
    const QString prettyFormat = QStringLiteral(
        "--pretty=format:%x1e%H%x1f%P%x1f%an%x1f%ae%x1f%ad%x1f%s%x1f%b%x1f%cn%x1f%ce%x1f%cd%x1f%D");

    const Result<GitCommandExecutor::CommandResult> result = m_executor.run(
        QStringList() << QStringLiteral("log")
                      << QStringLiteral("--all")
                      << QStringLiteral("--topo-order")
                      << QStringLiteral("--date=iso-strict")
                      << prettyFormat
                      << QStringLiteral("-n")
                      << QString::number(maxCount),
        repositoryPath);

    if (result.isFailure())
        return Result<QList<Commit>>::failure(result.errorMessage());
    if (!result.value().isSuccess())
        return Result<QList<Commit>>::failure(result.value().message());

    QHash<QString, ParsedRefNames> refNames;
    return Result<QList<Commit>>::success(
        parseGraphLogOutput(result.value().standardOutput, refNames));
}

Result<QList<Tag>> GitService::tags(const QString& repositoryPath) const
{
    // git for-each-ref refs/tags：格式与 branches() 一致，用 tab 分隔
    // 字段：%(refname:short)\t%(objectname)\t%(*objectname)
    // %(*objectname) 用于 annotated tag 的解引用
    const QString tagFormat = QStringLiteral(
        "--format=%(refname:short)\t%(*objectname)");
    const Result<GitCommandExecutor::CommandResult> result = m_executor.run(
        QStringList() << QStringLiteral("for-each-ref")
                      << QStringLiteral("refs/tags")
                      << tagFormat,
        repositoryPath);

    if (result.isFailure())
        return Result<QList<Tag>>::failure(result.errorMessage());
    if (!result.value().isSuccess())
        return Result<QList<Tag>>::failure(result.value().message());

    QList<Tag> tags;
    const QStringList lines = result.value().standardOutput
        .split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        const QStringList parts = line.split(RefFieldSeparator, Qt::KeepEmptyParts);
        Tag t;
        t.setName(parts.value(0).trimmed());
        // %(*objectname) 为空时回退到 %(objectname)
        QString hash = parts.value(1).trimmed();
        t.setTargetHash(hash);
        if (t.isValid()) tags.append(t);
    }
    return Result<QList<Tag>>::success(tags);
}

Result<QString> GitService::currentHEAD(const QString& repositoryPath) const
{
    const Result<GitCommandExecutor::CommandResult> result = m_executor.run(
        QStringList() << QStringLiteral("rev-parse") << QStringLiteral("HEAD"),
        repositoryPath);

    if (result.isFailure())
        return Result<QString>::failure(result.errorMessage());
    if (!result.value().isSuccess())
        return Result<QString>::failure(result.value().message());

    return Result<QString>::success(result.value().standardOutput.trimmed());
}

Result<void> GitService::reset(const QString& repoPath, const QString& hash, const QString& mode) const
{
    return toVoidResult(m_executor.run(
        QStringList() << QStringLiteral("reset") << (QStringLiteral("--") + mode.trimmed()) << hash, repoPath));
}

Result<void> GitService::cherryPick(const QString& repoPath, const QString& hash) const
{
    return toVoidResult(m_executor.run(
        QStringList() << QStringLiteral("cherry-pick") << hash, repoPath));
}

Result<void> GitService::checkoutCommit(const QString& repoPath, const QString& hash) const
{
    return toVoidResult(m_executor.run(
        QStringList() << QStringLiteral("checkout") << hash, repoPath));
}

Result<QList<Branch>> GitService::branches(const QString& repositoryPath) const
{
    QString currentName;

    const Result<QString> currentResult = m_executor.currentBranch(repositoryPath);
    if (currentResult.isSuccess()) {
        currentName = currentResult.value();
    }

    // for-each-ref --format 不支持 %xNN，使用字面量 tab（\t）作为字段分隔符
    const QString format = QStringLiteral(
        "--format=%(refname)\t%(refname:short)\t%(objectname)\t%(subject)\t%(committerdate:iso-strict)\t%(upstream:short)");

    const Result<GitCommandExecutor::CommandResult> result = m_executor.run(
        QStringList() << QStringLiteral("for-each-ref")
                      << QStringLiteral("--sort=-committerdate")
                      << format
                      << QStringLiteral("refs/heads")
                      << QStringLiteral("refs/remotes"),
        repositoryPath);

    if (result.isFailure()) {
        return Result<QList<Branch>>::failure(result.errorMessage());
    }

    if (!result.value().isSuccess()) {
        return Result<QList<Branch>>::failure(result.value().message());
    }

    return Result<QList<Branch>>::success(
        parseBranchOutput(result.value().standardOutput, currentName, m_executor, repositoryPath));
}

Result<QString> GitService::currentBranch(const QString& repositoryPath) const
{
    return m_executor.currentBranch(repositoryPath);
}

Result<void> GitService::stageFile(const QString& repositoryPath, const QString& filePath) const
{
    return m_executor.stageFile(repositoryPath, filePath);
}

Result<void> GitService::unstageFile(const QString& repositoryPath, const QString& filePath) const
{
    return m_executor.unstageFile(repositoryPath, filePath);
}

Result<void> GitService::restoreFile(const QString& repositoryPath, const QString& filePath) const
{
    return m_executor.restoreFile(repositoryPath, filePath);
}

Result<void> GitService::renameFile(const QString& repositoryPath,
                                    const QString& oldPath,
                                    const QString& newPath) const
{
    return m_executor.mvFile(repositoryPath, oldPath, newPath);
}

Result<void> GitService::addToGitignore(const QString& repositoryPath,
                                        const QString& pattern) const
{
    const QString gitignorePath = repositoryPath + QStringLiteral("/.gitignore");
    QFile file(gitignorePath);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return Result<void>::failure(
            QStringLiteral("Cannot open .gitignore for writing: %1").arg(gitignorePath));
    }
    QTextStream stream(&file);
    stream << pattern << QStringLiteral("\n");
    file.close();
    return Result<void>::success();
}

Result<void> GitService::commit(const QString& repositoryPath, const QString& message) const
{
    return m_executor.commit(repositoryPath, message);
}

Result<void> GitService::fetch(const QString& repositoryPath, const QString& remoteName) const
{
    QStringList arguments;
    arguments << QStringLiteral("fetch");

    if (!remoteName.trimmed().isEmpty()) {
        arguments << remoteName.trimmed();
    }

    return toVoidResult(m_executor.run(arguments, repositoryPath));
}

Result<void> GitService::pull(const QString& repositoryPath,
                             const QString& remoteName,
                             bool useRebase) const
{
    QStringList arguments;
    arguments << QStringLiteral("pull");
    if (useRebase) {
        arguments << QStringLiteral("--rebase");
    }
    if (!remoteName.trimmed().isEmpty()) {
        arguments << remoteName.trimmed();
    }
    return toVoidResult(m_executor.run(arguments, repositoryPath));
}

Result<void> GitService::push(const QString& repositoryPath,
                              const QString& remoteName,
                              const QString& branchName) const
{
    QStringList arguments;
    arguments << QStringLiteral("push");

    const QString cleanRemote = remoteName.trimmed();
    const QString cleanBranch = branchName.trimmed();

    if (!cleanRemote.isEmpty() && !cleanBranch.isEmpty()) {
        // 指定了 remote 和 branch → 设置上游并推送
        arguments << QStringLiteral("--set-upstream")
                  << cleanRemote << cleanBranch;
    } else if (!cleanRemote.isEmpty()) {
        // 仅指定 remote → git push <remote>
        arguments << cleanRemote;
    }
    // 都未指定 → git push（使用已配置的上游，由 git 默认行为处理）

    return toVoidResult(m_executor.run(arguments, repositoryPath));
}

// ============================================================================
// 仓库初始化与克隆
// ============================================================================

Result<void> GitService::init(const QString& path) const
{
    const QString cleanPath = Repository::resolveAbsolutePath(path);

    if (cleanPath.isEmpty()) {
        return Result<void>::failure(QStringLiteral("Repository path cannot be empty."));
    }

    // git init <path> 会创建目录（如果不存在）并在其中初始化仓库
    return toVoidResult(m_executor.run(
        QStringList() << QStringLiteral("init") << cleanPath));
}

Result<void> GitService::clone(const QString& url, const QString& targetPath) const
{
    const QString cleanUrl = url.trimmed();

    if (cleanUrl.isEmpty()) {
        return Result<void>::failure(QStringLiteral("Clone URL cannot be empty."));
    }

    const QString cleanTarget = targetPath.trimmed();

    if (cleanTarget.isEmpty()) {
        return Result<void>::failure(QStringLiteral("Target path cannot be empty."));
    }

    return toVoidResult(m_executor.run(
        QStringList() << QStringLiteral("clone") << cleanUrl << cleanTarget));
}

Result<bool> GitService::isGitRepository(const QString& path) const
{
    return m_executor.isRepository(path);
}

// ============================================================================
// 分支管理
// ============================================================================

Result<void> GitService::createBranch(const QString& repoPath,
                                      const QString& branchName) const
{
    const QString cleanName = branchName.trimmed();

    if (cleanName.isEmpty()) {
        return Result<void>::failure(QStringLiteral("Branch name cannot be empty."));
    }

    return toVoidResult(m_executor.run(
        QStringList() << QStringLiteral("branch") << cleanName, repoPath));
}

Result<void> GitService::createBranchAt(const QString& repoPath,
                                        const QString& branchName,
                                        const QString& commitHash) const
{
    const QString cleanName = branchName.trimmed();
    if (cleanName.isEmpty())
        return Result<void>::failure(QStringLiteral("Branch name cannot be empty."));
    return toVoidResult(m_executor.run(
        QStringList() << QStringLiteral("branch") << cleanName << commitHash, repoPath));
}

Result<void> GitService::checkoutBranch(const QString& repoPath,
                                        const QString& branchName) const
{
    const QString cleanName = branchName.trimmed();

    if (cleanName.isEmpty()) {
        return Result<void>::failure(QStringLiteral("Branch name cannot be empty."));
    }

    return toVoidResult(m_executor.run(
        QStringList() << QStringLiteral("checkout") << cleanName, repoPath));
}

Result<void> GitService::deleteBranch(const QString& repoPath,
                                      const QString& branchName,
                                      bool force) const
{
    const QString cleanName = branchName.trimmed();

    if (cleanName.isEmpty()) {
        return Result<void>::failure(QStringLiteral("Branch name cannot be empty."));
    }

    QStringList arguments;
    arguments << QStringLiteral("branch");

    if (force) {
        arguments << QStringLiteral("-D");
    } else {
        arguments << QStringLiteral("-d");
    }

    arguments << cleanName;

    return toVoidResult(m_executor.run(arguments, repoPath));
}

Result<void> GitService::merge(const QString& repoPath,
                               const QString& branchName) const
{
    const QString cleanName = branchName.trimmed();

    if (cleanName.isEmpty()) {
        return Result<void>::failure(QStringLiteral("Branch name cannot be empty."));
    }

    return toVoidResult(m_executor.run(
        QStringList() << QStringLiteral("merge") << cleanName, repoPath));
}

Result<void> GitService::renameBranch(const QString& repoPath,
                                      const QString& oldName,
                                      const QString& newName) const
{
    const QString cleanOldName = oldName.trimmed();
    const QString cleanNewName = newName.trimmed();

    if (cleanOldName.isEmpty()) {
        return Result<void>::failure(QStringLiteral("Old branch name cannot be empty."));
    }

    if (cleanNewName.isEmpty()) {
        return Result<void>::failure(QStringLiteral("New branch name cannot be empty."));
    }

    return toVoidResult(m_executor.run(
        QStringList() << QStringLiteral("branch")
                      << QStringLiteral("-m")
                      << cleanOldName
                      << cleanNewName,
        repoPath));
}

// ============================================================================
// Merge / Rebase 增强接口
// ============================================================================

Result<GitOperationResult> GitService::mergeBranch(const QString& repoPath,
                                                    const QString& branchName) const
{
    const QString cleanName = branchName.trimmed();

    if (cleanName.isEmpty()) {
        return Result<GitOperationResult>::failure(
            QStringLiteral("Branch name cannot be empty."));
    }

    const Result<GitCommandExecutor::CommandResult> result = m_executor.run(
        QStringList() << QStringLiteral("merge") << cleanName, repoPath);

    if (result.isFailure()) {
        return Result<GitOperationResult>::failure(result.errorMessage());
    }

    GitOperationResult opResult = parseMergeRebaseOutput(
        result.value(), QStringLiteral("merge"), cleanName);

    // 如果发生冲突，获取冲突文件列表
    if (opResult.hasConflicts) {
        opResult.conflictFiles = fetchConflictFiles(m_executor, repoPath);
    }

    return Result<GitOperationResult>::success(opResult);
}

Result<GitOperationResult> GitService::rebaseBranch(const QString& repoPath,
                                                     const QString& branchName) const
{
    const QString cleanName = branchName.trimmed();

    if (cleanName.isEmpty()) {
        return Result<GitOperationResult>::failure(
            QStringLiteral("Branch name cannot be empty."));
    }

    const Result<GitCommandExecutor::CommandResult> result = m_executor.run(
        QStringList() << QStringLiteral("rebase") << cleanName, repoPath);

    if (result.isFailure()) {
        return Result<GitOperationResult>::failure(result.errorMessage());
    }

    GitOperationResult opResult = parseMergeRebaseOutput(
        result.value(), QStringLiteral("rebase"), cleanName);

    // 如果发生冲突，获取冲突文件列表
    if (opResult.hasConflicts) {
        opResult.conflictFiles = fetchConflictFiles(m_executor, repoPath);
    }

    return Result<GitOperationResult>::success(opResult);
}

Result<void> GitService::abortMerge(const QString& repoPath) const
{
    return toVoidResult(m_executor.run(
        QStringList() << QStringLiteral("merge") << QStringLiteral("--abort"), repoPath));
}

Result<void> GitService::continueRebase(const QString& repoPath) const
{
    return toVoidResult(m_executor.run(
        QStringList() << QStringLiteral("rebase") << QStringLiteral("--continue"), repoPath));
}

Result<void> GitService::abortRebase(const QString& repoPath) const
{
    return toVoidResult(m_executor.run(
        QStringList() << QStringLiteral("rebase") << QStringLiteral("--abort"), repoPath));
}

// ============================================================================
// 仓库状态检测
// ============================================================================

Result<bool> GitService::isMerging(const QString& repoPath) const
{
    const Result<GitCommandExecutor::CommandResult> gitDirResult = m_executor.run(
        QStringList() << QStringLiteral("rev-parse")
                      << QStringLiteral("--git-dir"),
        repoPath);

    if (gitDirResult.isFailure() || !gitDirResult.value().isSuccess()) {
        return Result<bool>::failure(
            QStringLiteral("Cannot determine git directory."));
    }

    const QString gitDir = gitDirResult.value().standardOutput.trimmed();
    const QString mergeHeadPath = gitDir + QStringLiteral("/MERGE_HEAD");

    return Result<bool>::success(QFileInfo::exists(mergeHeadPath));
}

Result<bool> GitService::isRebasing(const QString& repoPath) const
{
    const Result<GitCommandExecutor::CommandResult> gitDirResult = m_executor.run(
        QStringList() << QStringLiteral("rev-parse")
                      << QStringLiteral("--git-dir"),
        repoPath);

    if (gitDirResult.isFailure() || !gitDirResult.value().isSuccess()) {
        return Result<bool>::failure(
            QStringLiteral("Cannot determine git directory."));
    }

    const QString gitDir = gitDirResult.value().standardOutput.trimmed();
    const QString rebaseMergeDir = gitDir + QStringLiteral("/rebase-merge");
    const QString rebaseApplyDir = gitDir + QStringLiteral("/rebase-apply");

    return Result<bool>::success(
        QFileInfo::exists(rebaseMergeDir) || QFileInfo::exists(rebaseApplyDir));
}

Result<bool> GitService::isWorkingTreeClean(const QString& repoPath) const
{
    const Result<QString> statusResult = m_executor.statusPorcelain(repoPath);

    if (statusResult.isFailure()) {
        return Result<bool>::failure(statusResult.errorMessage());
    }

    return Result<bool>::success(statusResult.value().trimmed().isEmpty());
}

// ============================================================================
// 冲突检测（Phase 3）
// ============================================================================

Result<bool> GitService::hasConflicts(const QString& repoPath) const
{
    const Result<QString> statusResult = m_executor.statusPorcelain(repoPath);
    if (statusResult.isFailure()) {
        return Result<bool>::failure(statusResult.errorMessage());
    }

    // 检查 porcelain 输出中是否包含冲突标记（U）
    const QString output = statusResult.value();
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        if (line.length() >= 2) {
            const QString code = line.left(2);
            if (code.contains(QLatin1Char('U'))
                || code == QStringLiteral("AA")
                || code == QStringLiteral("DD")) {
                return Result<bool>::success(true);
            }
        }
    }

    return Result<bool>::success(false);
}

Result<std::vector<ConflictFile>> GitService::listConflictFiles(
    const QString& repoPath) const
{
    const Result<QString> statusResult = m_executor.statusPorcelain(repoPath);
    if (statusResult.isFailure()) {
        return Result<std::vector<ConflictFile>>::failure(statusResult.errorMessage());
    }

    // 同时检测 merge/rebase 状态，确定冲突类型
    QString conflictType = QStringLiteral("merge");
    {
        const Result<bool> rebasingResult = isRebasing(repoPath);
        if (rebasingResult.isSuccess() && rebasingResult.value()) {
            conflictType = QStringLiteral("rebase");
        }
    }

    std::vector<ConflictFile> conflicts;
    const QString output = statusResult.value();
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        if (line.length() < 3) continue;

        const QString code = line.left(2);
        if (code.contains(QLatin1Char('U'))
            || code == QStringLiteral("AA")
            || code == QStringLiteral("DD")) {
            ConflictFile cf;
            cf.setPath(line.mid(3).trimmed());
            cf.setConflictType(conflictType);
            if (cf.isValid()) {
                conflicts.push_back(cf);
            }
        }
    }

    return Result<std::vector<ConflictFile>>::success(conflicts);
}

Result<void> GitService::markResolved(const QString& repoPath,
                                       const QString& filePath) const
{
    return stageFile(repoPath, filePath);
}

Result<void> GitService::checkoutOurs(const QString& repoPath,
                                       const QString& filePath) const
{
    return toVoidResult(m_executor.run(
        QStringList() << QStringLiteral("checkout")
                      << QStringLiteral("--ours")
                      << filePath,
        repoPath));
}

Result<void> GitService::checkoutTheirs(const QString& repoPath,
                                         const QString& filePath) const
{
    return toVoidResult(m_executor.run(
        QStringList() << QStringLiteral("checkout")
                      << QStringLiteral("--theirs")
                      << filePath,
        repoPath));
}

// ============================================================================
// 三方合并工具（Phase 4）
// ============================================================================

Result<QString> GitService::readOursFile(const QString& repoPath,
                                          const QString& filePath) const
{
    const Result<GitCommandExecutor::CommandResult> result = m_executor.run(
        QStringList() << QStringLiteral("show")
                      << QStringLiteral(":2:") + filePath,
        repoPath);
    if (result.isFailure())
        return Result<QString>::failure(result.errorMessage());
    if (!result.value().isSuccess())
        return Result<QString>::failure(result.value().message());
    return Result<QString>::success(result.value().standardOutput);
}

Result<QString> GitService::readTheirsFile(const QString& repoPath,
                                            const QString& filePath) const
{
    const Result<GitCommandExecutor::CommandResult> result = m_executor.run(
        QStringList() << QStringLiteral("show")
                      << QStringLiteral(":3:") + filePath,
        repoPath);
    if (result.isFailure())
        return Result<QString>::failure(result.errorMessage());
    if (!result.value().isSuccess())
        return Result<QString>::failure(result.value().message());
    return Result<QString>::success(result.value().standardOutput);
}

Result<QString> GitService::readBaseFile(const QString& repoPath,
                                          const QString& filePath) const
{
    const Result<GitCommandExecutor::CommandResult> result = m_executor.run(
        QStringList() << QStringLiteral("show")
                      << QStringLiteral(":1:") + filePath,
        repoPath);
    if (result.isFailure())
        return Result<QString>::failure(result.errorMessage());
    if (!result.value().isSuccess())
        return Result<QString>::failure(result.value().message());
    return Result<QString>::success(result.value().standardOutput);
}

Result<void> GitService::writeResolvedFile(const QString& repoPath,
                                            const QString& filePath,
                                            const QString& content) const
{
    QFile file(repoPath + QStringLiteral("/") + filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return Result<void>::failure(
            QStringLiteral("Cannot write to file: %1").arg(filePath));
    }
    QTextStream stream(&file);
    stream << content;
    file.close();
    return Result<void>::success();
}

// ============================================================================
// Remote 管理
// ============================================================================

Result<QStringList> GitService::listRemotes(const QString& repoPath) const
{
    const Result<GitCommandExecutor::CommandResult> result = m_executor.run(
        QStringList() << QStringLiteral("remote"), repoPath);

    if (result.isFailure()) {
        return Result<QStringList>::failure(result.errorMessage());
    }

    if (!result.value().isSuccess()) {
        return Result<QStringList>::failure(result.value().message());
    }

    const QStringList remotes = result.value().standardOutput
                                    .split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    QStringList trimmedRemotes;
    trimmedRemotes.reserve(remotes.size());

    for (const QString& remote : remotes) {
        const QString trimmed = remote.trimmed();

        if (!trimmed.isEmpty()) {
            trimmedRemotes.append(trimmed);
        }
    }

    return Result<QStringList>::success(trimmedRemotes);
}

Result<void> GitService::addRemote(const QString& repoPath,
                                   const QString& name,
                                   const QString& url) const
{
    const QString cleanName = name.trimmed();
    const QString cleanUrl = url.trimmed();

    if (cleanName.isEmpty()) {
        return Result<void>::failure(QStringLiteral("Remote name cannot be empty."));
    }

    if (cleanUrl.isEmpty()) {
        return Result<void>::failure(QStringLiteral("Remote URL cannot be empty."));
    }

    return toVoidResult(m_executor.run(
        QStringList() << QStringLiteral("remote")
                      << QStringLiteral("add")
                      << cleanName
                      << cleanUrl,
        repoPath));
}

Result<void> GitService::removeRemote(const QString& repoPath,
                                      const QString& name) const
{
    const QString cleanName = name.trimmed();

    if (cleanName.isEmpty()) {
        return Result<void>::failure(QStringLiteral("Remote name cannot be empty."));
    }

    return toVoidResult(m_executor.run(
        QStringList() << QStringLiteral("remote")
                      << QStringLiteral("remove")
                      << cleanName,
        repoPath));
}

Result<std::vector<Remote>> GitService::remoteDetails(const QString& repoPath) const
{
    const Result<QString> result = m_executor.remoteVerbose(repoPath);

    if (result.isFailure()) {
        return Result<std::vector<Remote>>::failure(result.error());
    }

    std::vector<Remote> remotes;
    const QStringList lines = result.value().split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }

        // 格式: "<name>\t<url> (fetch)" 或 "<name>\t<url> (push)"
        const int tabIndex = trimmed.indexOf(QLatin1Char('\t'));
        if (tabIndex < 0) {
            continue;
        }

        const QString name = trimmed.left(tabIndex).trimmed();
        QString remainder = trimmed.mid(tabIndex + 1).trimmed();

        const bool isPush = remainder.endsWith(QStringLiteral(" (push)"));
        if (remainder.endsWith(QStringLiteral(" (fetch)"))) {
            remainder.chop(8);
        } else if (isPush) {
            remainder.chop(7);
        }
        const QString url = remainder.trimmed();

        if (name.isEmpty()) {
            continue;
        }

        // 若已存在同名 remote 则更新 pushUrl，否则新增
        auto it = std::find_if(remotes.begin(), remotes.end(),
                               [&name](const Remote& r) { return r.name() == name; });
        if (it != remotes.end()) {
            if (isPush) {
                it->setPushUrl(url);
            } else if (it->url().isEmpty()) {
                it->setUrl(url);
            }
        } else {
            Remote remote(name);
            if (isPush) {
                remote.setPushUrl(url);
            } else {
                remote.setUrl(url);
            }
            remotes.push_back(remote);
        }
    }

    return Result<std::vector<Remote>>::success(remotes);
}

Result<void> GitService::renameRemote(const QString& repoPath,
                                     const QString& oldName,
                                     const QString& newName) const
{
    const QString cleanOld = oldName.trimmed();
    if (cleanOld.isEmpty()) {
        return Result<void>::failure(
            Error::invalidArgument(QStringLiteral("Old remote name cannot be empty.")));
    }

    const QString cleanNew = newName.trimmed();
    if (cleanNew.isEmpty()) {
        return Result<void>::failure(
            Error::invalidArgument(QStringLiteral("New remote name cannot be empty.")));
    }

    return m_executor.remoteRename(repoPath, cleanOld, cleanNew);
}

Result<void> GitService::setRemoteUrl(const QString& repoPath,
                                     const QString& name,
                                     const QString& url) const
{
    const QString cleanName = name.trimmed();
    if (cleanName.isEmpty()) {
        return Result<void>::failure(
            Error::invalidArgument(QStringLiteral("Remote name cannot be empty.")));
    }

    const QString cleanUrl = url.trimmed();
    if (cleanUrl.isEmpty()) {
        return Result<void>::failure(
            Error::invalidArgument(QStringLiteral("Remote URL cannot be empty.")));
    }

    return m_executor.remoteSetUrl(repoPath, cleanName, cleanUrl);
}

Result<QString> GitService::readConfig(const QString& key,
                                        const QString& repoPath,
                                        bool global) const
{
    return m_executor.readConfig(key, repoPath, global);
}

Result<void> GitService::writeConfig(const QString& key,
                                      const QString& value,
                                      const QString& repoPath,
                                      bool global) const
{
    return m_executor.writeConfig(key, value, repoPath, global);
}

// ============================================================================
// Stash 管理
// ============================================================================

Result<std::vector<Stash>> GitService::listStashes(const QString& repoPath) const
{
    const Result<QString> result = m_executor.stashList(repoPath);

    if (result.isFailure()) {
        return Result<std::vector<Stash>>::failure(result.error());
    }

    return Result<std::vector<Stash>>::success(parseStashListOutput(result.value()));
}

Result<void> GitService::createStash(const QString& repoPath, const QString& message) const
{
    const QString cleanMessage = message.trimmed();

    return m_executor.stashPush(repoPath, cleanMessage);
}

Result<void> GitService::applyStash(const QString& repoPath, int index) const
{
    if (index < 0) {
        return Result<void>::failure(QStringLiteral("Stash index cannot be negative."));
    }

    return m_executor.stashApply(repoPath, index);
}

Result<void> GitService::dropStash(const QString& repoPath, int index) const
{
    if (index < 0) {
        return Result<void>::failure(QStringLiteral("Stash index cannot be negative."));
    }

    return m_executor.stashDrop(repoPath, index);
}

Result<Diff> GitService::showStashDiff(const QString& repoPath, int index) const
{
    if (index < 0) {
        return Result<Diff>::failure(QStringLiteral("Stash index cannot be negative."));
    }

    const Result<QString> result = m_executor.stashDiff(repoPath, index);

    if (result.isFailure()) {
        return Result<Diff>::failure(result.error());
    }

    return Result<Diff>::success(Diff::fromUnifiedDiff(result.value()));
}

// ============================================================================
// AI Commit Message（Phase 5）
// ============================================================================

Result<QString> GitService::stagedDiffRaw(const QString& repoPath) const
{
    return rawDiff(repoPath, QString(), true);
}

void GitService::setCommandLogCallback(CommandLogCallback callback)
{
    m_executor.setCommandLogCallback(std::move(callback));
}
