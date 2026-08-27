#include "MergeService.h"

#include "services/gitservice.h"

MergeService::MergeService(GitService* gitService)
    : BaseService(gitService)
{
}

Result<GitOperationResult> MergeService::merge(const QString& repoPath,
                                                const QString& branchName) const
{
    if (!isInitialized())
        return serviceNotInitialized<GitOperationResult>();

    const QString cleanName = branchName.trimmed();
    if (cleanName.isEmpty()) {
        return Result<GitOperationResult>::failure(
            QStringLiteral("Branch name cannot be empty."));
    }

    // 前置检查：仓库是否已在合并状态中
    const Result<bool> mergingCheck = m_gitService->isMerging(repoPath);
    if (mergingCheck.isSuccess() && mergingCheck.value()) {
        return Result<GitOperationResult>::failure(
            QStringLiteral("仓库当前处于合并状态，请先解决冲突或中止合并。"));
    }

    // 执行合并
    return m_gitService->mergeBranch(repoPath, cleanName);
}

Result<void> MergeService::abort(const QString& repoPath) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();

    return m_gitService->abortMerge(repoPath);
}

Result<bool> MergeService::isWorkingTreeClean(const QString& repoPath) const
{
    if (!isInitialized())
        return serviceNotInitialized<bool>();

    return m_gitService->isWorkingTreeClean(repoPath);
}

Result<bool> MergeService::isMerging(const QString& repoPath) const
{
    if (!isInitialized())
        return serviceNotInitialized<bool>();

    return m_gitService->isMerging(repoPath);
}
