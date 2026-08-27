#ifndef CODEREVIEWSERVICE_H
#define CODEREVIEWSERVICE_H

#include "domain/ReviewFinding.h"
#include "infrastructure/result.h"

#include <QString>
#include <vector>

class AIService;
class GitService;

// AI 代码审查服务。
// 组合 GitService（获取 diff）和 AIService（调用 AI），
// 实现「Git diff → AI prompt → vector<ReviewFinding>」的完整流程。
class CodeReviewService
{
public:
    CodeReviewService(GitService* gitService, AIService* aiService);

    // 审查暂存区变更。
    // 对应 CLI: sakigit review --repo . --staged
    Result<std::vector<ReviewFinding>> reviewStagedDiff(
        const QString& repoPath);

    // 审查工作区变更（未暂存）。
    // 对应 CLI: sakigit review --repo .
    Result<std::vector<ReviewFinding>> reviewWorkingTreeDiff(
        const QString& repoPath);

    // 审查指定 commit（与 HistoryPage 联动）
    Result<std::vector<ReviewFinding>> reviewCommit(
        const QString& repoPath, const QString& commitHash);

    // 审查任意 diff 文本。
    // CLI 入口点：支持从 stdin 管道或文件读取 diff 后直接调用，
    // 绕过 GitService 依赖，适用于脚本、Git Hook 和 CI 流水线。
    Result<std::vector<ReviewFinding>> reviewDiff(
        const QString& diffText);

    // 取消正在进行的审查
    void cancel();

    // 是否正在进行审查
    bool isReviewing() const;

private:
    GitService* m_gitService;  // 不持有所有权
    AIService*  m_aiService;   // 不持有所有权
    bool        m_cancelled;

    // 构建 system prompt（定义 AI 的审查角色和行为规范）
    QString buildSystemPrompt() const;

    // 构建 user prompt（包含 diff 内容）
    QString buildUserPrompt(const QString& diffText) const;

    // 解析 AI 响应为发现列表
    std::vector<ReviewFinding> parseAIResponse(const QString& response) const;

    // AI 请求的 diff 最大长度限制
    static constexpr int MaxDiffLength = 15000;
};

#endif // CODEREVIEWSERVICE_H
