#include "StashService.h"

#include "services/gitservice.h"

StashService::StashService(GitService* gitService)
    : BaseService(gitService)
{
}

Result<std::vector<Stash>> StashService::list(const QString& repoPath) const
{
    if (!isInitialized())
        return serviceNotInitialized<std::vector<Stash>>();

    if (repoPath.trimmed().isEmpty()) {
        return Result<std::vector<Stash>>::failure(QStringLiteral("Repository path cannot be empty."));
    }

    return m_gitService->listStashes(repoPath);
}

Result<void> StashService::save(const QString& repoPath, const QString& message) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();

    if (repoPath.trimmed().isEmpty()) {
        return Result<void>::failure(QStringLiteral("Repository path cannot be empty."));
    }

    return m_gitService->createStash(repoPath, message);
}

Result<void> StashService::apply(const QString& repoPath, int index) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();

    if (repoPath.trimmed().isEmpty()) {
        return Result<void>::failure(QStringLiteral("Repository path cannot be empty."));
    }

    if (index < 0) {
        return Result<void>::failure(QStringLiteral("Stash index cannot be negative."));
    }

    return m_gitService->applyStash(repoPath, index);
}

Result<void> StashService::drop(const QString& repoPath, int index) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();

    if (repoPath.trimmed().isEmpty()) {
        return Result<void>::failure(QStringLiteral("Repository path cannot be empty."));
    }

    if (index < 0) {
        return Result<void>::failure(QStringLiteral("Stash index cannot be negative."));
    }

    return m_gitService->dropStash(repoPath, index);
}

Result<Diff> StashService::diff(const QString& repoPath, int index) const
{
    if (!isInitialized())
        return serviceNotInitialized<Diff>();

    if (repoPath.trimmed().isEmpty()) {
        return Result<Diff>::failure(QStringLiteral("Repository path cannot be empty."));
    }

    if (index < 0) {
        return Result<Diff>::failure(QStringLiteral("Stash index cannot be negative."));
    }

    return m_gitService->showStashDiff(repoPath, index);
}
