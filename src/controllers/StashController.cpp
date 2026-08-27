#include "StashController.h"

#include "domain/Stash.h"
#include "domain/diff.h"
#include "models/difflinemodel.h"
#include "models/StashListModel.h"
#include "services/gittaskrunner.h"
#include "services/StashService.h"

StashController::StashController(StashService* stashService,
                                 StashListModel* stashListModel,
                                 DiffLineModel* diffLineModel,
                                 QObject* parent)
    : BaseController(parent),
    m_stashService(stashService),
    m_stashListModel(stashListModel),
    m_diffLineModel(diffLineModel)
{
}

void StashController::setGitTaskRunner(GitTaskRunner* taskRunner)
{
    if (m_taskRunner) {
        QObject::disconnect(m_taskRunner, &GitTaskRunner::stashCreated,
                            this, &StashController::onStashCreated);
        QObject::disconnect(m_taskRunner, &GitTaskRunner::stashApplied,
                            this, &StashController::onStashApplied);
        QObject::disconnect(m_taskRunner, &GitTaskRunner::stashDropped,
                            this, &StashController::onStashDropped);
        QObject::disconnect(m_taskRunner, &GitTaskRunner::stashDiffLoaded,
                            this, &StashController::onStashDiffLoaded);
    }

    m_taskRunner = taskRunner;

    if (m_taskRunner) {
        QObject::connect(m_taskRunner, &GitTaskRunner::stashCreated,
                         this, &StashController::onStashCreated);
        QObject::connect(m_taskRunner, &GitTaskRunner::stashApplied,
                         this, &StashController::onStashApplied);
        QObject::connect(m_taskRunner, &GitTaskRunner::stashDropped,
                         this, &StashController::onStashDropped);
        QObject::connect(m_taskRunner, &GitTaskRunner::stashDiffLoaded,
                         this, &StashController::onStashDiffLoaded);
    }
}

void StashController::loadStashes(const QString& repoPath)
{
    if (!m_stashService) {
        emit stashesLoaded(false, QStringLiteral("Stash service is not available."));
        return;
    }

    const Result<std::vector<Stash>> result = m_stashService->list(repoPath);

    if (result.isFailure()) {
        emit stashesLoaded(false, result.errorMessage());
        emit errorOccurred(QStringLiteral("loadStashes"), result.errorMessage());
        return;
    }

    if (m_stashListModel) {
        m_stashListModel->setStashes(result.value());
    } else {
        emit stashesLoaded(false, QStringLiteral("Stash list model is not available."));
        return;
    }

    emit stashesLoaded(true, QString());
}

Result<void> StashController::saveStash(const QString& repoPath, const QString& message)
{
    if (m_taskRunner) {
        m_taskRunner->createStash(repoPath, message);
        return Result<void>::success();
    }

    // 回退：同步执行
    if (!m_stashService) {
        const QString errorMessage = QStringLiteral("Stash service is not available.");
        emit operationFinished(QStringLiteral("saveStash"), false, errorMessage);
        return Result<void>::failure(errorMessage);
    }

    const Result<void> result = m_stashService->save(repoPath, message);

    if (result.isFailure()) {
        emit operationFinished(QStringLiteral("saveStash"), false, result.errorMessage());
        emit errorOccurred(QStringLiteral("saveStash"), result.errorMessage());
        return result;
    }

    loadStashes(repoPath);

    emit operationFinished(QStringLiteral("saveStash"), true, QString());
    return Result<void>::success();
}

Result<void> StashController::applyStash(const QString& repoPath, int index)
{
    if (m_taskRunner) {
        m_taskRunner->applyStash(repoPath, index);
        return Result<void>::success();
    }

    // 回退：同步执行
    if (!m_stashService) {
        const QString errorMessage = QStringLiteral("Stash service is not available.");
        emit operationFinished(QStringLiteral("applyStash"), false, errorMessage);
        return Result<void>::failure(errorMessage);
    }

    const Result<void> result = m_stashService->apply(repoPath, index);

    if (result.isFailure()) {
        emit operationFinished(QStringLiteral("applyStash"), false, result.errorMessage());
        emit errorOccurred(QStringLiteral("applyStash"), result.errorMessage());
        return result;
    }

    loadStashes(repoPath);

    emit operationFinished(QStringLiteral("applyStash"), true, QString());
    return Result<void>::success();
}

Result<void> StashController::dropStash(const QString& repoPath, int index)
{
    if (m_taskRunner) {
        m_taskRunner->dropStash(repoPath, index);
        return Result<void>::success();
    }

    // 回退：同步执行
    if (!m_stashService) {
        const QString errorMessage = QStringLiteral("Stash service is not available.");
        emit operationFinished(QStringLiteral("dropStash"), false, errorMessage);
        return Result<void>::failure(errorMessage);
    }

    const Result<void> result = m_stashService->drop(repoPath, index);

    if (result.isFailure()) {
        emit operationFinished(QStringLiteral("dropStash"), false, result.errorMessage());
        emit errorOccurred(QStringLiteral("dropStash"), result.errorMessage());
        return result;
    }

    loadStashes(repoPath);

    if (m_diffLineModel) {
        m_diffLineModel->clear();
    }

    emit operationFinished(QStringLiteral("dropStash"), true, QString());
    return Result<void>::success();
}

void StashController::showStashDiff(const QString& repoPath, int index)
{
    if (m_taskRunner) {
        m_taskRunner->showStashDiff(repoPath, index);
        return;
    }

    // 回退：同步执行
    if (!m_stashService) {
        emit stashDiffLoaded(false, index, QStringLiteral("Stash service is not available."));
        return;
    }

    const Result<Diff> result = m_stashService->diff(repoPath, index);

    if (result.isFailure()) {
        emit stashDiffLoaded(false, index, result.errorMessage());
        emit errorOccurred(QStringLiteral("showStashDiff"), result.errorMessage());
        return;
    }

    if (m_diffLineModel) {
        m_diffLineModel->setDiff(result.value());
    }

    emit stashDiffLoaded(true, index, QString());
}

StashListModel* StashController::stashListModel() const
{
    return m_stashListModel;
}

void StashController::clear()
{
    if (m_stashListModel) {
        m_stashListModel->clear();
    }
}

// -- GitTaskRunner 异步回调 --

void StashController::onStashCreated(bool success, const QString& repoPath, const QString& message, const QString& errorMessage)
{
    if (success) {
        loadStashes(repoPath);
        emit operationFinished(QStringLiteral("saveStash"), true, QString());
    } else {
        emit operationFinished(QStringLiteral("saveStash"), false, errorMessage);
        emit errorOccurred(QStringLiteral("saveStash"), errorMessage);
    }
}

void StashController::onStashApplied(bool success, const QString& repoPath, int index, const QString& errorMessage)
{
    if (success) {
        loadStashes(repoPath);
        emit operationFinished(QStringLiteral("applyStash"), true, QString());
    } else {
        emit operationFinished(QStringLiteral("applyStash"), false, errorMessage);
        emit errorOccurred(QStringLiteral("applyStash"), errorMessage);
    }
}

void StashController::onStashDropped(bool success, const QString& repoPath, int index, const QString& errorMessage)
{
    if (success) {
        loadStashes(repoPath);
        if (m_diffLineModel) m_diffLineModel->clear();
        emit operationFinished(QStringLiteral("dropStash"), true, QString());
    } else {
        emit operationFinished(QStringLiteral("dropStash"), false, errorMessage);
        emit errorOccurred(QStringLiteral("dropStash"), errorMessage);
    }
}

void StashController::onStashDiffLoaded(bool success, const QString& repoPath, int index, const Diff& diff, const QString& errorMessage)
{
    Q_UNUSED(repoPath);
    if (success && m_diffLineModel) {
        m_diffLineModel->setDiff(diff);
    }
    if (!success) {
        emit errorOccurred(QStringLiteral("showStashDiff"), errorMessage);
    }
    emit stashDiffLoaded(success, index, errorMessage);
}
