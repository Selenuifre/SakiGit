#ifndef GITHUBAPICLIENT_H
#define GITHUBAPICLIENT_H

#include "infrastructure/result.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

class HttpClient;

// GitHub REST API 客户端——将 HTTP 调用封装为类型安全的 API 方法。
class GitHubApiClient
{
public:
    explicit GitHubApiClient(HttpClient* httpClient);

    // 设置认证 token
    void setAuthToken(const QString& token);

    // 验证当前 token 是否有效（GET /user）
    Result<QJsonObject> getUserInfo();

    // 列出仓库的 Pull Request（GET /repos/:owner/:repo/pulls）
    Result<QJsonArray> listPullRequests(
        const QString& owner,
        const QString& repo,
        const QString& state = QStringLiteral("open"));

    // 获取单个 Pull Request（GET /repos/:owner/:repo/pulls/:number）
    Result<QJsonObject> getPullRequest(
        const QString& owner,
        const QString& repo,
        int number);

    // 创建 Pull Request（POST /repos/:owner/:repo/pulls）
    Result<QJsonObject> createPullRequest(
        const QString& owner,
        const QString& repo,
        const QString& title,
        const QString& body,
        const QString& head,
        const QString& base);

    // 合并 Pull Request（PUT /repos/:owner/:repo/pulls/:number/merge）
    Result<QJsonObject> mergePullRequest(
        const QString& owner,
        const QString& repo,
        int number);

    // 返回最后一个响应的 HTTP 状态码
    int lastStatusCode() const;

private:
    HttpClient* m_httpClient;
};

#endif // GITHUBAPICLIENT_H
