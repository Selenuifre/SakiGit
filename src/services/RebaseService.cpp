#include "RebaseService.h"

#include "services/gitservice.h"

RebaseService::RebaseService(GitService* gitService)
    : BaseService(gitService)
{
}

Result<GitOperationResult> RebaseService::rebase(const QString& repoPath,
                                                  const QString& branchName) const
{
    if (!isInitialized())
        return serviceNotInitialized<GitOperationResult>();

    const QString cleanName = branchName.trimmed();
    if (cleanName.isEmpty()) {
        return Result<GitOperationResult>::failure(
            QStringLiteral("Branch name cannot be empty."));
    }

    // 前置检查：仓库是否已在变基状态中
    const Result<bool> rebasingCheck = m_gitService->isRebasing(repoPath);
    if (rebasingCheck.isSuccess() && rebasingCheck.value()) {
        return Result<GitOperationResult>::failure(
            QStringLiteral("仓库当前处于变基状态，请先继续或中止变基。"));
    }

    // 执行变基
    return m_gitService->rebaseBranch(repoPath, cleanName);
}

Result<void> RebaseService::continueRebase(const QString& repoPath) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();

    return m_gitService->continueRebase(repoPath);
}

Result<void> RebaseService::abortRebase(const QString& repoPath) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();

    return m_gitService->abortRebase(repoPath);
}

Result<bool> RebaseService::isWorkingTreeClean(const QString& repoPath) const
{
    if (!isInitialized())
        return serviceNotInitialized<bool>();

    return m_gitService->isWorkingTreeClean(repoPath);
}

Result<bool> RebaseService::isRebasing(const QString& repoPath) const
{
    if (!isInitialized())
        return serviceNotInitialized<bool>();

    return m_gitService->isRebasing(repoPath);
}
