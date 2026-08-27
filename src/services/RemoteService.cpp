#include "RemoteService.h"

#include "services/gitservice.h"

RemoteService::RemoteService(GitService* gitService)
    : gitService_(gitService)
{
}

Result<std::vector<Remote>> RemoteService::listRemotes(const QString& repoPath) const
{
    if (gitService_ == nullptr) {
        return Result<std::vector<Remote>>::failure(QStringLiteral("GitService is not initialized."));
    }

    return gitService_->remoteDetails(repoPath);
}

Result<void> RemoteService::addRemote(const QString& repoPath,
                                      const QString& name,
                                      const QString& url) const
{
    if (gitService_ == nullptr) {
        return Result<void>::failure(QStringLiteral("GitService is not initialized."));
    }

    const QString cleanName = name.trimmed();
    if (cleanName.isEmpty()) {
        return Result<void>::failure(QStringLiteral("Remote name cannot be empty."));
    }

    const QString cleanUrl = url.trimmed();
    if (cleanUrl.isEmpty()) {
        return Result<void>::failure(QStringLiteral("Remote URL cannot be empty."));
    }

    return gitService_->addRemote(repoPath, cleanName, cleanUrl);
}

Result<void> RemoteService::removeRemote(const QString& repoPath,
                                         const QString& name) const
{
    if (gitService_ == nullptr) {
        return Result<void>::failure(QStringLiteral("GitService is not initialized."));
    }

    const QString cleanName = name.trimmed();
    if (cleanName.isEmpty()) {
        return Result<void>::failure(QStringLiteral("Remote name cannot be empty."));
    }

    return gitService_->removeRemote(repoPath, cleanName);
}

Result<void> RemoteService::renameRemote(const QString& repoPath,
                                         const QString& oldName,
                                         const QString& newName) const
{
    if (gitService_ == nullptr) {
        return Result<void>::failure(QStringLiteral("GitService is not initialized."));
    }

    const QString cleanOld = oldName.trimmed();
    if (cleanOld.isEmpty()) {
        return Result<void>::failure(QStringLiteral("Old remote name cannot be empty."));
    }

    const QString cleanNew = newName.trimmed();
    if (cleanNew.isEmpty()) {
        return Result<void>::failure(QStringLiteral("New remote name cannot be empty."));
    }

    return gitService_->renameRemote(repoPath, cleanOld, cleanNew);
}

Result<void> RemoteService::setRemoteUrl(const QString& repoPath,
                                         const QString& name,
                                         const QString& url) const
{
    if (gitService_ == nullptr) {
        return Result<void>::failure(QStringLiteral("GitService is not initialized."));
    }

    const QString cleanName = name.trimmed();
    if (cleanName.isEmpty()) {
        return Result<void>::failure(QStringLiteral("Remote name cannot be empty."));
    }

    const QString cleanUrl = url.trimmed();
    if (cleanUrl.isEmpty()) {
        return Result<void>::failure(QStringLiteral("Remote URL cannot be empty."));
    }

    return gitService_->setRemoteUrl(repoPath, cleanName, cleanUrl);
}
