#ifndef BRANCHCONTROLLER_H
#define BRANCHCONTROLLER_H

#include "domain/GitOperationResult.h"
#include "domain/branch.h"
#include "infrastructure/result.h"

#include "BaseController.h"
#include <QList>
#include <QString>

class BranchListModel;
class BranchService;
class GitTaskRunner;
class MergeController;
class RebaseController;

class BranchController : public BaseController
{
    Q_OBJECT

public:
    // 使用分支服务和分支列表模型构造控制器
    explicit BranchController(BranchService* branchService,
                              BranchListModel* branchListModel,
                              QObject* parent = nullptr);

    // 加载分支列表
    void loadBranches(const QString& repoPath);

    // 创建新分支
    Result<void> createBranch(const QString& repoPath, const QString& branchName);

    // 切换到指定分支
    Result<void> checkoutBranch(const QString& repoPath, const QString& branchName);

    // 删除指定分支
    Result<void> deleteBranch(const QString& repoPath, const QString& branchName, bool force = false);

    // 合并指定分支到当前分支
    Result<void> mergeBranch(const QString& repoPath, const QString& branchName);

    // 重命名分支
    Result<void> renameBranch(const QString& repoPath, const QString& oldName, const QString& newName);

    // 获取当前分支名称
    Result<QString> currentBranchName(const QString& repoPath) const;

    // 返回分支列表模型
    BranchListModel* branchListModel() const;

    // 注入 Merge / Rebase 控制器（可选，用于增强版合并/变基流程）
    void setMergeController(MergeController* controller);
    void setRebaseController(RebaseController* controller);
    void setGitTaskRunner(GitTaskRunner* taskRunner);

    // 返回注入的控制器
    MergeController* mergeController() const;
    RebaseController* rebaseController() const;

    // 打开 Merge 对话框（使用 MergeController 的增强流程）
    void showMergeDialog(const QString& repoPath, const QString& currentBranch);

    // 打开 Rebase 对话框（使用 RebaseController 的增强流程）
    void showRebaseDialog(const QString& repoPath, const QString& currentBranch);

    // 清空模型数据
    void clear();

signals:
    // 分支列表加载完成
    void branchesLoaded(bool success, const QString& errorMessage);

    // Merge/Rebase 流程完成（需要 UI 刷新）
    void mergeRebaseCompleted(const QString& operation, const QString& message);

private slots:
    // 处理 MergeController 的合并完成信号
    void onMergeCompleted(const GitOperationResult& result);

    void onMergeConflict(const GitOperationResult& result);
    void onMergeFailed(const QString& errorMessage);
    void onRebaseCompleted(const GitOperationResult& result);
    void onRebaseConflict(const GitOperationResult& result);
    void onRebaseFailed(const QString& errorMessage);

    // GitTaskRunner 异步回调
    void onBranchesLoaded(bool success, const QString& repoPath, const QList<Branch>& branches, const QString& errorMessage);
    void onBranchCreated(bool success, const QString& repoPath, const QString& branchName, const QString& errorMessage);
    void onBranchCheckedOut(bool success, const QString& repoPath, const QString& branchName, const QString& errorMessage);
    void onBranchDeleted(bool success, const QString& repoPath, const QString& branchName, const QString& errorMessage);

private:
    // 连接 MergeController / RebaseController 信号
    void connectMergeRebaseSignals();

    // 连接 GitTaskRunner 信号
    void connectTaskRunnerSignals();

private:
    // 分支业务服务
    BranchService* m_branchService;

    // 分支列表模型
    BranchListModel* m_branchListModel;

    // 可选注入的 Merge/Rebase 控制器
    MergeController* m_mergeController = nullptr;
    RebaseController* m_rebaseController = nullptr;
    GitTaskRunner* m_taskRunner = nullptr;

    // 当前操作的仓库路径（用于 merge/rebase 完成后刷新）
    QString m_currentRepoPath;
};

#endif // BRANCHCONTROLLER_H
