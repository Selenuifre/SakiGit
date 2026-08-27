#include "CodeReviewController.h"

#include "models/ReviewFindingModel.h"
#include "services/CodeReviewService.h"

#include <QTimer>

CodeReviewController::CodeReviewController(CodeReviewService* service,
                                           ReviewFindingModel* model,
                                           QObject* parent)
    : BaseController(parent)
    , m_service(service)
    , m_model(model)
{
}

void CodeReviewController::reviewStaged(const QString& repoPath)
{
    if (!m_service) {
        emit reviewFailed(QStringLiteral("Code review service is not available."));
        return;
    }

    if (repoPath.trimmed().isEmpty()) {
        emit reviewFailed(QStringLiteral("No repository is currently selected."));
        return;
    }

    emit reviewStarted();

    QTimer::singleShot(50, this, [this, repoPath]() {
        const auto result = m_service->reviewStagedDiff(repoPath);

        if (result.isFailure()) {
            emit reviewFailed(result.errorMessage());
            emit errorOccurred(QStringLiteral("CodeReview:staged"), result.errorMessage());
            emit reviewFinished();
            return;
        }

        std::vector<ReviewFinding> findings = result.value();

        if (m_model) {
            m_model->setFindings(findings);
        }

        emit reviewCompleted(findings);
        emit operationFinished(QStringLiteral("CodeReview:staged"), true, QString());
        emit reviewFinished();
    });
}

void CodeReviewController::reviewWorkingTree(const QString& repoPath)
{
    if (!m_service) {
        emit reviewFailed(QStringLiteral("Code review service is not available."));
        return;
    }

    if (repoPath.trimmed().isEmpty()) {
        emit reviewFailed(QStringLiteral("No repository is currently selected."));
        return;
    }

    emit reviewStarted();

    QTimer::singleShot(50, this, [this, repoPath]() {
        const auto result = m_service->reviewWorkingTreeDiff(repoPath);

        if (result.isFailure()) {
            emit reviewFailed(result.errorMessage());
            emit errorOccurred(QStringLiteral("CodeReview:workingTree"), result.errorMessage());
            emit reviewFinished();
            return;
        }

        std::vector<ReviewFinding> findings = result.value();

        if (m_model) {
            m_model->setFindings(findings);
        }

        emit reviewCompleted(findings);
        emit operationFinished(QStringLiteral("CodeReview:workingTree"), true, QString());
        emit reviewFinished();
    });
}

void CodeReviewController::reviewCommit(const QString& repoPath, const QString& commitHash)
{
    if (!m_service) {
        emit reviewFailed(QStringLiteral("Code review service is not available."));
        return;
    }

    if (repoPath.trimmed().isEmpty()) {
        emit reviewFailed(QStringLiteral("No repository is currently selected."));
        return;
    }

    if (commitHash.trimmed().isEmpty()) {
        emit reviewFailed(QStringLiteral("Commit hash is empty."));
        return;
    }

    emit reviewStarted();

    QTimer::singleShot(50, this, [this, repoPath, commitHash]() {
        const auto result = m_service->reviewCommit(repoPath, commitHash);

        if (result.isFailure()) {
            emit reviewFailed(result.errorMessage());
            emit errorOccurred(QStringLiteral("CodeReview:commit"), result.errorMessage());
            emit reviewFinished();
            return;
        }

        std::vector<ReviewFinding> findings = result.value();

        if (m_model) {
            m_model->setFindings(findings);
        }

        emit reviewCompleted(findings);
        emit operationFinished(QStringLiteral("CodeReview:commit"), true, QString());
        emit reviewFinished();
    });
}

void CodeReviewController::cancelReview()
{
    if (m_service) {
        m_service->cancel();
    }
}

ReviewFindingModel* CodeReviewController::model() const
{
    return m_model;
}

bool CodeReviewController::hasFindings() const
{
    return m_model && !m_model->isEmpty();
}
