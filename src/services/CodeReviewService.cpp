#include "CodeReviewService.h"

#include "AIService.h"
#include "domain/ReviewFinding.h"
#include "services/gitservice.h"

#include <QString>

namespace {

const char* SystemPromptTemplate = R"PROMPT(
You are an expert code reviewer. Analyze the git diff and identify issues
in the changed code. Focus on:

1. Potential bugs (null pointers, logic errors, race conditions, off-by-one, etc.)
2. Security vulnerabilities (injection risks, insecure data handling, missing validation, etc.)
3. Performance issues (unnecessary copies, inefficient algorithms, blocking calls, etc.)
4. Style and best practice violations
5. Maintainability concerns (overly complex code, missing error handling, unclear naming)

For each finding, classify severity as: critical, high, medium, low, info.
- critical: would cause crash, data loss, or security breach
- high: likely to cause incorrect behavior
- medium: code smell, may cause subtle bugs
- low: style/minor improvement
- info: educational suggestion

Categories: bug, security, performance, style, maintainability, best-practice, typo

Respond ONLY with a valid JSON array. Each element must be:
{
  "file": "path/to/file",
  "line": <number or null for global issues>,
  "severity": "critical|high|medium|low|info",
  "category": "bug|security|performance|style|maintainability|best-practice|typo",
  "title": "one-line summary of the issue",
  "message": "detailed explanation of why this is a problem",
  "suggestion": "how to fix this issue (can be empty string)"
}

If you find no issues, return an empty array [].
Do NOT include any text outside the JSON array.
)PROMPT";

} // anonymous namespace

CodeReviewService::CodeReviewService(GitService* gitService, AIService* aiService)
    : m_gitService(gitService)
    , m_aiService(aiService)
    , m_cancelled(false)
{
}

Result<std::vector<ReviewFinding>> CodeReviewService::reviewStagedDiff(
    const QString& repoPath)
{
    if (!m_gitService) {
        return Result<std::vector<ReviewFinding>>::failure(
            QStringLiteral("Git service is not available."));
    }

    const Result<QString> diffResult = m_gitService->stagedDiffRaw(repoPath);

    if (diffResult.isFailure()) {
        return Result<std::vector<ReviewFinding>>::failure(diffResult.errorMessage());
    }

    const QString diffText = diffResult.value().trimmed();

    if (diffText.isEmpty()) {
        return Result<std::vector<ReviewFinding>>::failure(
            QStringLiteral("No staged changes found. Stage files before reviewing."));
    }

    return reviewDiff(diffText);
}

Result<std::vector<ReviewFinding>> CodeReviewService::reviewWorkingTreeDiff(
    const QString& repoPath)
{
    if (!m_gitService) {
        return Result<std::vector<ReviewFinding>>::failure(
            QStringLiteral("Git service is not available."));
    }

    const Result<QString> diffResult = m_gitService->rawDiff(repoPath, QString(), false);

    if (diffResult.isFailure()) {
        return Result<std::vector<ReviewFinding>>::failure(diffResult.errorMessage());
    }

    const QString diffText = diffResult.value().trimmed();

    if (diffText.isEmpty()) {
        return Result<std::vector<ReviewFinding>>::failure(
            QStringLiteral("No working tree changes found."));
    }

    return reviewDiff(diffText);
}

Result<std::vector<ReviewFinding>> CodeReviewService::reviewCommit(
    const QString& repoPath, const QString& commitHash)
{
    if (!m_gitService) {
        return Result<std::vector<ReviewFinding>>::failure(
            QStringLiteral("Git service is not available."));
    }

    const Result<QString> diffResult = m_gitService->rawCommitDiff(repoPath, commitHash);

    if (diffResult.isFailure()) {
        return Result<std::vector<ReviewFinding>>::failure(diffResult.errorMessage());
    }

    const QString diffText = diffResult.value().trimmed();

    if (diffText.isEmpty()) {
        return Result<std::vector<ReviewFinding>>::failure(
            QStringLiteral("No diff found for commit %1.").arg(commitHash));
    }

    return reviewDiff(diffText);
}

Result<std::vector<ReviewFinding>> CodeReviewService::reviewDiff(
    const QString& diffText)
{
    if (!m_aiService) {
        return Result<std::vector<ReviewFinding>>::failure(
            QStringLiteral("AI service is not available."));
    }

    if (diffText.trimmed().isEmpty()) {
        return Result<std::vector<ReviewFinding>>::failure(
            QStringLiteral("Diff text is empty."));
    }

    m_cancelled = false;

    // 限制 diff 长度（避免超出 AI token 限制）
    QString truncatedDiff = diffText;
    if (truncatedDiff.length() > MaxDiffLength) {
        truncatedDiff = truncatedDiff.left(MaxDiffLength);
        truncatedDiff += QStringLiteral("\n\n... (diff truncated, total %1 characters)")
                             .arg(diffText.length());
    }

    // 构建 prompt 并调用 AI
    const QString systemPrompt = buildSystemPrompt();
    const QString userPrompt   = buildUserPrompt(truncatedDiff);

    const Result<QString> aiResult = m_aiService->complete(systemPrompt, userPrompt);

    if (m_cancelled) {
        return Result<std::vector<ReviewFinding>>::failure(
            QStringLiteral("Review was cancelled."));
    }

    if (aiResult.isFailure()) {
        return Result<std::vector<ReviewFinding>>::failure(aiResult.errorMessage());
    }

    // 解析 AI 响应
    std::vector<ReviewFinding> findings = parseAIResponse(aiResult.value());

    return Result<std::vector<ReviewFinding>>::success(findings);
}

void CodeReviewService::cancel()
{
    m_cancelled = true;
}

bool CodeReviewService::isReviewing() const
{
    return !m_cancelled;
}

QString CodeReviewService::buildSystemPrompt() const
{
    return QString::fromUtf8(SystemPromptTemplate).trimmed();
}

QString CodeReviewService::buildUserPrompt(const QString& diffText) const
{
    return QStringLiteral(
        "Please review the following git diff and identify any issues:\n\n%1")
        .arg(diffText);
}

std::vector<ReviewFinding> CodeReviewService::parseAIResponse(
    const QString& response) const
{
    return ReviewFinding::fromJsonArray(response);
}
