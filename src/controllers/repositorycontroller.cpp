#include "repositorycontroller.h"

#include "models/repositorylistmodel.h"
#include "services/gittaskrunner.h"
#include "services/repositoryservice.h"

RepositoryController::RepositoryController(RepositoryService* repositoryService,
                                           RepositoryListModel* repositoryListModel,
                                           QObject* parent)
    : BaseController(parent),
    m_repositoryService(repositoryService),
    m_repositoryListModel(repositoryListModel)
{
}

void RepositoryController::setGitTaskRunner(GitTaskRunner* taskRunner)
{
    if (m_taskRunner) {
        QObject::disconnect(m_taskRunner, &GitTaskRunner::repositoryOpened,
                            this, &RepositoryController::onRepositoryOpened);
        QObject::disconnect(m_taskRunner, &GitTaskRunner::repositoryInitialized,
                            this, &RepositoryController::onRepositoryInitialized);
        QObject::disconnect(m_taskRunner, &GitTaskRunner::repositoryCloned,
                            this, &RepositoryController::onRepositoryCloned);
    }

    m_taskRunner = taskRunner;

    if (m_taskRunner) {
        QObject::connect(m_taskRunner, &GitTaskRunner::repositoryOpened,
                         this, &RepositoryController::onRepositoryOpened);
        QObject::connect(m_taskRunner, &GitTaskRunner::repositoryInitialized,
                         this, &RepositoryController::onRepositoryInitialized);
        QObject::connect(m_taskRunner, &GitTaskRunner::repositoryCloned,
                         this, &RepositoryController::onRepositoryCloned);
    }
}

void RepositoryController::openRepository(const QString& path)
{
    // 优先使用异步执行器，避免 UI 阻塞
    if (m_taskRunner) {
        m_taskRunner->openRepository(path);
        return;
    }

    // 回退：同步执行（无 GitTaskRunner 时）
    if (!m_repositoryService) {
        emit openFailed(path, QStringLiteral("Repository service is not available."));
        return;
    }

    const Result<Repository> result = m_repositoryService->openRepository(path);

    if (result.isFailure()) {
        emit openFailed(path, result.errorMessage());
        emit errorOccurred(QStringLiteral("openRepository"), result.errorMessage());
        return;
    }

    const Repository& repository = result.value();

    if (m_repositoryListModel) {
        m_repositoryListModel->upsertRepository(repository);
    }

    m_repositoryService->addRecentRepository(repository);

    emit repositoryOpened(repository);
}

void RepositoryController::onRepositoryOpened(bool success,
                                               const QString& repositoryPath,
                                               const Repository& repository,
                                               const QString& errorMessage)
{
    if (!success) {
        emit openFailed(repositoryPath, errorMessage);
        emit errorOccurred(QStringLiteral("openRepository"), errorMessage);
        return;
    }

    if (m_repositoryListModel) {
        m_repositoryListModel->upsertRepository(repository);
    }

    if (m_repositoryService) {
        m_repositoryService->addRecentRepository(repository);
    }

    emit repositoryOpened(repository);
}

void RepositoryController::onRepositoryInitialized(bool success,
                                                     const QString& path,
                                                     const Repository& repository,
                                                     const QString& errorMessage)
{
    if (!success) {
        emit errorOccurred(QStringLiteral("initRepository"), errorMessage);
        return;
    }

    if (m_repositoryListModel) {
        m_repositoryListModel->upsertRepository(repository);
    }

    if (m_repositoryService) {
        m_repositoryService->addRecentRepository(repository);
    }

    emit repositoryInitialized(repository);
}

void RepositoryController::onRepositoryCloned(bool success,
                                                const QString& url,
                                                const Repository& repository,
                                                const QString& errorMessage)
{
    if (!success) {
        emit cloneFailed(url, errorMessage);
        emit errorOccurred(QStringLiteral("cloneRepository"), errorMessage);
        return;
    }

    if (m_repositoryListModel) {
        m_repositoryListModel->addRepository(repository);
    }

    if (m_repositoryService) {
        m_repositoryService->addRecentRepository(repository);
    }

    emit repositoryCloned(repository);
}

void RepositoryController::initRepository(const QString& path)
{
    if (m_taskRunner) {
        m_taskRunner->initRepository(path);
        return;
    }

    // 回退：同步执行
    if (!m_repositoryService) {
        emit errorOccurred(QStringLiteral("initRepository"),
                           QStringLiteral("Repository service is not available."));
        return;
    }

    const Result<Repository> result = m_repositoryService->initRepository(path);

    if (result.isFailure()) {
        emit errorOccurred(QStringLiteral("initRepository"), result.errorMessage());
        return;
    }

    const Repository& repository = result.value();

    if (m_repositoryListModel) {
        m_repositoryListModel->upsertRepository(repository);
    }

    emit repositoryInitialized(repository);
}

void RepositoryController::cloneRepository(const QString& url, const QString& targetPath)
{
    if (m_taskRunner) {
        m_taskRunner->cloneRepository(url, targetPath);
        return;
    }

    // 回退：同步执行
    if (!m_repositoryService) {
        emit cloneFailed(url, QStringLiteral("Repository service is not available."));
        return;
    }

    const Result<Repository> result = m_repositoryService->cloneRepository(url, targetPath);

    if (result.isFailure()) {
        emit cloneFailed(url, result.errorMessage());
        emit errorOccurred(QStringLiteral("cloneRepository"), result.errorMessage());
        return;
    }

    const Repository& repository = result.value();

    if (m_repositoryListModel) {
        m_repositoryListModel->addRepository(repository);
    }

    emit repositoryCloned(repository);
}

Result<void> RepositoryController::closeRepository(const QString& path)
{
    if (!m_repositoryService) {
        return Result<void>::failure(QStringLiteral("Repository service is not available."));
    }

    return m_repositoryService->closeRepository(path);
}

void RepositoryController::loadRecentRepositories()
{
    if (!m_repositoryService || !m_repositoryListModel) {
        return;
    }

    const QList<Repository> repositories = m_repositoryService->recentRepositories();
    m_repositoryListModel->setRepositories(repositories);

    emit recentRepositoriesLoaded();
}

void RepositoryController::addRecentRepository(const Repository& repository)
{
    if (m_repositoryService) {
        m_repositoryService->addRecentRepository(repository);
    }

    if (m_repositoryListModel) {
        m_repositoryListModel->upsertRepository(repository);
    }
}

void RepositoryController::removeRecentRepository(const QString& path)
{
    if (m_repositoryService) {
        m_repositoryService->removeRecentRepository(path);
    }

    if (m_repositoryListModel) {
        m_repositoryListModel->removeRepository(path);
    }
}

void RepositoryController::clearRecentRepositories()
{
    if (m_repositoryListModel) {
        m_repositoryListModel->clear();
    }
}

Result<bool> RepositoryController::isGitRepository(const QString& path) const
{
    if (!m_repositoryService) {
        return Result<bool>::failure(QStringLiteral("Repository service is not available."));
    }

    return m_repositoryService->isGitRepository(path);
}

RepositoryListModel* RepositoryController::repositoryListModel() const
{
    return m_repositoryListModel;
}
