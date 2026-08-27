#ifndef PULLREQUESTCONTROLLER_H
#define PULLREQUESTCONTROLLER_H

#include "BaseController.h"
#include "domain/PullRequest.h"
#include "infrastructure/result.h"

#include <QString>

class ICodeHostingProvider;
class PullRequestModel;
class PullRequestService;

class PullRequestController : public BaseController
{
    Q_OBJECT

public:
    explicit PullRequestController(PullRequestService* prService,
                                   PullRequestModel* prModel,
                                   QObject* parent = nullptr);

    void loadPullRequests(ICodeHostingProvider* provider,
                          const QString& owner, const QString& repo);
    Result<PullRequest> createPullRequest(
        ICodeHostingProvider* provider,
        const QString& owner, const QString& repo,
        const QString& title, const QString& body,
        const QString& head, const QString& base);
    Result<void> mergePullRequest(
        ICodeHostingProvider* provider,
        const QString& owner, const QString& repo, int number);

    PullRequestModel* pullRequestModel() const;
    void clear();

signals:
    void pullRequestsLoaded(bool success, const QString& errorMessage);

private:
    PullRequestService* m_prService;
    PullRequestModel* m_prModel;
};

#endif // PULLREQUESTCONTROLLER_H
