#include "MergeController.h"

#include "services/MergeService.h"
#include "services/GitTaskRunner.h"

#include <QtConcurrent/QtConcurrentRun>

MergeController::MergeController(MergeService* mergeService,
                                   GitTaskRunner* taskRunner,
                                   QObject* parent)
    : GitOperationController(parent),
      m_mergeService(mergeService),
      m_taskRunner(taskRunner)
{
}

void MergeController::checkPreconditions(const QString& repoPath)
{
    if (!m_mergeService) {
        emit preconditionsChecked(false,
            QStringLiteral("Merge service is not available."));
        return;
    }

    // 检查工作区是否干净
    const Result<bool> cleanResult = m_mergeService->isWorkingTreeClean(repoPath);
    if (cleanResult.isFailure()) {
        emit preconditionsChecked(false,
            QStringLiteral("无法检查工作区状态: ") + cleanResult.errorMessage());
        return;
    }

    if (!cleanResult.value()) {
        emit preconditionsChecked(true,
            QStringLiteral("工作区有未提交的修改，建议先提交或暂存后再合并。"));
        return;
    }

    // 检查是否已在合并状态
    const Result<bool> mergingResult = m_mergeService->isMerging(repoPath);
    if (mergingResult.isSuccess() && mergingResult.value()) {
        emit preconditionsChecked(false,
            QStringLiteral("仓库当前处于合并冲突状态，请先解决冲突或中止合并。"));
        return;
    }

    emit preconditionsChecked(true, QString());
}

void MergeController::performMerge(const QString& repoPath,
                                     const QString& branchName)
{
    if (!m_mergeService) {
        emit mergeFailed(QStringLiteral("Merge service is not available."));
        emit errorOccurred(QStringLiteral("merge"), QStringLiteral("Merge service is not available."));
        return;
    }

    // 异步执行：使用 QtConcurrent::run 将合并操作放入后台线程
    emit operationStarted(QStringLiteral("merge"));

    // 捕获 repoPath 和 branchName 副本用于后台线程
    MergeService* service = m_mergeService;
    const QString repo = repoPath;
    const QString branch = branchName;

    QtConcurrent::run([service, repo, branch]() {
        return service->merge(repo, branch);
    }).then([this, repo, branch](const Result<GitOperationResult>& result) {
        emit operationFinished(QStringLiteral("merge"));

        if (result.isFailure()) {
            emit mergeFailed(result.errorMessage());
            emit errorOccurred(QStringLiteral("merge"), result.errorMessage());
            return;
        }

        const GitOperationResult& opResult = result.value();
        if (opResult.hasConflicts) {
            emit mergeConflict(opResult);
        } else if (opResult.success) {
            emit mergeCompleted(opResult);
        } else {
            emit mergeFailed(opResult.message);
        }
    });
}

void MergeController::abortMerge(const QString& repoPath)
{
    if (!m_mergeService) {
        emit mergeFailed(QStringLiteral("Merge service is not available."));
        emit errorOccurred(QStringLiteral("abortMerge"), QStringLiteral("Merge service is not available."));
        return;
    }

    emit operationStarted(QStringLiteral("abortMerge"));

    MergeService* service = m_mergeService;
    const QString repo = repoPath;

    QtConcurrent::run([service, repo]() {
        return service->abort(repo);
    }).then([this](const Result<void>& result) {
        emit operationFinished(QStringLiteral("abortMerge"));

        if (result.isFailure()) {
            emit mergeFailed(result.errorMessage());
            emit errorOccurred(QStringLiteral("abortMerge"), result.errorMessage());
            return;
        }

        GitOperationResult opResult;
        opResult.operationType = QStringLiteral("merge");
        opResult.success = true;
        opResult.message = QStringLiteral("合并已中止");
        emit mergeCompleted(opResult);
    });
}
