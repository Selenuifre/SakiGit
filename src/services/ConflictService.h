#ifndef CONFLICTSERVICE_H
#define CONFLICTSERVICE_H

#include "BaseService.h"
#include "domain/ConflictFile.h"
#include "infrastructure/result.h"

#include <QString>
#include <vector>

class GitService;

class ConflictService : public BaseService
{
public:
    explicit ConflictService(GitService* gitService);

    Result<bool> hasConflicts(const QString& repoPath) const;

    Result<std::vector<ConflictFile>> listFiles(const QString& repoPath) const;

    Result<void> acceptOurs(const QString& repoPath,
                            const QString& filePath) const;

    Result<void> acceptTheirs(const QString& repoPath,
                              const QString& filePath) const;

    Result<void> markResolved(const QString& repoPath,
                              const QString& filePath) const;
};

#endif // CONFLICTSERVICE_H
