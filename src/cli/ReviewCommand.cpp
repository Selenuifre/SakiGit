#include "ReviewCommand.h"

#include "domain/ReviewFinding.h"
#include "services/CodeReviewService.h"
#include "services/SettingsService.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

ReviewCommand::ReviewCommand(CodeReviewService* codeReviewService,
                             SettingsService* settingsService)
    : m_codeReviewService(codeReviewService)
    , m_settingsService(settingsService)
{
}

QString ReviewCommand::name() const
{
    return QStringLiteral("review");
}

QString ReviewCommand::description() const
{
    return QStringLiteral("AI code review on staged or working tree changes");
}

QString ReviewCommand::usage() const
{
    return QStringLiteral("review --repo <path> [--staged] [--strict]");
}

CliResult ReviewCommand::run(const QStringList& args)
{
    // 1. 基础设施检查：SettingsService 是否存在
    if (!m_settingsService) {
        return CliResult::failure(CliExitCode::AINotConfigured,
                                  QStringLiteral("sakigit review: error: "
                                                 "Settings service not available."));
    }

    // 2. 解析参数并验证仓库路径（先于 AI 配置检查，提供更好的错误提示）
    const QString repoPath = extractRepoPath(args);
    const bool staged = extractStaged(args);
    const bool strict = extractStrict(args);

    const QFileInfo repoInfo(repoPath);
    if (!repoInfo.exists() || !repoInfo.isDir()) {
        return CliResult::failure(CliExitCode::GeneralError,
                                  QStringLiteral("sakigit review: error: "
                                                 "Repository not found: %1").arg(repoPath));
    }

    const QDir repoDir(repoPath);
    if (!repoDir.exists(QStringLiteral(".git"))) {
        return CliResult::failure(CliExitCode::GeneralError,
                                  QStringLiteral("sakigit review: error: "
                                                 "Not a Git repository: %1").arg(repoPath));
    }

    // 3. AI 配置检查
    const QString apiKey = m_settingsService->aiApiKey();
    if (apiKey.isEmpty()) {
        return CliResult::failure(CliExitCode::AINotConfigured,
                                  QStringLiteral("sakigit review: error: "
                                                 "AI service not configured.\n"
                                                 "Please set up your AI provider, API key, "
                                                 "and model in SakiGit Settings."));
    }

    // 4. 服务可用性检查
    if (!m_codeReviewService) {
        return CliResult::failure(CliExitCode::GeneralError,
                                  QStringLiteral("sakigit review: error: "
                                                 "Code review service not available."));
    }

    // 5. 调用 CodeReviewService
    Result<std::vector<ReviewFinding>> result;
    if (staged) {
        result = m_codeReviewService->reviewStagedDiff(repoPath);
    } else {
        result = m_codeReviewService->reviewWorkingTreeDiff(repoPath);
    }

    if (!result.isSuccess()) {
        return CliResult::failure(CliExitCode::AIServiceError,
                                  QStringLiteral("sakigit review: error: %1")
                                      .arg(result.errorMessage()));
    }

    // 5. 格式化输出
    const std::vector<ReviewFinding>& findings = result.value();
    const QString jsonOutput = formatOutput(findings, strict);

    // 6. 严格模式：有 critical/high 级别问题时返回非零退出码
    if (strict) {
        bool hasBlockingIssue = false;
        for (const auto& f : findings) {
            const QString sev = f.severity().toLower();
            if (sev == QStringLiteral("critical") || sev == QStringLiteral("high")) {
                hasBlockingIssue = true;
                break;
            }
        }
        if (hasBlockingIssue) {
            CliResult cliResult;
            cliResult.exitCode = CliExitCode::GeneralError;
            cliResult.stdOut = jsonOutput;
            cliResult.stdErr = QStringLiteral(
                "sakigit review: Blocking issues found (strict mode).\n");
            return cliResult;
        }
    }

    return CliResult::success(jsonOutput);
}

QString ReviewCommand::extractRepoPath(const QStringList& args) const
{
    for (int i = 0; i < args.size(); ++i) {
        if (args.at(i) == QStringLiteral("--repo")) {
            if (i + 1 < args.size()) {
                return args.at(i + 1);
            }
        }
    }
    return QStringLiteral(".");
}

bool ReviewCommand::extractStaged(const QStringList& args) const
{
    return args.contains(QStringLiteral("--staged"));
}

bool ReviewCommand::extractStrict(const QStringList& args) const
{
    return args.contains(QStringLiteral("--strict"));
}

QString ReviewCommand::formatOutput(const std::vector<ReviewFinding>& findings,
                                    bool strictMode) const
{
    QJsonArray findingsArray;

    int criticalCount = 0;
    int highCount     = 0;
    int mediumCount   = 0;
    int lowCount      = 0;
    int infoCount     = 0;

    for (const auto& f : findings) {
        QJsonObject findingObj;
        findingObj[QStringLiteral("file")]       = f.filePath();
        findingObj[QStringLiteral("line")]       = f.lineNumber();
        findingObj[QStringLiteral("severity")]   = f.severity();
        findingObj[QStringLiteral("category")]   = f.category();
        findingObj[QStringLiteral("title")]      = f.title();
        findingObj[QStringLiteral("message")]    = f.message();
        findingObj[QStringLiteral("suggestion")] = f.suggestion();

        if (!f.codeSnippet().isEmpty()) {
            findingObj[QStringLiteral("codeSnippet")] = f.codeSnippet();
        }

        findingsArray.append(findingObj);

        // 统计
        const QString sev = f.severity().toLower();
        if (sev == QStringLiteral("critical"))      ++criticalCount;
        else if (sev == QStringLiteral("high"))     ++highCount;
        else if (sev == QStringLiteral("medium"))   ++mediumCount;
        else if (sev == QStringLiteral("low"))      ++lowCount;
        else if (sev == QStringLiteral("info"))     ++infoCount;
    }

    QJsonObject summaryObj;
    summaryObj[QStringLiteral("total")]    = static_cast<int>(findings.size());
    summaryObj[QStringLiteral("critical")] = criticalCount;
    summaryObj[QStringLiteral("high")]     = highCount;
    summaryObj[QStringLiteral("medium")]   = mediumCount;
    summaryObj[QStringLiteral("low")]      = lowCount;
    summaryObj[QStringLiteral("info")]     = infoCount;

    QJsonObject root;
    root[QStringLiteral("success")]  = true;
    root[QStringLiteral("findings")] = findingsArray;
    root[QStringLiteral("summary")]  = summaryObj;
    root[QStringLiteral("strict")]   = strictMode;
    root[QStringLiteral("hasIssues")] = !findings.empty();

    QJsonDocument doc(root);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
}
