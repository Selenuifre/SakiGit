#ifndef MERGETOOLSERVICE_H
#define MERGETOOLSERVICE_H

#include "BaseService.h"
#include "domain/MergeFileContent.h"
#include "infrastructure/result.h"

#include <QString>

class GitService;

class MergeToolService : public BaseService
{
public:
    explicit MergeToolService(GitService* gitService);

    Result<MergeFileContent> loadThreeWayContent(const QString& repoPath,
                                                  const QString& filePath) const;
    Result<void> saveResolvedContent(const QString& repoPath,
                                      const QString& filePath,
                                      const QString& content) const;
};

#endif // MERGETOOLSERVICE_H
