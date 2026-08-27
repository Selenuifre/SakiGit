#ifndef AICOMMITCONTROLLER_H
#define AICOMMITCONTROLLER_H

#include "BaseController.h"
#include "domain/CommitMessageSuggestion.h"

#include <QString>

class CommitMessageService;
class CommitMessageSuggestionModel;

// AI 提交信息生成控制器。
// 协调 CommitMessageService 和 UI 组件，
// 处理「生成 → 填充」的交互流程。
class AICommitController : public BaseController
{
    Q_OBJECT

public:
    AICommitController(CommitMessageService* service,
                       CommitMessageSuggestionModel* model,
                       QObject* parent = nullptr);

    // 从指定仓库的暂存区 diff 生成提交信息
    void generate(const QString& repoPath);

    // 将选中的建议应用到 CommitPanel
    QString applySuggestion(int index = 0) const;

    // 检查是否有暂存的变更
    void checkStagedChanges(const QString& repoPath);

    // 返回当前是否有未应用的生成结果
    bool hasCurrentSuggestion() const;

    // 返回最新生成的建议
    CommitMessageSuggestion currentSuggestion() const;

    // 返回建议模型
    CommitMessageSuggestionModel* model() const;

signals:
    // 生成流程开始（用于 UI 显示 loading 状态）
    void generationStarted();

    // 生成成功
    void generationSucceeded(const CommitMessageSuggestion& suggestion);

    // 生成失败
    void generationFailed(const QString& errorMessage);

    // 生成流程结束（无论成功或失败，用于 UI 恢复按钮状态）
    void generationFinished();

    // 暂存区状态变化（有暂存 → 启用 AI 按钮，无暂存 → 禁用）
    void stagedChangesAvailable(bool available);

private:
    CommitMessageService*         m_service;  // 不持有所有权
    CommitMessageSuggestionModel* m_model;    // 不持有所有权

    // 暂存最后生成的建议（以便快速 apply）
    CommitMessageSuggestion m_currentSuggestion;
};

#endif // AICOMMITCONTROLLER_H
