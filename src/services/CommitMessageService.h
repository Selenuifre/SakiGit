#ifndef COMMITMESSAGESERVICE_H
#define COMMITMESSAGESERVICE_H

#include "domain/CommitMessageSuggestion.h"
#include "infrastructure/result.h"

#include <QString>

class AIService;
class GitService;

// 提交信息生成服务。
// 组合 GitService（获取 diff）和 AIService（调用 AI），
// 实现「暂存区 diff → AI prompt → CommitMessageSuggestion」的完整流程。
class CommitMessageService
{
public:
    CommitMessageService(GitService* gitService, AIService* aiService);

    // 核心方法：从暂存区 diff 生成提交信息建议。
    // 供 GUI（AICommitController）和 CLI（GenerateCommitCommand）复用。
    Result<CommitMessageSuggestion> generateFromStagedDiff(
        const QString& repoPath);

    // 从指定的 diff 文本生成提交信息。
    // CLI 入口点：支持从 stdin 管道或文件读取 diff 后直接调用，
    // 绕过 GitService 依赖，适用于脚本和 CI 流水线。
    Result<CommitMessageSuggestion> generateFromDiff(
        const QString& diffText);

private:
    GitService* m_gitService;  // 不持有所有权
    AIService*  m_aiService;   // 不持有所有权

    // 构建 system prompt（定义 AI 的行为规范）
    QString buildSystemPrompt() const;

    // 构建 user prompt（包含 diff 内容）
    QString buildUserPrompt(const QString& diffText) const;

    // 解析 AI 返回的文本为 CommitMessageSuggestion
    CommitMessageSuggestion parseAIResponse(const QString& response) const;
};

#endif // COMMITMESSAGESERVICE_H
