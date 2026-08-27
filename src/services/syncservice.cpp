#include "syncservice.h"

#include "services/gitservice.h"

SyncService::SyncService(GitService* gitService)
    : BaseService(gitService)
{
}

Result<void> SyncService::fetch(const QString& repoPath) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();

    return m_gitService->fetch(repoPath);
}

Result<void> SyncService::pull(const QString& repoPath) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();

    return m_gitService->pull(repoPath);
}

Result<void> SyncService::push(const QString& repoPath) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();

    return m_gitService->push(repoPath);
}

Result<void> SyncService::push(const QString& repoPath,
                               const QString& remote,
                               const QString& branch) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();

    return m_gitService->push(repoPath, remote, branch);
}

Result<QStringList> SyncService::listRemotes(const QString& repoPath) const
{
    if (!isInitialized())
        return serviceNotInitialized<QStringList>();

    return m_gitService->listRemotes(repoPath);
}

Result<void> SyncService::addRemote(const QString& repoPath,
                                    const QString& name,
                                    const QString& url) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();

    return m_gitService->addRemote(repoPath, name, url);
}

Result<void> SyncService::removeRemote(const QString& repoPath,
                                       const QString& name) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();

    return m_gitService->removeRemote(repoPath, name);
}
