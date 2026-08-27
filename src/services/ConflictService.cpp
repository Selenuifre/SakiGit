#include "ConflictService.h"

#include "services/gitservice.h"

ConflictService::ConflictService(GitService* gitService)
    : BaseService(gitService)
{
}

Result<bool> ConflictService::hasConflicts(const QString& repoPath) const
{
    if (!isInitialized())
        return serviceNotInitialized<bool>();
    return m_gitService->hasConflicts(repoPath);
}

Result<std::vector<ConflictFile>> ConflictService::listFiles(
    const QString& repoPath) const
{
    if (!isInitialized())
        return serviceNotInitialized<std::vector<ConflictFile>>();
    return m_gitService->listConflictFiles(repoPath);
}

Result<void> ConflictService::acceptOurs(const QString& repoPath,
                                          const QString& filePath) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();
    return m_gitService->checkoutOurs(repoPath, filePath);
}

Result<void> ConflictService::acceptTheirs(const QString& repoPath,
                                            const QString& filePath) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();
    return m_gitService->checkoutTheirs(repoPath, filePath);
}

Result<void> ConflictService::markResolved(const QString& repoPath,
                                            const QString& filePath) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();
    return m_gitService->markResolved(repoPath, filePath);
}
