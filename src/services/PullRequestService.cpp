#include "PullRequestService.h"

#include "infrastructure/ICodeHostingProvider.h"

PullRequestService::PullRequestService()
{
}

Result<std::vector<PullRequest>> PullRequestService::listPullRequests(
    ICodeHostingProvider* provider,
    const QString& owner,
    const QString& repo) const
{
    if (!provider) {
        return Result<std::vector<PullRequest>>::failure(
            QStringLiteral("Provider is not initialized."));
    }

    const Result<QJsonArray> result = provider->listPullRequests(owner, repo);

    if (result.isFailure()) {
        return Result<std::vector<PullRequest>>::failure(result.error());
    }

    std::vector<PullRequest> prs;
    const QJsonArray array = result.value();

    for (const QJsonValue& val : array) {
        if (val.isObject()) {
            prs.push_back(provider->parsePullRequest(val.toObject()));
        }
    }

    return Result<std::vector<PullRequest>>::success(prs);
}

Result<PullRequest> PullRequestService::getPullRequest(
    ICodeHostingProvider* provider,
    const QString& owner,
    const QString& repo,
    int number) const
{
    if (!provider) {
        return Result<PullRequest>::failure(
            QStringLiteral("Provider is not initialized."));
    }

    const Result<QJsonObject> result = provider->getPullRequest(owner, repo, number);

    if (result.isFailure()) {
        return Result<PullRequest>::failure(result.error());
    }

    return Result<PullRequest>::success(provider->parsePullRequest(result.value()));
}

Result<PullRequest> PullRequestService::createPullRequest(
    ICodeHostingProvider* provider,
    const QString& owner,
    const QString& repo,
    const QString& title,
    const QString& body,
    const QString& head,
    const QString& base) const
{
    if (!provider) {
        return Result<PullRequest>::failure(
            QStringLiteral("Provider is not initialized."));
    }

    const Result<QJsonObject> result = provider->createPullRequest(
        owner, repo, title, body, head, base);

    if (result.isFailure()) {
        return Result<PullRequest>::failure(result.error());
    }

    return Result<PullRequest>::success(provider->parsePullRequest(result.value()));
}

Result<void> PullRequestService::mergePullRequest(
    ICodeHostingProvider* provider,
    const QString& owner,
    const QString& repo,
    int number) const
{
    if (!provider) {
        return Result<void>::failure(
            QStringLiteral("Provider is not initialized."));
    }

    const Result<QJsonObject> result = provider->mergePullRequest(owner, repo, number);

    if (result.isFailure()) {
        return Result<void>::failure(result.error());
    }

    return Result<void>::success();
}
