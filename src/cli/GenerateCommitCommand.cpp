#include "GenerateCommitCommand.h"

#include "domain/CommitMessageSuggestion.h"
#include "services/CommitMessageService.h"
#include "services/SettingsService.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>

GenerateCommitCommand::GenerateCommitCommand(
    CommitMessageService* commitMessageService,
    SettingsService* settingsService)
    : m_commitMessageService(commitMessageService)
    , m_settingsService(settingsService)
{
}

QString GenerateCommitCommand::name() const
{
    return QStringLiteral("commit-message");
}

QString GenerateCommitCommand::description() const
{
    return QStringLiteral("Generate AI commit message from staged changes");
}

QString GenerateCommitCommand::usage() const
{
    return QStringLiteral("commit-message --repo <path>");
}

CliResult GenerateCommitCommand::run(const QStringList& args)
{
    // 1. 基础设施检查：SettingsService 是否存在
    if (!m_settingsService) {
        return CliResult::failure(CliExitCode::AINotConfigured,
                                  QStringLiteral("sakigit commit-message: error: "
                                                 "Settings service not available."));
    }

    // 2. 解析并验证仓库路径（先于 AI 配置检查，提供更好的错误提示）
    const QString repoPath = extractRepoPath(args);

    const QFileInfo repoInfo(repoPath);
    if (!repoInfo.exists() || !repoInfo.isDir()) {
        return CliResult::failure(CliExitCode::GeneralError,
                                  QStringLiteral("sakigit commit-message: error: "
                                                 "Repository not found: %1").arg(repoPath));
    }

    const QDir repoDir(repoPath);
    if (!repoDir.exists(QStringLiteral(".git"))) {
        return CliResult::failure(CliExitCode::GeneralError,
                                  QStringLiteral("sakigit commit-message: error: "
                                                 "Not a Git repository: %1").arg(repoPath));
    }

    // 3. AI 配置检查
    const QString apiKey = m_settingsService->aiApiKey();
    if (apiKey.isEmpty()) {
        return CliResult::failure(CliExitCode::AINotConfigured,
                                  QStringLiteral("sakigit commit-message: error: "
                                                 "AI service not configured.\n"
                                                 "Please set up your AI provider, API key, "
                                                 "and model in SakiGit Settings."));
    }

    // 4. 服务可用性检查
    if (!m_commitMessageService) {
        return CliResult::failure(CliExitCode::GeneralError,
                                  QStringLiteral("sakigit commit-message: error: "
                                                 "Commit message service not available."));
    }

    // 5. 调用 CommitMessageService 生成提交信息
    const auto result = m_commitMessageService->generateFromStagedDiff(repoPath);

    if (!result.isSuccess()) {
        // 检查是否因为无暂存变更
        const QString errorMsg = result.errorMessage();
        if (errorMsg.contains(QStringLiteral("stage"), Qt::CaseInsensitive) ||
            errorMsg.contains(QStringLiteral("暂存"), Qt::CaseInsensitive)) {
            return CliResult::failure(CliExitCode::GeneralError,
                                      QStringLiteral("sakigit commit-message: error: "
                                                     "No staged changes found. "
                                                     "Please stage files first using 'git add'."));
        }

        return CliResult::failure(CliExitCode::AIServiceError,
                                  QStringLiteral("sakigit commit-message: error: %1")
                                      .arg(errorMsg));
    }

    // 6. 格式化输出
    const QString jsonOutput = formatOutput(result.value());
    return CliResult::success(jsonOutput);
}

QString GenerateCommitCommand::extractRepoPath(const QStringList& args) const
{
    for (int i = 0; i < args.size(); ++i) {
        if (args.at(i) == QStringLiteral("--repo")) {
            if (i + 1 < args.size()) {
                return args.at(i + 1);
            }
        }
    }
    // 默认当前目录
    return QStringLiteral(".");
}

QString GenerateCommitCommand::formatOutput(
    const CommitMessageSuggestion& suggestion) const
{
    QJsonObject obj;
    obj[QStringLiteral("success")]  = suggestion.isValid();
    obj[QStringLiteral("type")]     = suggestion.type();
    obj[QStringLiteral("scope")]    = suggestion.scope();
    obj[QStringLiteral("subject")]  = suggestion.subject();
    obj[QStringLiteral("body")]     = suggestion.body();
    obj[QStringLiteral("fullMessage")] = suggestion.fullMessage();

    QJsonDocument doc(obj);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
}
