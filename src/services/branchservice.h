#ifndef BRANCHSERVICE_H
#define BRANCHSERVICE_H

#include "BaseService.h"
#include "domain/branch.h"
#include "infrastructure/result.h"

#include <QList>
#include <QString>

class GitService;

class BranchService : public BaseService
{
public:
    explicit BranchService(GitService* gitService);

    Result<QList<Branch>> listBranches(const QString& repoPath) const;
    Result<QString> currentBranch(const QString& repoPath) const;
    Result<void> createBranch(const QString& repoPath,
                              const QString& branchName) const;
    Result<void> checkoutBranch(const QString& repoPath,
                                const QString& branchName) const;
    Result<void> deleteBranch(const QString& repoPath,
                              const QString& branchName,
                              bool force = false) const;
    Result<void> mergeBranch(const QString& repoPath,
                             const QString& branchName) const;
    Result<void> renameBranch(const QString& repoPath,
                              const QString& oldName,
                              const QString& newName) const;
};

#endif // BRANCHSERVICE_H
