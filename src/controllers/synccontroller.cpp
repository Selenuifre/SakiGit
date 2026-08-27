#include "synccontroller.h"

#include "services/gittaskrunner.h"
#include "services/syncservice.h"

SyncController::SyncController(SyncService* syncService, QObject* parent)
    : BaseController(parent),
    m_syncService(syncService)
{
}

void SyncController::setGitTaskRunner(GitTaskRunner* taskRunner)
{
    if (m_taskRunner) {
        QObject::disconnect(m_taskRunner, &GitTaskRunner::fetchFinished,
                            this, &SyncController::onFetchFinished);
        QObject::disconnect(m_taskRunner, &GitTaskRunner::pullFinished,
                            this, &SyncController::onPullFinished);
        QObject::disconnect(m_taskRunner, &GitTaskRunner::pushFinished,
                            this, &SyncController::onPushFinished);
    }

    m_taskRunner = taskRunner;

    if (m_taskRunner) {
        QObject::connect(m_taskRunner, &GitTaskRunner::fetchFinished,
                         this, &SyncController::onFetchFinished);
        QObject::connect(m_taskRunner, &GitTaskRunner::pullFinished,
                         this, &SyncController::onPullFinished);
        QObject::connect(m_taskRunner, &GitTaskRunner::pushFinished,
                         this, &SyncController::onPushFinished);
    }
}

void SyncController::fetch(const QString& repoPath, const QString& remoteName)
{
    if (m_taskRunner) {
        m_taskRunner->fetch(repoPath, remoteName);
        return;
    }

    // 回退：同步执行
    if (!m_syncService) {
        emit fetchFinished(false, QStringLiteral("Sync service is not available."));
        return;
    }

    const Result<void> result = m_syncService->fetch(repoPath);

    if (result.isFailure()) {
        emit fetchFinished(false, result.errorMessage());
        emit errorOccurred(QStringLiteral("fetch"), result.errorMessage());
        return;
    }

    emit fetchFinished(true, QString());
}

void SyncController::pull(const QString& repoPath)
{
    if (m_taskRunner) {
        m_taskRunner->pull(repoPath);
        return;
    }

    // 回退：同步执行
    if (!m_syncService) {
        emit pullFinished(false, QStringLiteral("Sync service is not available."));
        return;
    }

    const Result<void> result = m_syncService->pull(repoPath);

    if (result.isFailure()) {
        emit pullFinished(false, result.errorMessage());
        emit errorOccurred(QStringLiteral("pull"), result.errorMessage());
        return;
    }

    emit pullFinished(true, QString());
}

void SyncController::push(const QString& repoPath)
{
    push(repoPath, QString(), QString());
}

void SyncController::push(const QString& repoPath, const QString& remote, const QString& branch)
{
    if (m_taskRunner) {
        m_taskRunner->push(repoPath, remote, branch);
        return;
    }

    // 回退：同步执行
    if (!m_syncService) {
        emit pushFinished(false, QStringLiteral("Sync service is not available."));
        return;
    }

    const Result<void> result = m_syncService->push(repoPath, remote, branch);

    if (result.isFailure()) {
        emit pushFinished(false, result.errorMessage());
        emit errorOccurred(QStringLiteral("push"), result.errorMessage());
        return;
    }

    emit pushFinished(true, QString());
}

Result<QStringList> SyncController::listRemotes(const QString& repoPath) const
{
    if (!m_syncService) {
        return Result<QStringList>::failure(QStringLiteral("Sync service is not available."));
    }

    return m_syncService->listRemotes(repoPath);
}

Result<void> SyncController::addRemote(const QString& repoPath, const QString& name, const QString& url)
{
    if (!m_syncService) {
        const QString errorMessage = QStringLiteral("Sync service is not available.");
        emit remoteOperationFinished(QStringLiteral("addRemote"), false, errorMessage);
        return Result<void>::failure(errorMessage);
    }

    const Result<void> result = m_syncService->addRemote(repoPath, name, url);

    if (result.isFailure()) {
        emit remoteOperationFinished(QStringLiteral("addRemote"), false, result.errorMessage());
        emit errorOccurred(QStringLiteral("addRemote"), result.errorMessage());
        return result;
    }

    emit remoteOperationFinished(QStringLiteral("addRemote"), true, QString());
    return Result<void>::success();
}

Result<void> SyncController::removeRemote(const QString& repoPath, const QString& name)
{
    if (!m_syncService) {
        const QString errorMessage = QStringLiteral("Sync service is not available.");
        emit remoteOperationFinished(QStringLiteral("removeRemote"), false, errorMessage);
        return Result<void>::failure(errorMessage);
    }

    const Result<void> result = m_syncService->removeRemote(repoPath, name);

    if (result.isFailure()) {
        emit remoteOperationFinished(QStringLiteral("removeRemote"), false, result.errorMessage());
        emit errorOccurred(QStringLiteral("removeRemote"), result.errorMessage());
        return result;
    }

    emit remoteOperationFinished(QStringLiteral("removeRemote"), true, QString());
    return Result<void>::success();
}

void SyncController::onFetchFinished(bool success, const QString& repoPath, const QString& remoteName, const QString& errorMessage)
{
    Q_UNUSED(repoPath);
    Q_UNUSED(remoteName);
    if (!success) {
        emit errorOccurred(QStringLiteral("fetch"), errorMessage);
    }
    emit fetchFinished(success, errorMessage);
}

void SyncController::onPullFinished(bool success, const QString& repoPath, const QString& errorMessage)
{
    Q_UNUSED(repoPath);
    if (!success) {
        emit errorOccurred(QStringLiteral("pull"), errorMessage);
    }
    emit pullFinished(success, errorMessage);
}

void SyncController::onPushFinished(bool success, const QString& repoPath, const QString& remoteName, const QString& branchName, const QString& errorMessage)
{
    Q_UNUSED(repoPath);
    Q_UNUSED(remoteName);
    Q_UNUSED(branchName);
    if (!success) {
        emit errorOccurred(QStringLiteral("push"), errorMessage);
    }
    emit pushFinished(success, errorMessage);
}
