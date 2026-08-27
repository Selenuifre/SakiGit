#ifndef GITLABPROVIDER_H
#define GITLABPROVIDER_H

#include "infrastructure/ICodeHostingProvider.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

class HttpClient;

// GitLab API Provider——实现 ICodeHostingProvider 接口。
// GitLab 使用 Merge Request（MR）概念，API 路径格式与 GitHub 不同。
// 支持 gitlab.com 和自托管 GitLab 实例。
class GitLabProvider : public ICodeHostingProvider
{
public:
    explicit GitLabProvider(HttpClient* httpClient);

    // 设置自托管 GitLab 的域名（例如 "gitlab.mycompany.com"）
    void setHostDomain(const QString& hostDomain);
    QString hostDomain() const;

    CodeHostingPlatform platform() const override;
    QString apiBaseUrl() const override;

    void setAuthToken(const QString& token) override;
    QString authToken() const override;

    Result<QJsonObject> getUserInfo() override;
    int lastStatusCode() const override;

    Result<QJsonArray> listPullRequests(
        const QString& owner, const QString& repo,
        const QString& state = QStringLiteral("open")) override;

    Result<QJsonObject> getPullRequest(
        const QString& owner, const QString& repo, int number) override;

    Result<QJsonObject> createPullRequest(
        const QString& owner, const QString& repo,
        const QString& title, const QString& body,
        const QString& head, const QString& base) override;

    Result<QJsonObject> mergePullRequest(
        const QString& owner, const QString& repo, int number) override;

    PullRequest parsePullRequest(const QJsonObject& json) const override;

    bool canHandleRemote(const QString& remoteUrl) const override;
    Result<RepoInfo> parseRemoteUrl(const QString& remoteUrl) const override;
    QMap<QString, QString> customHeaders() const override;

private:
    // GitLab 使用项目 ID 或 URL 编码路径（owner%2Frepo）
    QString projectPath(const QString& owner, const QString& repo) const;
    void applyHeaders();

    HttpClient* m_httpClient;
    QString m_authToken;
    QString m_hostDomain;   // 默认 "gitlab.com"，自托管时可自定义
};

#endif // GITLABPROVIDER_H
