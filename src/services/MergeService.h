#ifndef MERGESERVICE_H
#define MERGESERVICE_H

#include "BaseService.h"
#include "domain/GitOperationResult.h"
#include "infrastructure/result.h"

#include <QString>

class GitService;

class MergeService : public BaseService
{
public:
    explicit MergeService(GitService* gitService);

    // 执行合并操作：将 branchName 合并到当前分支
    Result<GitOperationResult> merge(const QString& repoPath,
                                     const QString& branchName) const;
    Result<void> abort(const QString& repoPath) const;
    Result<bool> isWorkingTreeClean(const QString& repoPath) const;
    Result<bool> isMerging(const QString& repoPath) const;
};

#endif // MERGESERVICE_H
