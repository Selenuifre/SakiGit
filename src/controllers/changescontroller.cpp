#include "changescontroller.h"

#include "domain/diff.h"
#include "domain/filechange.h"
#include "models/difflinemodel.h"
#include "models/filechangemodel.h"
#include "services/gitservice.h"
#include "services/gittaskrunner.h"

#include <QList>
#include <vector>

ChangesController::ChangesController(GitService* gitService,
                                     FileChangeModel* fileChangeModel,
                                     DiffLineModel* diffLineModel,
                                     QObject* parent)
    : BaseController(parent),
    m_gitService(gitService),
    m_fileChangeModel(fileChangeModel),
    m_diffLineModel(diffLineModel)
{
}

void ChangesController::setGitTaskRunner(GitTaskRunner* taskRunner)
{
    if (m_taskRunner) {
        QObject::disconnect(m_taskRunner, &GitTaskRunner::fileStaged,
                            this, &ChangesController::onFileStaged);
        QObject::disconnect(m_taskRunner, &GitTaskRunner::fileUnstaged,
                            this, &ChangesController::onFileUnstaged);
        QObject::disconnect(m_taskRunner, &GitTaskRunner::commitFinished,
                            this, &ChangesController::onCommitFinished);
        QObject::disconnect(m_taskRunner, &GitTaskRunner::diffLoaded,
                            this, &ChangesController::onDiffLoaded);
        QObject::disconnect(m_taskRunner, &GitTaskRunner::allFilesStaged,
                            this, &ChangesController::onAllFilesStaged);
    }

    m_taskRunner = taskRunner;

    if (m_taskRunner) {
        QObject::connect(m_taskRunner, &GitTaskRunner::fileStaged,
                         this, &ChangesController::onFileStaged);
        QObject::connect(m_taskRunner, &GitTaskRunner::fileUnstaged,
                         this, &ChangesController::onFileUnstaged);
        QObject::connect(m_taskRunner, &GitTaskRunner::commitFinished,
                         this, &ChangesController::onCommitFinished);
        QObject::connect(m_taskRunner, &GitTaskRunner::diffLoaded,
                         this, &ChangesController::onDiffLoaded);
        QObject::connect(m_taskRunner, &GitTaskRunner::allFilesStaged,
                         this, &ChangesController::onAllFilesStaged);
    }
}

void ChangesController::refreshChanges(const QString& repoPath)
{
    if (!m_gitService) {
        emit changesRefreshed(false, QStringLiteral("Git service is not available."));
        return;
    }

    const Result<QList<FileChange>> result = m_gitService->status(repoPath);

    if (result.isFailure()) {
        emit changesRefreshed(false, result.errorMessage());
        emit errorOccurred(QStringLiteral("refreshChanges"), result.errorMessage());
        return;
    }

    if (m_fileChangeModel) {
        const QList<FileChange>& changes = result.value();
        const std::vector<FileChange> changeVector(changes.begin(), changes.end());
        m_fileChangeModel->setFileChanges(changeVector);
    }

    emit changesRefreshed(true, QString());
}

void ChangesController::loadDiff(const QString& repoPath, const QString& filePath, bool staged)
{
    if (m_taskRunner) {
        m_taskRunner->loadDiff(repoPath, filePath, staged);
        return;
    }

    // 回退：同步执行
    if (!m_gitService) {
        emit diffLoaded(false, filePath, QStringLiteral("Git service is not available."));
        return;
    }

    const Result<Diff> result = m_gitService->diff(repoPath, filePath, staged);

    if (result.isFailure()) {
        emit diffLoaded(false, filePath, result.errorMessage());
        emit errorOccurred(QStringLiteral("loadDiff"), result.errorMessage());
        return;
    }

    if (m_diffLineModel) {
        m_diffLineModel->setDiff(result.value());
    }

    emit diffLoaded(true, filePath, QString());
}

Result<void> ChangesController::stageFile(const QString& repoPath, const QString& filePath)
{
    if (m_taskRunner) {
        m_pendingRepoPath = repoPath;
        m_taskRunner->stageFile(repoPath, filePath);
        return Result<void>::success();  // 结果通过 onFileStaged 异步返回
    }

    // 回退：同步执行
    if (!m_gitService) {
        return Result<void>::failure(QStringLiteral("Git service is not available."));
    }

    const Result<void> result = m_gitService->stageFile(repoPath, filePath);

    if (result.isFailure()) {
        emit fileStaged(false, filePath, result.errorMessage());
        emit errorOccurred(QStringLiteral("stageFile"), result.errorMessage());
        return result;
    }

    refreshChanges(repoPath);

    emit fileStaged(true, filePath, QString());
    return Result<void>::success();
}

Result<void> ChangesController::stageAllFiles(const QString& repoPath)
{
    if (m_taskRunner) {
        m_pendingRepoPath = repoPath;
        m_taskRunner->stageAllFiles(repoPath);
        return Result<void>::success();
    }

    // 回退：同步执行
    if (!m_gitService) {
        return Result<void>::failure(QStringLiteral("Git service is not available."));
    }

    const Result<QList<FileChange>> statusResult = m_gitService->status(repoPath);

    if (statusResult.isFailure()) {
        emit errorOccurred(QStringLiteral("stageAllFiles"), statusResult.errorMessage());
        return Result<void>::failure(statusResult.errorMessage());
    }

    const QList<FileChange>& changes = statusResult.value();

    for (const FileChange& change : changes) {
        if (!change.isStaged() && !change.isConflict()) {
            const Result<void> stageResult = m_gitService->stageFile(repoPath, change.path());

            if (stageResult.isFailure()) {
                emit errorOccurred(QStringLiteral("stageAllFiles"), stageResult.errorMessage());
                return stageResult;
            }
        }
    }

    refreshChanges(repoPath);

    return Result<void>::success();
}

Result<void> ChangesController::unstageFile(const QString& repoPath, const QString& filePath)
{
    if (m_taskRunner) {
        m_pendingRepoPath = repoPath;
        m_taskRunner->unstageFile(repoPath, filePath);
        return Result<void>::success();
    }

    // 回退：同步执行
    if (!m_gitService) {
        return Result<void>::failure(QStringLiteral("Git service is not available."));
    }

    const Result<void> result = m_gitService->unstageFile(repoPath, filePath);

    if (result.isFailure()) {
        emit fileUnstaged(false, filePath, result.errorMessage());
        emit errorOccurred(QStringLiteral("unstageFile"), result.errorMessage());
        return result;
    }

    refreshChanges(repoPath);

    emit fileUnstaged(true, filePath, QString());
    return Result<void>::success();
}

Result<void> ChangesController::unstageAllFiles(const QString& repoPath)
{
    if (!m_gitService) {
        return Result<void>::failure(QStringLiteral("Git service is not available."));
    }

    const Result<QList<FileChange>> statusResult = m_gitService->status(repoPath);

    if (statusResult.isFailure()) {
        emit errorOccurred(QStringLiteral("unstageAllFiles"), statusResult.errorMessage());
        return Result<void>::failure(statusResult.errorMessage());
    }

    const QList<FileChange>& changes = statusResult.value();

    for (const FileChange& change : changes) {
        if (change.isStaged()) {
            const Result<void> unstageResult = m_gitService->unstageFile(repoPath, change.path());

            if (unstageResult.isFailure()) {
                emit errorOccurred(QStringLiteral("unstageAllFiles"), unstageResult.errorMessage());
                return unstageResult;
            }
        }
    }

    refreshChanges(repoPath);

    return Result<void>::success();
}

Result<void> ChangesController::commit(const QString& repoPath, const QString& message)
{
    if (m_taskRunner) {
        m_pendingRepoPath = repoPath;
        m_taskRunner->commit(repoPath, message);
        return Result<void>::success();
    }

    // 回退：同步执行
    if (!m_gitService) {
        return Result<void>::failure(QStringLiteral("Git service is not available."));
    }

    const Result<void> result = m_gitService->commit(repoPath, message);

    if (result.isFailure()) {
        emit commitFinished(false, result.errorMessage());
        emit errorOccurred(QStringLiteral("commit"), result.errorMessage());
        return result;
    }

    if (m_diffLineModel) {
        m_diffLineModel->clear();
    }

    refreshChanges(repoPath);

    emit commitFinished(true, QString());
    return Result<void>::success();
}

Result<void> ChangesController::discardChanges(const QString& repoPath, const QString& filePath)
{
    Q_UNUSED(repoPath);
    Q_UNUSED(filePath);
    if (!m_gitService) {
        return Result<void>::failure(QStringLiteral("Git service is not available."));
    }

    const Result<void> result = m_gitService->restoreFile(repoPath, filePath);

    if (result.isFailure()) {
        emit errorOccurred(QStringLiteral("discardChanges"), result.errorMessage());
        return result;
    }

    // 丢弃成功后自动刷新变更列表
    refreshChanges(repoPath);

    return Result<void>::success();
}

Result<void> ChangesController::ignoreFile(const QString& repoPath, const QString& filePath)
{
    if (!m_gitService) {
        return Result<void>::failure(QStringLiteral("Git service is not available."));
    }

    // 取文件路径的最后一段作为忽略模式
    const QString pattern = filePath.section(QLatin1Char('/'), -1);

    const Result<void> result = m_gitService->addToGitignore(repoPath, pattern);

    if (result.isFailure()) {
        emit errorOccurred(QStringLiteral("ignoreFile"), result.errorMessage());
        return result;
    }

    // 忽略成功后自动刷新变更列表
    refreshChanges(repoPath);

    return Result<void>::success();
}

Result<void> ChangesController::renameFile(const QString& repoPath, const QString& oldPath, const QString& newPath)
{
    if (!m_gitService) {
        return Result<void>::failure(QStringLiteral("Git service is not available."));
    }

    const Result<void> result = m_gitService->renameFile(repoPath, oldPath, newPath);

    if (result.isFailure()) {
        emit errorOccurred(QStringLiteral("renameFile"), result.errorMessage());
        return result;
    }

    // 重命名成功后自动刷新变更列表
    refreshChanges(repoPath);

    return Result<void>::success();
}

FileChangeModel* ChangesController::fileChangeModel() const
{
    return m_fileChangeModel;
}

DiffLineModel* ChangesController::diffLineModel() const
{
    return m_diffLineModel;
}

void ChangesController::clear()
{
    if (m_fileChangeModel) {
        m_fileChangeModel->clear();
    }

    if (m_diffLineModel) {
        m_diffLineModel->clear();
    }
}

// -- 私有槽：接收 GitTaskRunner 异步结果 --

void ChangesController::onFileStaged(bool success, const QString& repoPath, const QString& filePath, const QString& errorMessage)
{
    if (success) {
        refreshChanges(repoPath);
    } else {
        emit errorOccurred(QStringLiteral("stageFile"), errorMessage);
    }
    emit fileStaged(success, filePath, errorMessage);
}

void ChangesController::onFileUnstaged(bool success, const QString& repoPath, const QString& filePath, const QString& errorMessage)
{
    if (success) {
        refreshChanges(repoPath);
    } else {
        emit errorOccurred(QStringLiteral("unstageFile"), errorMessage);
    }
    emit fileUnstaged(success, filePath, errorMessage);
}

void ChangesController::onCommitFinished(bool success, const QString& repoPath, const QString& errorMessage)
{
    if (success) {
        if (m_diffLineModel) m_diffLineModel->clear();
        refreshChanges(repoPath);
    } else {
        emit errorOccurred(QStringLiteral("commit"), errorMessage);
    }
    emit commitFinished(success, errorMessage);
}

void ChangesController::onDiffLoaded(bool success, const QString& repoPath, const QString& filePath, const Diff& diff, const QString& errorMessage)
{
    Q_UNUSED(repoPath);
    if (success && m_diffLineModel) {
        m_diffLineModel->setDiff(diff);
    }
    if (!success) {
        emit errorOccurred(QStringLiteral("loadDiff"), errorMessage);
    }
    emit diffLoaded(success, filePath, errorMessage);
}

void ChangesController::onAllFilesStaged(bool success, const QString& repoPath, const QString& errorMessage)
{
    if (success) {
        refreshChanges(repoPath);
    } else {
        emit errorOccurred(QStringLiteral("stageAllFiles"), errorMessage);
    }
}
