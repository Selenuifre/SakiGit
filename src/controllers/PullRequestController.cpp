#include "PullRequestController.h"

#include "models/PullRequestModel.h"
#include "services/PullRequestService.h"
#include "infrastructure/ICodeHostingProvider.h"

PullRequestController::PullRequestController(PullRequestService* prService,
                                             PullRequestModel* prModel,
                                             QObject* parent)
    : BaseController(parent),
    m_prService(prService),
    m_prModel(prModel)
{
}

void PullRequestController::loadPullRequests(ICodeHostingProvider* provider,
                                              const QString& owner,
                                              const QString& repo)
{
    if (!m_prService) {
        emit pullRequestsLoaded(false, QStringLiteral("PullRequestService is not available."));
        return;
    }

    if (!provider) {
        emit pullRequestsLoaded(false, QStringLiteral("Provider is not available."));
        return;
    }

    const Result<std::vector<PullRequest>> result =
        m_prService->listPullRequests(provider, owner, repo);

    if (result.isFailure()) {
        emit pullRequestsLoaded(false, result.errorMessage());
        emit errorOccurred(QStringLiteral("loadPullRequests"), result.errorMessage());
        return;
    }

    if (m_prModel) {
        m_prModel->setPullRequests(result.value());
    }

    emit pullRequestsLoaded(true, QString());
}

Result<PullRequest> PullRequestController::createPullRequest(
    ICodeHostingProvider* provider,
    const QString& owner, const QString& repo,
    const QString& title, const QString& body,
    const QString& head, const QString& base)
{
    if (!m_prService) {
        const QString errorMessage = QStringLiteral("PullRequestService is not available.");
        emit operationFinished(QStringLiteral("createPullRequest"), false, errorMessage);
        return Result<PullRequest>::failure(errorMessage);
    }

    if (!provider) {
        const QString errorMessage = QStringLiteral("Provider is not available.");
        emit operationFinished(QStringLiteral("createPullRequest"), false, errorMessage);
        return Result<PullRequest>::failure(errorMessage);
    }

    const Result<PullRequest> result =
        m_prService->createPullRequest(provider, owner, repo, title, body, head, base);

    if (result.isFailure()) {
        emit operationFinished(QStringLiteral("createPullRequest"), false,
                               result.errorMessage());
        emit errorOccurred(QStringLiteral("createPullRequest"), result.errorMessage());
        return result;
    }

    // 刷新列表
    loadPullRequests(provider, owner, repo);

    emit operationFinished(QStringLiteral("createPullRequest"), true, QString());
    return result;
}

Result<void> PullRequestController::mergePullRequest(
    ICodeHostingProvider* provider,
    const QString& owner, const QString& repo, int number)
{
    if (!m_prService) {
        const QString errorMessage = QStringLiteral("PullRequestService is not available.");
        emit operationFinished(QStringLiteral("mergePullRequest"), false, errorMessage);
        return Result<void>::failure(errorMessage);
    }

    if (!provider) {
        const QString errorMessage = QStringLiteral("Provider is not available.");
        emit operationFinished(QStringLiteral("mergePullRequest"), false, errorMessage);
        return Result<void>::failure(errorMessage);
    }

    const Result<void> result = m_prService->mergePullRequest(provider, owner, repo, number);

    if (result.isFailure()) {
        emit operationFinished(QStringLiteral("mergePullRequest"), false,
                               result.errorMessage());
        emit errorOccurred(QStringLiteral("mergePullRequest"), result.errorMessage());
        return result;
    }

    // 刷新列表
    loadPullRequests(provider, owner, repo);

    emit operationFinished(QStringLiteral("mergePullRequest"), true, QString());
    return Result<void>::success();
}

PullRequestModel* PullRequestController::pullRequestModel() const
{
    return m_prModel;
}

void PullRequestController::clear()
{
    if (m_prModel) {
        m_prModel->clear();
    }
}
