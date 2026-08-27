#ifndef CODEREVIEWCONTROLLER_H
#define CODEREVIEWCONTROLLER_H

#include "BaseController.h"
#include "domain/ReviewFinding.h"

#include <QString>
#include <vector>

class CodeReviewService;
class ReviewFindingModel;

// AI Code Review 控制器。
// 协调 CodeReviewService 和 UI 组件，
// 处理「触发审查 → 展示结果」的交互流程。
class CodeReviewController : public BaseController
{
    Q_OBJECT

public:
    CodeReviewController(CodeReviewService* service,
                         ReviewFindingModel* model,
                         QObject* parent = nullptr);

    // 审查暂存区 diff
    void reviewStaged(const QString& repoPath);

    // 审查工作区 diff（未暂存）
    void reviewWorkingTree(const QString& repoPath);

    // 审查指定 commit
    void reviewCommit(const QString& repoPath, const QString& commitHash);

    // 取消正在进行的审查
    void cancelReview();

    // 返回发现模型
    ReviewFindingModel* model() const;

    // 是否有审查结果
    bool hasFindings() const;

signals:
    // 审查开始（UI 显示 loading）
    void reviewStarted();

    // 审查完成，返回发现列表
    void reviewCompleted(const std::vector<ReviewFinding>& findings);

    // 审查失败
    void reviewFailed(const QString& errorMessage);

    // 审查结束（无论成功或失败）
    void reviewFinished();

private:
    CodeReviewService* m_service;  // 不持有所有权
    ReviewFindingModel* m_model;   // 不持有所有权
};

#endif // CODEREVIEWCONTROLLER_H
