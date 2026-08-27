#include "branchcontroller.h"

#include "MergeController.h"
#include "RebaseController.h"
#include "domain/branch.h"
#include "domain/GitOperationResult.h"
#include "models/branchlistmodel.h"
#include "services/branchservice.h"
#include "services/GitTaskRunner.h"
#include "ui/MergeDialog.h"
#include "ui/RebaseDialog.h"

#include <QList>
#include <vector>

BranchController::BranchController(BranchService* branchService,
                                   BranchListModel* branchListModel,
                                   QObject* parent)
    : BaseController(parent),
    m_branchService(branchService),
    m_branchListModel(branchListModel)
{
}

void BranchController::loadBranches(const QString& repoPath)
{
    if (!m_branchService) {
        emit branchesLoaded(false, QStringLiteral("Branch service is not available."));
        return;
    }

    const Result<QList<Branch>> result = m_branchService->listBranches(repoPath);

    if (result.isFailure()) {
        emit branchesLoaded(false, result.errorMessage());
        emit errorOccurred(QStringLiteral("loadBranches"), result.errorMessage());
        return;
    }

    if (m_branchListModel) {
        const QList<Branch>& branches = result.value();
        const std::vector<Branch> branchVector(branches.begin(), branches.end());
        m_branchListModel->setBranches(branchVector);
    }

    emit branchesLoaded(true, QString());
}

Result<void> BranchController::createBranch(const QString& repoPath, const QString& branchName)
{
    if (m_taskRunner) {
        m_currentRepoPath = repoPath;
        m_taskRunner->createBranch(repoPath, branchName);
        return Result<void>::success();
    }

    // 回退：同步执行
    if (!m_branchService) {
        const QString errorMessage = QStringLiteral("Branch service is not available.");
        emit operationFinished(QStringLiteral("createBranch"), false, errorMessage);
        return Result<void>::failure(errorMessage);
    }

    const Result<void> result = m_branchService->createBranch(repoPath, branchName);

    if (result.isFailure()) {
        emit operationFinished(QStringLiteral("createBranch"), false, result.errorMessage());
        emit errorOccurred(QStringLiteral("createBranch"), result.errorMessage());
        return result;
    }

    loadBranches(repoPath);

    emit operationFinished(QStringLiteral("createBranch"), true, QString());
    return Result<void>::success();
}

Result<void> BranchController::checkoutBranch(const QString& repoPath, const QString& branchName)
{
    if (m_taskRunner) {
        m_currentRepoPath = repoPath;
        m_taskRunner->checkoutBranch(repoPath, branchName);
        return Result<void>::success();
    }

    // 回退：同步执行
    if (!m_branchService) {
        const QString errorMessage = QStringLiteral("Branch service is not available.");
        emit operationFinished(QStringLiteral("checkoutBranch"), false, errorMessage);
        return Result<void>::failure(errorMessage);
    }

    const Result<void> result = m_branchService->checkoutBranch(repoPath, branchName);

    if (result.isFailure()) {
        emit operationFinished(QStringLiteral("checkoutBranch"), false, result.errorMessage());
        emit errorOccurred(QStringLiteral("checkoutBranch"), result.errorMessage());
        return result;
    }

    loadBranches(repoPath);

    emit operationFinished(QStringLiteral("checkoutBranch"), true, QString());
    return Result<void>::success();
}

Result<void> BranchController::deleteBranch(const QString& repoPath, const QString& branchName, bool force)
{
    if (m_taskRunner) {
        m_currentRepoPath = repoPath;
        m_taskRunner->deleteBranch(repoPath, branchName, force);
        return Result<void>::success();
    }

    // 回退：同步执行
    if (!m_branchService) {
        const QString errorMessage = QStringLiteral("Branch service is not available.");
        emit operationFinished(QStringLiteral("deleteBranch"), false, errorMessage);
        return Result<void>::failure(errorMessage);
    }

    const Result<void> result = m_branchService->deleteBranch(repoPath, branchName, force);

    if (result.isFailure()) {
        emit operationFinished(QStringLiteral("deleteBranch"), false, result.errorMessage());
        emit errorOccurred(QStringLiteral("deleteBranch"), result.errorMessage());
        return result;
    }

    loadBranches(repoPath);

    emit operationFinished(QStringLiteral("deleteBranch"), true, QString());
    return Result<void>::success();
}

Result<void> BranchController::mergeBranch(const QString& repoPath, const QString& branchName)
{
    if (!m_branchService) {
        const QString errorMessage = QStringLiteral("Branch service is not available.");
        emit operationFinished(QStringLiteral("mergeBranch"), false, errorMessage);
        return Result<void>::failure(errorMessage);
    }

    const Result<void> result = m_branchService->mergeBranch(repoPath, branchName);

    if (result.isFailure()) {
        emit operationFinished(QStringLiteral("mergeBranch"), false, result.errorMessage());
        emit errorOccurred(QStringLiteral("mergeBranch"), result.errorMessage());
        return result;
    }

    emit operationFinished(QStringLiteral("mergeBranch"), true, QString());
    return Result<void>::success();
}

Result<void> BranchController::renameBranch(const QString& repoPath, const QString& oldName, const QString& newName)
{
    if (!m_branchService) {
        const QString errorMessage = QStringLiteral("Branch service is not available.");
        emit operationFinished(QStringLiteral("renameBranch"), false, errorMessage);
        return Result<void>::failure(errorMessage);
    }

    const Result<void> result = m_branchService->renameBranch(repoPath, oldName, newName);

    if (result.isFailure()) {
        emit operationFinished(QStringLiteral("renameBranch"), false, result.errorMessage());
        emit errorOccurred(QStringLiteral("renameBranch"), result.errorMessage());
        return result;
    }

    // 重命名成功后自动刷新分支列表
    loadBranches(repoPath);

    emit operationFinished(QStringLiteral("renameBranch"), true, QString());
    return Result<void>::success();
}

Result<QString> BranchController::currentBranchName(const QString& repoPath) const
{
    if (!m_branchService) {
        return Result<QString>::failure(QStringLiteral("Branch service is not available."));
    }

    return m_branchService->currentBranch(repoPath);
}

BranchListModel* BranchController::branchListModel() const
{
    return m_branchListModel;
}

void BranchController::clear()
{
    if (m_branchListModel) {
        m_branchListModel->clear();
    }
}

// ============================================================================
// Merge / Rebase 控制器集成
// ============================================================================

void BranchController::setMergeController(MergeController* controller)
{
    if (m_mergeController) {
        QObject::disconnect(m_mergeController, nullptr, this, nullptr);
    }
    m_mergeController = controller;
    connectMergeRebaseSignals();
}

void BranchController::setRebaseController(RebaseController* controller)
{
    if (m_rebaseController) {
        QObject::disconnect(m_rebaseController, nullptr, this, nullptr);
    }
    m_rebaseController = controller;
    connectMergeRebaseSignals();
}

void BranchController::setGitTaskRunner(GitTaskRunner* taskRunner)
{
    if (m_taskRunner) {
        QObject::disconnect(m_taskRunner, &GitTaskRunner::branchesLoaded,
                            this, &BranchController::onBranchesLoaded);
        QObject::disconnect(m_taskRunner, &GitTaskRunner::branchCreated,
                            this, &BranchController::onBranchCreated);
        QObject::disconnect(m_taskRunner, &GitTaskRunner::branchCheckedOut,
                            this, &BranchController::onBranchCheckedOut);
        QObject::disconnect(m_taskRunner, &GitTaskRunner::branchDeleted,
                            this, &BranchController::onBranchDeleted);
    }

    m_taskRunner = taskRunner;

    if (m_taskRunner) {
        QObject::connect(m_taskRunner, &GitTaskRunner::branchesLoaded,
                         this, &BranchController::onBranchesLoaded);
        QObject::connect(m_taskRunner, &GitTaskRunner::branchCreated,
                         this, &BranchController::onBranchCreated);
        QObject::connect(m_taskRunner, &GitTaskRunner::branchCheckedOut,
                         this, &BranchController::onBranchCheckedOut);
        QObject::connect(m_taskRunner, &GitTaskRunner::branchDeleted,
                         this, &BranchController::onBranchDeleted);
    }
}

MergeController* BranchController::mergeController() const
{
    return m_mergeController;
}

RebaseController* BranchController::rebaseController() const
{
    return m_rebaseController;
}

void BranchController::showMergeDialog(const QString& repoPath,
                                         const QString& currentBranch)
{
    m_currentRepoPath = repoPath;  // 保存路径供后续刷新使用

    if (!m_mergeController) {
        // 回退到简单合并流程
        emit errorOccurred(QStringLiteral("merge"),
                           QStringLiteral("Merge controller is not available."));
        return;
    }

    // 获取分支列表用于对话框
    if (!m_branchService) {
        emit errorOccurred(QStringLiteral("merge"),
                           QStringLiteral("Branch service is not available."));
        return;
    }

    const Result<QList<Branch>> branchResult = m_branchService->listBranches(repoPath);
    if (branchResult.isFailure()) {
        emit errorOccurred(QStringLiteral("merge"),
                           QStringLiteral("无法加载分支列表: ") + branchResult.errorMessage());
        return;
    }

    QStringList branchNames;
    for (const Branch& branch : branchResult.value()) {
        if (branch.isLocal() && branch.name() != currentBranch) {
            branchNames.append(branch.name());
        }
    }

    if (branchNames.isEmpty()) {
        emit errorOccurred(QStringLiteral("merge"),
                           QStringLiteral("没有可合并的其他本地分支。"));
        return;
    }

    // 创建对话框并连接
    auto* dialog = new MergeDialog(currentBranch, branchNames);
    QObject::connect(dialog, &MergeDialog::mergeConfirmed, this, [this, repoPath](const QString& branchName) {
        if (m_mergeController) {
            m_mergeController->performMerge(repoPath, branchName);
        }
    });
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void BranchController::showRebaseDialog(const QString& repoPath,
                                          const QString& currentBranch)
{
    m_currentRepoPath = repoPath;  // 保存路径供后续刷新使用

    if (!m_rebaseController) {
        emit errorOccurred(QStringLiteral("rebase"),
                           QStringLiteral("Rebase controller is not available."));
        return;
    }

    // 获取分支列表用于对话框
    if (!m_branchService) {
        emit errorOccurred(QStringLiteral("rebase"),
                           QStringLiteral("Branch service is not available."));
        return;
    }

    const Result<QList<Branch>> branchResult = m_branchService->listBranches(repoPath);
    if (branchResult.isFailure()) {
        emit errorOccurred(QStringLiteral("rebase"),
                           QStringLiteral("无法加载分支列表: ") + branchResult.errorMessage());
        return;
    }

    QStringList branchNames;
    for (const Branch& branch : branchResult.value()) {
        if (branch.isLocal() && branch.name() != currentBranch) {
            branchNames.append(branch.name());
        }
    }

    if (branchNames.isEmpty()) {
        emit errorOccurred(QStringLiteral("rebase"),
                           QStringLiteral("没有可用的其他本地分支作为变基目标。"));
        return;
    }

    // 创建对话框并连接
    auto* dialog = new RebaseDialog(currentBranch, branchNames);
    QObject::connect(dialog, &RebaseDialog::rebaseConfirmed, this, [this, repoPath](const QString& branchName) {
        if (m_rebaseController) {
            m_rebaseController->performRebase(repoPath, branchName);
        }
    });
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

// -- 私有槽函数 --

void BranchController::onMergeCompleted(const GitOperationResult& result)
{
    if (m_taskRunner) {
        m_taskRunner->loadBranches(m_currentRepoPath);
    } else {
        loadBranches(m_currentRepoPath);
    }
    emit mergeRebaseCompleted(QStringLiteral("merge"), result.summary());
}

void BranchController::onMergeConflict(const GitOperationResult& result)
{
    if (m_taskRunner) {
        m_taskRunner->loadBranches(m_currentRepoPath);
    } else {
        loadBranches(m_currentRepoPath);
    }
    emit mergeRebaseCompleted(QStringLiteral("merge"),
                              result.summary() + QStringLiteral("\n冲突文件: ")
                              + result.conflictFiles.join(QStringLiteral(", ")));
    emit errorOccurred(QStringLiteral("merge"), result.summary());
}

void BranchController::onMergeFailed(const QString& errorMessage)
{
    emit mergeRebaseCompleted(QStringLiteral("merge"), errorMessage);
    emit errorOccurred(QStringLiteral("merge"), errorMessage);
}

void BranchController::onRebaseCompleted(const GitOperationResult& result)
{
    if (m_taskRunner) {
        m_taskRunner->loadBranches(m_currentRepoPath);
    } else {
        loadBranches(m_currentRepoPath);
    }
    emit mergeRebaseCompleted(QStringLiteral("rebase"), result.summary());
}

void BranchController::onRebaseConflict(const GitOperationResult& result)
{
    if (m_taskRunner) {
        m_taskRunner->loadBranches(m_currentRepoPath);
    } else {
        loadBranches(m_currentRepoPath);
    }
    emit mergeRebaseCompleted(QStringLiteral("rebase"),
                              result.summary() + QStringLiteral("\n冲突文件: ")
                              + result.conflictFiles.join(QStringLiteral(", ")));
    emit errorOccurred(QStringLiteral("rebase"), result.summary());
}

void BranchController::onRebaseFailed(const QString& errorMessage)
{
    emit mergeRebaseCompleted(QStringLiteral("rebase"), errorMessage);
    emit errorOccurred(QStringLiteral("rebase"), errorMessage);
}

void BranchController::connectMergeRebaseSignals()
{
    if (m_mergeController) {
        QObject::connect(m_mergeController, &MergeController::mergeCompleted,
                         this, &BranchController::onMergeCompleted);
        QObject::connect(m_mergeController, &MergeController::mergeConflict,
                         this, &BranchController::onMergeConflict);
        QObject::connect(m_mergeController, &MergeController::mergeFailed,
                         this, &BranchController::onMergeFailed);
    }

    if (m_rebaseController) {
        QObject::connect(m_rebaseController, &RebaseController::rebaseCompleted,
                         this, &BranchController::onRebaseCompleted);
        QObject::connect(m_rebaseController, &RebaseController::rebaseConflict,
                         this, &BranchController::onRebaseConflict);
        QObject::connect(m_rebaseController, &RebaseController::rebaseFailed,
                         this, &BranchController::onRebaseFailed);
    }
}

// -- GitTaskRunner 异步回调 --

void BranchController::onBranchesLoaded(bool success, const QString& repoPath,
                                         const QList<Branch>& branches,
                                         const QString& errorMessage)
{
    if (!success) {
        emit errorOccurred(QStringLiteral("loadBranches"), errorMessage);
        return;
    }

    if (m_branchListModel) {
        const std::vector<Branch> branchVector(branches.begin(), branches.end());
        m_branchListModel->setBranches(branchVector);
    }

    emit branchesLoaded(true, QString());
}

void BranchController::onBranchCreated(bool success, const QString& repoPath, const QString& branchName, const QString& errorMessage)
{
    if (success) {
        if (m_taskRunner) {
            m_taskRunner->loadBranches(repoPath);
        } else {
            loadBranches(repoPath);
        }
        emit operationFinished(QStringLiteral("createBranch"), true, QString());
    } else {
        emit operationFinished(QStringLiteral("createBranch"), false, errorMessage);
        emit errorOccurred(QStringLiteral("createBranch"), errorMessage);
    }
}

void BranchController::onBranchCheckedOut(bool success, const QString& repoPath, const QString& branchName, const QString& errorMessage)
{
    if (success) {
        // 异步加载分支列表，避免主线程调用同步 QProcess 导致 UI 卡顿
        if (m_taskRunner) {
            m_taskRunner->loadBranches(repoPath);
        } else {
            loadBranches(repoPath);
        }
        emit operationFinished(QStringLiteral("checkoutBranch"), true, QString());
    } else {
        emit operationFinished(QStringLiteral("checkoutBranch"), false, errorMessage);
        emit errorOccurred(QStringLiteral("checkoutBranch"), errorMessage);
    }
}

void BranchController::onBranchDeleted(bool success, const QString& repoPath, const QString& branchName, const QString& errorMessage)
{
    if (success) {
        if (m_taskRunner) {
            m_taskRunner->loadBranches(repoPath);
        } else {
            loadBranches(repoPath);
        }
        emit operationFinished(QStringLiteral("deleteBranch"), true, QString());
    } else {
        emit operationFinished(QStringLiteral("deleteBranch"), false, errorMessage);
        emit errorOccurred(QStringLiteral("deleteBranch"), errorMessage);
    }
}
