#include "CommitMessageService.h"

#include "AIService.h"
#include "domain/CommitMessageSuggestion.h"
#include "services/gitservice.h"

#include <QString>

namespace {

const char* SystemPromptTemplate = R"(
You are an expert Git commit message generator. Analyze the git diff
and generate a commit message following the Conventional Commits
specification (https://www.conventionalcommits.org/).

Rules:
1. Determine the type: feat, fix, chore, docs, refactor, test, style, perf, ci, build
2. If applicable, include the scope in parentheses (e.g., "auth", "ui", "api")
3. Write a concise subject line (maximum 72 characters) in imperative mood
4. If the changes are complex or non-obvious, provide a body explaining WHY
   (not what — the diff already shows what changed)
5. Respond ONLY with a valid JSON object in this exact format:
   {"type": "...", "scope": "...", "subject": "...", "body": "..."}
6. If scope is not clear from the diff, use an empty string for scope
7. The body field may be an empty string for simple changes
8. Do NOT include any text outside the JSON object
)";

} // anonymous namespace

CommitMessageService::CommitMessageService(GitService* gitService,
                                           AIService* aiService)
    : m_gitService(gitService)
    , m_aiService(aiService)
{
}

Result<CommitMessageSuggestion> CommitMessageService::generateFromStagedDiff(
    const QString& repoPath)
{
    if (!m_gitService) {
        return Result<CommitMessageSuggestion>::failure(
            QStringLiteral("Git service is not available."));
    }

    // 获取暂存区 diff
    const Result<QString> diffResult = m_gitService->stagedDiffRaw(repoPath);

    if (diffResult.isFailure()) {
        return Result<CommitMessageSuggestion>::failure(diffResult.errorMessage());
    }

    const QString diffText = diffResult.value().trimmed();

    if (diffText.isEmpty()) {
        return Result<CommitMessageSuggestion>::failure(
            QStringLiteral("No staged changes found. Stage files before generating a commit message."));
    }

    return generateFromDiff(diffText);
}

Result<CommitMessageSuggestion> CommitMessageService::generateFromDiff(
    const QString& diffText)
{
    if (!m_aiService) {
        return Result<CommitMessageSuggestion>::failure(
            QStringLiteral("AI service is not available."));
    }

    if (diffText.trimmed().isEmpty()) {
        return Result<CommitMessageSuggestion>::failure(
            QStringLiteral("Diff text is empty."));
    }

    // 限制 diff 长度（避免超出 AI token 限制）
    QString truncatedDiff = diffText;
    const int maxDiffLength = 8000;
    if (truncatedDiff.length() > maxDiffLength) {
        truncatedDiff = truncatedDiff.left(maxDiffLength);
        truncatedDiff += QStringLiteral("\n\n... (diff truncated, total %1 characters)")
                             .arg(diffText.length());
    }

    // 构建 prompt 并调用 AI
    const QString systemPrompt = buildSystemPrompt();
    const QString userPrompt   = buildUserPrompt(truncatedDiff);

    const Result<QString> aiResult = m_aiService->complete(systemPrompt, userPrompt);

    if (aiResult.isFailure()) {
        return Result<CommitMessageSuggestion>::failure(aiResult.errorMessage());
    }

    // 解析 AI 响应
    CommitMessageSuggestion suggestion = parseAIResponse(aiResult.value());

    if (!suggestion.isValid()) {
        return Result<CommitMessageSuggestion>::failure(
            QStringLiteral("Failed to parse AI response into a valid commit message. "
                           "Raw response: %1").arg(aiResult.value().left(200)));
    }

    return Result<CommitMessageSuggestion>::success(suggestion);
}

QString CommitMessageService::buildSystemPrompt() const
{
    return QString::fromUtf8(SystemPromptTemplate).trimmed();
}

QString CommitMessageService::buildUserPrompt(const QString& diffText) const
{
    return QStringLiteral(
        "Generate a commit message for the following staged changes:\n\n%1")
        .arg(diffText);
}

CommitMessageSuggestion CommitMessageService::parseAIResponse(
    const QString& response) const
{
    return CommitMessageSuggestion::fromJson(response);
}
