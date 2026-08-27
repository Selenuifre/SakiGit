#include "MergeToolController.h"

#include "models/MergeLineModel.h"
#include "services/MergeToolService.h"

MergeToolController::MergeToolController(MergeToolService* mergeToolService,
                                           MergeLineModel* mergeLineModel,
                                           QObject* parent)
    : BaseController(parent),
      m_mergeToolService(mergeToolService),
      m_mergeLineModel(mergeLineModel)
{
}

void MergeToolController::loadThreeWayContent(const QString& repoPath,
                                                const QString& filePath)
{
    if (!m_mergeToolService) {
        emit contentLoadFailed(QStringLiteral("Merge tool service not available"));
        return;
    }

    const Result<MergeFileContent> result =
        m_mergeToolService->loadThreeWayContent(repoPath, filePath);

    if (result.isFailure()) {
        emit contentLoadFailed(result.errorMessage());
        return;
    }

    emit contentLoaded(result.value());
}

Result<void> MergeToolController::saveResolvedContent(
    const QString& repoPath, const QString& filePath, const QString& content)
{
    if (!m_mergeToolService)
        return Result<void>::failure(QStringLiteral("Merge tool service not available"));

    const Result<void> result =
        m_mergeToolService->saveResolvedContent(repoPath, filePath, content);

    if (result.isSuccess()) {
        emit contentSaved(filePath);
        emit operationFinished(QStringLiteral("saveResolved"), true, QString());
    } else {
        emit operationFinished(QStringLiteral("saveResolved"), false,
                               result.errorMessage());
        emit errorOccurred(QStringLiteral("saveResolved"), result.errorMessage());
    }
    return result;
}

MergeLineModel* MergeToolController::mergeLineModel() const
{
    return m_mergeLineModel;
}

void MergeToolController::clear()
{
    if (m_mergeLineModel) {
        m_mergeLineModel->clear();
    }
}
