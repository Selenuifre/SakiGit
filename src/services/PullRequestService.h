#ifndef PULLREQUESTSERVICE_H
#define PULLREQUESTSERVICE_H

#include "domain/PullRequest.h"
#include "infrastructure/result.h"

#include <QString>
#include <vector>

class ICodeHostingProvider;

// Pull Request 业务服务——依赖 ICodeHostingProvider 接口，
// 不直接依赖具体平台实现，因此可透明支持 GitHub / Gitee / GitLab 等。
class PullRequestService
{
public:
    // provider 参数决定当前使用的平台（调用方在调用前通过 ProviderFactory 获取）
    // 传 nullptr 会导致所有方法返回失败
    explicit PullRequestService();

    Result<std::vector<PullRequest>> listPullRequests(
        ICodeHostingProvider* provider,
        const QString& owner,
        const QString& repo) const;

    Result<PullRequest> getPullRequest(
        ICodeHostingProvider* provider,
        const QString& owner,
        const QString& repo,
        int number) const;

    Result<PullRequest> createPullRequest(
        ICodeHostingProvider* provider,
        const QString& owner,
        const QString& repo,
        const QString& title,
        const QString& body,
        const QString& head,
        const QString& base) const;

    Result<void> mergePullRequest(
        ICodeHostingProvider* provider,
        const QString& owner,
        const QString& repo,
        int number) const;
};

#endif // PULLREQUESTSERVICE_H
