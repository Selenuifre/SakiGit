#ifndef REBASESERVICE_H
#define REBASESERVICE_H

#include "BaseService.h"
#include "domain/GitOperationResult.h"
#include "infrastructure/result.h"

#include <QString>

class GitService;

class RebaseService : public BaseService
{
public:
    explicit RebaseService(GitService* gitService);

    Result<GitOperationResult> rebase(const QString& repoPath,
                                      const QString& branchName) const;
    Result<void> continueRebase(const QString& repoPath) const;
    Result<void> abortRebase(const QString& repoPath) const;
    Result<bool> isWorkingTreeClean(const QString& repoPath) const;
    Result<bool> isRebasing(const QString& repoPath) const;
};

#endif // REBASESERVICE_H
