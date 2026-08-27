#include "RemoteController.h"

#include "domain/Remote.h"
#include "models/RemoteListModel.h"
#include "services/RemoteService.h"

RemoteController::RemoteController(RemoteService* remoteService,
                                   RemoteListModel* remoteListModel,
                                   QObject* parent)
    : QObject(parent),
    m_remoteService(remoteService),
    m_remoteListModel(remoteListModel)
{
}

void RemoteController::loadRemotes(const QString& repoPath)
{
    if (!m_remoteService) {
        emit remotesLoaded(false, QStringLiteral("Remote service is not available."));
        return;
    }

    const Result<std::vector<Remote>> result = m_remoteService->listRemotes(repoPath);

    if (result.isFailure()) {
        emit remotesLoaded(false, result.errorMessage());
        emit errorOccurred(QStringLiteral("loadRemotes"), result.errorMessage());
        return;
    }

    if (m_remoteListModel) {
        m_remoteListModel->setRemotes(result.value());
    } else {
        emit remotesLoaded(false, QStringLiteral("Remote list model is not available."));
        return;
    }

    emit remotesLoaded(true, QString());
}

Result<void> RemoteController::addRemote(const QString& repoPath,
                                         const QString& name,
                                         const QString& url)
{
    if (!m_remoteService) {
        const QString errorMessage = QStringLiteral("Remote service is not available.");
        emit operationFinished(QStringLiteral("addRemote"), false, errorMessage);
        return Result<void>::failure(errorMessage);
    }

    const Result<void> result = m_remoteService->addRemote(repoPath, name, url);

    if (result.isFailure()) {
        emit operationFinished(QStringLiteral("addRemote"), false, result.errorMessage());
        emit errorOccurred(QStringLiteral("addRemote"), result.errorMessage());
        return result;
    }

    loadRemotes(repoPath);

    emit operationFinished(QStringLiteral("addRemote"), true, QString());
    return Result<void>::success();
}

Result<void> RemoteController::removeRemote(const QString& repoPath,
                                            const QString& name)
{
    if (!m_remoteService) {
        const QString errorMessage = QStringLiteral("Remote service is not available.");
        emit operationFinished(QStringLiteral("removeRemote"), false, errorMessage);
        return Result<void>::failure(errorMessage);
    }

    const Result<void> result = m_remoteService->removeRemote(repoPath, name);

    if (result.isFailure()) {
        emit operationFinished(QStringLiteral("removeRemote"), false, result.errorMessage());
        emit errorOccurred(QStringLiteral("removeRemote"), result.errorMessage());
        return result;
    }

    loadRemotes(repoPath);

    emit operationFinished(QStringLiteral("removeRemote"), true, QString());
    return Result<void>::success();
}

Result<void> RemoteController::renameRemote(const QString& repoPath,
                                            const QString& oldName,
                                            const QString& newName)
{
    if (!m_remoteService) {
        const QString errorMessage = QStringLiteral("Remote service is not available.");
        emit operationFinished(QStringLiteral("renameRemote"), false, errorMessage);
        return Result<void>::failure(errorMessage);
    }

    const Result<void> result = m_remoteService->renameRemote(repoPath, oldName, newName);

    if (result.isFailure()) {
        emit operationFinished(QStringLiteral("renameRemote"), false, result.errorMessage());
        emit errorOccurred(QStringLiteral("renameRemote"), result.errorMessage());
        return result;
    }

    loadRemotes(repoPath);

    emit operationFinished(QStringLiteral("renameRemote"), true, QString());
    return Result<void>::success();
}

Result<void> RemoteController::setRemoteUrl(const QString& repoPath,
                                            const QString& name,
                                            const QString& url)
{
    if (!m_remoteService) {
        const QString errorMessage = QStringLiteral("Remote service is not available.");
        emit operationFinished(QStringLiteral("setRemoteUrl"), false, errorMessage);
        return Result<void>::failure(errorMessage);
    }

    const Result<void> result = m_remoteService->setRemoteUrl(repoPath, name, url);

    if (result.isFailure()) {
        emit operationFinished(QStringLiteral("setRemoteUrl"), false, result.errorMessage());
        emit errorOccurred(QStringLiteral("setRemoteUrl"), result.errorMessage());
        return result;
    }

    loadRemotes(repoPath);

    emit operationFinished(QStringLiteral("setRemoteUrl"), true, QString());
    return Result<void>::success();
}

RemoteListModel* RemoteController::remoteListModel() const
{
    return m_remoteListModel;
}

void RemoteController::clear()
{
    if (m_remoteListModel) {
        m_remoteListModel->clear();
    }
}
