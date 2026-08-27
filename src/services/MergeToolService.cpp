#include "MergeToolService.h"

#include "services/gitservice.h"

MergeToolService::MergeToolService(GitService* gitService)
    : BaseService(gitService)
{
}

Result<MergeFileContent> MergeToolService::loadThreeWayContent(
    const QString& repoPath, const QString& filePath) const
{
    if (!isInitialized())
        return serviceNotInitialized<MergeFileContent>();

    MergeFileContent content;
    content.filePath = filePath;

    const Result<QString> baseResult = m_gitService->readBaseFile(repoPath, filePath);
    if (baseResult.isSuccess()) content.baseContent = baseResult.value();

    const Result<QString> oursResult = m_gitService->readOursFile(repoPath, filePath);
    if (oursResult.isSuccess()) content.oursContent = oursResult.value();

    const Result<QString> theirsResult = m_gitService->readTheirsFile(repoPath, filePath);
    if (theirsResult.isSuccess()) content.theirsContent = theirsResult.value();

    return Result<MergeFileContent>::success(content);
}

Result<void> MergeToolService::saveResolvedContent(
    const QString& repoPath, const QString& filePath, const QString& content) const
{
    if (!isInitialized())
        return serviceNotInitialized<void>();
    return m_gitService->writeResolvedFile(repoPath, filePath, content);
}
