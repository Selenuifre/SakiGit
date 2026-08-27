#ifndef STASHSERVICE_H
#define STASHSERVICE_H

#include "BaseService.h"
#include "domain/Stash.h"
#include "domain/diff.h"
#include "infrastructure/result.h"

#include <QString>
#include <vector>

class GitService;

class StashService : public BaseService
{
public:
    explicit StashService(GitService* gitService);

    Result<std::vector<Stash>> list(const QString& repoPath) const;
    Result<void> save(const QString& repoPath, const QString& message) const;
    Result<void> apply(const QString& repoPath, int index) const;
    Result<void> drop(const QString& repoPath, int index) const;
    Result<Diff> diff(const QString& repoPath, int index) const;
};

#endif // STASHSERVICE_H
