#include "branchservice.h"

#include "services/gitservice.h"

BranchService::BranchService(GitService* gitService)
    : BaseService(gitService)
{
}

Result<QList<Branch>> BranchService::listBranches(const QString& repoPath) const
{
    if (!isInitialized())
        return serviceNotInitialized<QList<Branch>>();

    return m_gitService->branches(repoPath);
}

Result<QString> BranchService::currentBranch(const QString& repoPath) const
{
    if (!isInitialized())
        return serviceNotInitialized<QString>();

    return m_gitService->currentBranch(repoPath);
}

Result<void> BranchService::createBranch(const QString& repoPath,
                                         const QString& branchName) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();

    return m_gitService->createBranch(repoPath, branchName);
}

Result<void> BranchService::checkoutBranch(const QString& repoPath,
                                           const QString& branchName) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();

    return m_gitService->checkoutBranch(repoPath, branchName);
}

Result<void> BranchService::deleteBranch(const QString& repoPath,
                                         const QString& branchName,
                                         bool force) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();

    return m_gitService->deleteBranch(repoPath, branchName, force);
}

Result<void> BranchService::mergeBranch(const QString& repoPath,
                                        const QString& branchName) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();

    return m_gitService->merge(repoPath, branchName);
}

Result<void> BranchService::renameBranch(const QString& repoPath,
                                         const QString& oldName,
                                         const QString& newName) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();

    return m_gitService->renameBranch(repoPath, oldName, newName);
}
