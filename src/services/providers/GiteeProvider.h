#ifndef GITEEPROVIDER_H
#define GITEEPROVIDER_H

#include "infrastructure/ICodeHostingProvider.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

class HttpClient;

// Gitee（码云）API Provider——实现 ICodeHostingProvider 接口。
// Gitee API v5 与 GitHub API 风格高度相似，主要差异在域名和认证方式。
class GiteeProvider : public ICodeHostingProvider
{
public:
    explicit GiteeProvider(HttpClient* httpClient);

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
    void applyHeaders();

    HttpClient* m_httpClient;
    QString m_authToken;
};

#endif // GITEEPROVIDER_H
