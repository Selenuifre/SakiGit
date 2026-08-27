#ifndef ICODEHOSTINGPROVIDER_H
#define ICODEHOSTINGPROVIDER_H

#include "domain/CodeHostingPlatform.h"
#include "domain/PullRequest.h"
#include "infrastructure/result.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

// 代码托管平台 Provider 抽象接口。
// 每个实现封装一个平台（GitHub / Gitee / GitLab）的 API 细节，
// 包括 API 根地址、认证方式、JSON 字段映射、Remote URL 解析等。
class ICodeHostingProvider
{
public:
    virtual ~ICodeHostingProvider() = default;

    // ---- 平台标识 ----
    virtual CodeHostingPlatform platform() const = 0;
    virtual QString apiBaseUrl() const = 0;        // 例如 "https://api.github.com"

    // ---- 认证 ----
    virtual void setAuthToken(const QString& token) = 0;
    virtual QString authToken() const = 0;

    // 验证 token 是否有效（返回 /user 接口响应）
    virtual Result<QJsonObject> getUserInfo() = 0;
    virtual int lastStatusCode() const = 0;

    // ---- Pull Request 操作（返回原始 JSON，由 Service 层进一步解析）----
    virtual Result<QJsonArray> listPullRequests(
        const QString& owner,
        const QString& repo,
        const QString& state = QStringLiteral("open")) = 0;

    virtual Result<QJsonObject> getPullRequest(
        const QString& owner,
        const QString& repo,
        int number) = 0;

    virtual Result<QJsonObject> createPullRequest(
        const QString& owner,
        const QString& repo,
        const QString& title,
        const QString& body,
        const QString& head,
        const QString& base) = 0;

    virtual Result<QJsonObject> mergePullRequest(
        const QString& owner,
        const QString& repo,
        int number) = 0;

    // ---- 平台特有的 JSON → PullRequest 解析 ----
    virtual PullRequest parsePullRequest(const QJsonObject& json) const = 0;

    // ---- Remote URL 解析 ----
    struct RepoInfo {
        QString owner;
        QString repoName;
    };

    // 判断是否能够处理给定的 git remote URL
    virtual bool canHandleRemote(const QString& remoteUrl) const = 0;

    // 从 git remote URL 中提取 owner/repoName
    // 类似 "git@github.com:owner/repo.git" → ("owner", "repo")
    virtual Result<RepoInfo> parseRemoteUrl(const QString& remoteUrl) const = 0;

    // ---- 自定义请求头（各平台 API 要求不同）----
    // 返回该平台需要的额外 HTTP headers（如 Accept, X-GitHub-Api-Version 等）
    virtual QMap<QString, QString> customHeaders() const = 0;
};

#endif // ICODEHOSTINGPROVIDER_H
