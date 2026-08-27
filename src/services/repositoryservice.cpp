#include "repositoryservice.h"

#include "services/gitservice.h"
#include "services/settingsservice.h"

#include <QDateTime>
#include <QDir>

RepositoryService::RepositoryService(GitService* gitService,
                                     SettingsService* settingsService)
    : m_gitService(gitService)
    , m_settingsService(settingsService)
    , m_currentRepository(std::nullopt)
{
}

Result<Repository> RepositoryService::openRepository(const QString& path)
{
    if (m_gitService == nullptr) {
        return Result<Repository>::failure(QStringLiteral("GitService is not initialized."));
    }

    const Result<Repository> result = m_gitService->openRepository(path);

    if (result.isFailure()) {
        return result;
    }

    Repository repository = result.value();
    m_currentRepository = repository;

    // 将仓库添加到最近使用列表
    if (m_settingsService != nullptr) {
        m_settingsService->addRecentRepository(repository);
    }

    return Result<Repository>::success(repository);
}

Result<Repository> RepositoryService::initRepository(const QString& path)
{
    if (m_gitService == nullptr) {
        return Result<Repository>::failure(QStringLiteral("GitService is not initialized."));
    }

    const Result<void> initResult = m_gitService->init(path);

    if (initResult.isFailure()) {
        return Result<Repository>::failure(initResult.errorMessage());
    }

    // init 成功后打开仓库以获取完整信息
    return openRepository(path);
}

Result<Repository> RepositoryService::cloneRepository(const QString& url,
                                                      const QString& targetPath)
{
    if (m_gitService == nullptr) {
        return Result<Repository>::failure(QStringLiteral("GitService is not initialized."));
    }

    const Result<void> cloneResult = m_gitService->clone(url, targetPath);

    if (cloneResult.isFailure()) {
        return Result<Repository>::failure(cloneResult.errorMessage());
    }

    // clone 成功后打开仓库以获取完整信息
    return openRepository(targetPath);
}

Result<void> RepositoryService::closeRepository(const QString& path)
{
    if (m_currentRepository.has_value()
        && Repository::resolveAbsolutePath(m_currentRepository->localPath())
           == Repository::resolveAbsolutePath(path)) {
        m_currentRepository.reset();
    }

    return Result<void>::success();
}

Result<Repository> RepositoryService::currentRepository() const
{
    if (!m_currentRepository.has_value()) {
        return Result<Repository>::failure(QStringLiteral("No repository is currently open."));
    }

    return Result<Repository>::success(m_currentRepository.value());
}

Result<QString> RepositoryService::currentBranch(const QString& repoPath) const
{
    if (m_gitService == nullptr) {
        return Result<QString>::failure(QStringLiteral("GitService is not initialized."));
    }

    return m_gitService->currentBranch(repoPath);
}

Result<bool> RepositoryService::isGitRepository(const QString& path) const
{
    if (m_gitService == nullptr) {
        return Result<bool>::failure(QStringLiteral("GitService is not initialized."));
    }

    return m_gitService->isGitRepository(path);
}

QStringList RepositoryService::recentRepositoryPaths() const
{
    if (m_settingsService == nullptr) {
        return QStringList();
    }

    QStringList paths;
    const QList<Repository> recent = m_settingsService->recentRepositories();

    for (const Repository& repo : recent) {
        paths.append(repo.localPath());
    }

    return paths;
}

QList<Repository> RepositoryService::recentRepositories() const
{
    if (m_settingsService == nullptr) {
        return QList<Repository>();
    }

    return m_settingsService->recentRepositories();
}

void RepositoryService::addRecentRepository(const Repository& repository)
{
    if (m_settingsService != nullptr) {
        m_settingsService->addRecentRepository(repository);
    }
}

void RepositoryService::removeRecentRepository(const QString& path)
{
    if (m_settingsService != nullptr) {
        m_settingsService->removeRecentRepository(path);
    }
}
