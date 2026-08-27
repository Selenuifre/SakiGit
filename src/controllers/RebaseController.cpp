#include "RebaseController.h"

#include "services/RebaseService.h"
#include "services/GitTaskRunner.h"

#include <QtConcurrent/QtConcurrentRun>

RebaseController::RebaseController(RebaseService* rebaseService,
                                     GitTaskRunner* taskRunner,
                                     QObject* parent)
    : GitOperationController(parent),
      m_rebaseService(rebaseService),
      m_taskRunner(taskRunner)
{
}

void RebaseController::checkPreconditions(const QString& repoPath)
{
    if (!m_rebaseService) {
        emit preconditionsChecked(false,
            QStringLiteral("Rebase service is not available."));
        return;
    }

    // 检查工作区是否干净
    const Result<bool> cleanResult = m_rebaseService->isWorkingTreeClean(repoPath);
    if (cleanResult.isFailure()) {
        emit preconditionsChecked(false,
            QStringLiteral("无法检查工作区状态: ") + cleanResult.errorMessage());
        return;
    }

    if (!cleanResult.value()) {
        emit preconditionsChecked(true,
            QStringLiteral("工作区有未提交的修改，变基前建议先提交或暂存。"));
        return;
    }

    // 检查是否已在变基状态
    const Result<bool> rebasingResult = m_rebaseService->isRebasing(repoPath);
    if (rebasingResult.isSuccess() && rebasingResult.value()) {
        emit preconditionsChecked(false,
            QStringLiteral("仓库当前处于变基冲突状态，请先解决冲突、继续或中止变基。"));
        return;
    }

    emit preconditionsChecked(true, QString());
}

void RebaseController::performRebase(const QString& repoPath,
                                       const QString& branchName)
{
    if (!m_rebaseService) {
        emit rebaseFailed(QStringLiteral("Rebase service is not available."));
        emit errorOccurred(QStringLiteral("rebase"), QStringLiteral("Rebase service is not available."));
        return;
    }

    // 异步执行：使用 QtConcurrent::run 将变基操作放入后台线程
    emit operationStarted(QStringLiteral("rebase"));

    RebaseService* service = m_rebaseService;
    const QString repo = repoPath;
    const QString branch = branchName;

    QtConcurrent::run([service, repo, branch]() {
        return service->rebase(repo, branch);
    }).then([this, repo, branch](const Result<GitOperationResult>& result) {
        emit operationFinished(QStringLiteral("rebase"));

        if (result.isFailure()) {
            emit rebaseFailed(result.errorMessage());
            emit errorOccurred(QStringLiteral("rebase"), result.errorMessage());
            return;
        }

        const GitOperationResult& opResult = result.value();
        if (opResult.hasConflicts) {
            emit rebaseConflict(opResult);
        } else if (opResult.success) {
            emit rebaseCompleted(opResult);
        } else {
            emit rebaseFailed(opResult.message);
        }
    });
}

void RebaseController::continueRebase(const QString& repoPath)
{
    if (!m_rebaseService) {
        emit rebaseFailed(QStringLiteral("Rebase service is not available."));
        emit errorOccurred(QStringLiteral("continueRebase"), QStringLiteral("Rebase service is not available."));
        return;
    }

    emit operationStarted(QStringLiteral("continueRebase"));

    RebaseService* service = m_rebaseService;
    const QString repo = repoPath;

    QtConcurrent::run([service, repo]() {
        return service->continueRebase(repo);
    }).then([this](const Result<void>& result) {
        emit operationFinished(QStringLiteral("continueRebase"));

        if (result.isFailure()) {
            emit rebaseFailed(result.errorMessage());
            emit errorOccurred(QStringLiteral("continueRebase"), result.errorMessage());
            return;
        }

        GitOperationResult opResult;
        opResult.operationType = QStringLiteral("rebase");
        opResult.success = true;
        opResult.message = QStringLiteral("变基已继续");
        emit rebaseCompleted(opResult);
    });
}

void RebaseController::abortRebase(const QString& repoPath)
{
    if (!m_rebaseService) {
        emit rebaseFailed(QStringLiteral("Rebase service is not available."));
        emit errorOccurred(QStringLiteral("abortRebase"), QStringLiteral("Rebase service is not available."));
        return;
    }

    emit operationStarted(QStringLiteral("abortRebase"));

    RebaseService* service = m_rebaseService;
    const QString repo = repoPath;

    QtConcurrent::run([service, repo]() {
        return service->abortRebase(repo);
    }).then([this](const Result<void>& result) {
        emit operationFinished(QStringLiteral("abortRebase"));

        if (result.isFailure()) {
            emit rebaseFailed(result.errorMessage());
            emit errorOccurred(QStringLiteral("abortRebase"), result.errorMessage());
            return;
        }

        GitOperationResult opResult;
        opResult.operationType = QStringLiteral("rebase");
        opResult.success = true;
        opResult.message = QStringLiteral("变基已中止");
        emit rebaseCompleted(opResult);
    });
}
