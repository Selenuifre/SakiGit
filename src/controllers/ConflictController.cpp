#include "ConflictController.h"

#include "models/ConflictFileModel.h"
#include "services/ConflictService.h"

ConflictController::ConflictController(ConflictService* conflictService,
                                         ConflictFileModel* conflictFileModel,
                                         GitTaskRunner* taskRunner,
                                         QObject* parent)
    : BaseController(parent),
      m_conflictService(conflictService),
      m_conflictFileModel(conflictFileModel),
      m_taskRunner(taskRunner)
{
}

void ConflictController::loadConflicts(const QString& repoPath)
{
    if (!m_conflictService) {
        emit conflictsLoaded(false, QStringLiteral("Conflict service not available"));
        return;
    }

    const Result<std::vector<ConflictFile>> result =
        m_conflictService->listFiles(repoPath);

    if (result.isFailure()) {
        emit conflictsLoaded(false, result.errorMessage());
        return;
    }

    if (m_conflictFileModel) {
        m_conflictFileModel->setConflictFiles(result.value());
    }

    emit conflictsLoaded(true, QString());
}

Result<void> ConflictController::acceptOurs(const QString& repoPath,
                                              const QString& filePath)
{
    if (!m_conflictService)
        return Result<void>::failure(QStringLiteral("Conflict service not available"));

    const Result<void> result = m_conflictService->acceptOurs(repoPath, filePath);
    if (result.isSuccess()) {
        emit conflictResolved(filePath);
        emit operationFinished(QStringLiteral("acceptOurs"), true, QString());
    } else {
        emit operationFinished(QStringLiteral("acceptOurs"), false, result.errorMessage());
        emit errorOccurred(QStringLiteral("acceptOurs"), result.errorMessage());
    }
    return result;
}

Result<void> ConflictController::acceptTheirs(const QString& repoPath,
                                                const QString& filePath)
{
    if (!m_conflictService)
        return Result<void>::failure(QStringLiteral("Conflict service not available"));

    const Result<void> result = m_conflictService->acceptTheirs(repoPath, filePath);
    if (result.isSuccess()) {
        emit conflictResolved(filePath);
        emit operationFinished(QStringLiteral("acceptTheirs"), true, QString());
    } else {
        emit operationFinished(QStringLiteral("acceptTheirs"), false, result.errorMessage());
        emit errorOccurred(QStringLiteral("acceptTheirs"), result.errorMessage());
    }
    return result;
}

Result<void> ConflictController::markResolved(const QString& repoPath,
                                                const QString& filePath)
{
    if (!m_conflictService)
        return Result<void>::failure(QStringLiteral("Conflict service not available"));

    const Result<void> result = m_conflictService->markResolved(repoPath, filePath);
    if (result.isSuccess()) {
        emit conflictResolved(filePath);
        emit operationFinished(QStringLiteral("markResolved"), true, QString());
        // 重新加载冲突列表
        loadConflicts(repoPath);
    } else {
        emit operationFinished(QStringLiteral("markResolved"), false, result.errorMessage());
        emit errorOccurred(QStringLiteral("markResolved"), result.errorMessage());
    }
    return result;
}

Result<bool> ConflictController::hasConflicts(const QString& repoPath) const
{
    if (!m_conflictService)
        return Result<bool>::failure(QStringLiteral("Conflict service not available"));
    return m_conflictService->hasConflicts(repoPath);
}

ConflictFileModel* ConflictController::conflictFileModel() const
{
    return m_conflictFileModel;
}

void ConflictController::clear()
{
    if (m_conflictFileModel) {
        m_conflictFileModel->clear();
    }
}
