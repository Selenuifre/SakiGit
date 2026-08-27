#ifndef REMOTESERVICE_H
#define REMOTESERVICE_H

#include "domain/Remote.h"
#include "infrastructure/result.h"

#include <QString>
#include <vector>

class GitService;

class RemoteService
{
public:
    explicit RemoteService(GitService* gitService);

    Result<std::vector<Remote>> listRemotes(const QString& repoPath) const;

    Result<void> addRemote(const QString& repoPath,
                           const QString& name,
                           const QString& url) const;

    Result<void> removeRemote(const QString& repoPath,
                              const QString& name) const;

    Result<void> renameRemote(const QString& repoPath,
                              const QString& oldName,
                              const QString& newName) const;

    Result<void> setRemoteUrl(const QString& repoPath,
                              const QString& name,
                              const QString& url) const;

private:
    GitService* gitService_;
};

#endif // REMOTESERVICE_H
