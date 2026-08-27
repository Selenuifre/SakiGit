#include "GitLabProvider.h"

#include "infrastructure/HttpClient.h"

#include <QJsonDocument>
#include <QUrl>

GitLabProvider::GitLabProvider(HttpClient* httpClient)
    : m_httpClient(httpClient),
    m_hostDomain(QStringLiteral("gitlab.com"))
{
    applyHeaders();
}

void GitLabProvider::setHostDomain(const QString& hostDomain)
{
    m_hostDomain = hostDomain.trimmed();
    if (m_hostDomain.isEmpty()) {
        m_hostDomain = QStringLiteral("gitlab.com");
    }
    applyHeaders();
}

QString GitLabProvider::hostDomain() const
{
    return m_hostDomain;
}

void GitLabProvider::applyHeaders()
{
    if (!m_httpClient) return;

    m_httpClient->setBaseUrl(
        QStringLiteral("https://%1/api/v4").arg(m_hostDomain));

    m_httpClient->setAuthHeaderName(QStringLiteral("PRIVATE-TOKEN"));
    m_httpClient->setAuthToken(m_authToken);

    QMap<QString, QString> headers;
    headers.insert(QStringLiteral("Accept"), QStringLiteral("application/json"));
    m_httpClient->setCustomHeaders(headers);
}

CodeHostingPlatform GitLabProvider::platform() const
{
    return CodeHostingPlatform::GitLab;
}

QString GitLabProvider::apiBaseUrl() const
{
    return QStringLiteral("https://%1/api/v4").arg(m_hostDomain);
}

void GitLabProvider::setAuthToken(const QString& token)
{
    m_authToken = token.trimmed();
    applyHeaders();
}

QString GitLabProvider::authToken() const
{
    return m_authToken;
}

int GitLabProvider::lastStatusCode() const
{
    return m_httpClient ? m_httpClient->lastStatusCode() : 0;
}

QString GitLabProvider::projectPath(const QString& owner, const QString& repo) const
{
    // GitLab API 使用 URL 编码的 "namespace/project" 路径
    return QStringLiteral("%1%%2F%2").arg(owner, repo);
}

// ---- ICodeHostingProvider 接口实现 ----

Result<QJsonObject> GitLabProvider::getUserInfo()
{
    if (!m_httpClient) {
        return Result<QJsonObject>::failure(QStringLiteral("HttpClient is not initialized."));
    }

    applyHeaders();

    const Result<QJsonDocument> result = m_httpClient->get(QStringLiteral("/user"));

    if (result.isFailure()) {
        return Result<QJsonObject>::failure(result.error());
    }

    const QJsonDocument doc = result.value();
    if (!doc.isObject()) {
        return Result<QJsonObject>::failure(QStringLiteral("Invalid response: expected JSON object."));
    }

    return Result<QJsonObject>::success(doc.object());
}

Result<QJsonArray> GitLabProvider::listPullRequests(
    const QString& owner,
    const QString& repo,
    const QString& state)
{
    if (!m_httpClient) {
        return Result<QJsonArray>::failure(QStringLiteral("HttpClient is not initialized."));
    }

    applyHeaders();

    // GitLab MR API: GET /projects/:id/merge_requests
    const QString path = QStringLiteral("/projects/%1/merge_requests")
                             .arg(projectPath(owner, repo));

    QMap<QString, QString> params;
    // GitLab state: "opened", "closed", "merged", "all"
    if (state == QStringLiteral("open")) {
        params.insert(QStringLiteral("state"), QStringLiteral("opened"));
    } else if (state == QStringLiteral("closed")) {
        params.insert(QStringLiteral("state"), QStringLiteral("closed"));
    } else if (state == QStringLiteral("merged")) {
        params.insert(QStringLiteral("state"), QStringLiteral("merged"));
    } else {
        params.insert(QStringLiteral("state"), QStringLiteral("all"));
    }
    params.insert(QStringLiteral("per_page"), QStringLiteral("30"));

    const Result<QJsonDocument> result = m_httpClient->get(path, params);

    if (result.isFailure()) {
        return Result<QJsonArray>::failure(result.error());
    }

    const QJsonDocument doc = result.value();
    if (!doc.isArray()) {
        return Result<QJsonArray>::failure(QStringLiteral("Invalid response: expected JSON array."));
    }

    return Result<QJsonArray>::success(doc.array());
}

Result<QJsonObject> GitLabProvider::getPullRequest(
    const QString& owner,
    const QString& repo,
    int number)
{
    if (!m_httpClient) {
        return Result<QJsonObject>::failure(QStringLiteral("HttpClient is not initialized."));
    }

    applyHeaders();

    // GitLab: GET /projects/:id/merge_requests/:iid
    // GitLab 使用 iid（项目内 MR 编号）而非全局 id
    const QString path = QStringLiteral("/projects/%1/merge_requests/%2")
                             .arg(projectPath(owner, repo))
                             .arg(number);

    const Result<QJsonDocument> result = m_httpClient->get(path);

    if (result.isFailure()) {
        return Result<QJsonObject>::failure(result.error());
    }

    const QJsonDocument doc = result.value();
    if (!doc.isObject()) {
        return Result<QJsonObject>::failure(QStringLiteral("Invalid response: expected JSON object."));
    }

    return Result<QJsonObject>::success(doc.object());
}

Result<QJsonObject> GitLabProvider::createPullRequest(
    const QString& owner,
    const QString& repo,
    const QString& title,
    const QString& body,
    const QString& head,
    const QString& base)
{
    if (!m_httpClient) {
        return Result<QJsonObject>::failure(QStringLiteral("HttpClient is not initialized."));
    }

    applyHeaders();

    // GitLab: POST /projects/:id/merge_requests
    QJsonObject mrData;
    mrData.insert(QStringLiteral("title"), title);
    mrData.insert(QStringLiteral("description"), body);      // GitLab 用 "description"
    mrData.insert(QStringLiteral("source_branch"), head);     // GitLab 用 "source_branch"
    mrData.insert(QStringLiteral("target_branch"), base);     // GitLab 用 "target_branch"

    const QJsonDocument bodyDoc(mrData);
    const QString path = QStringLiteral("/projects/%1/merge_requests")
                             .arg(projectPath(owner, repo));
    const Result<QJsonDocument> result = m_httpClient->post(path, bodyDoc);

    if (result.isFailure()) {
        return Result<QJsonObject>::failure(result.error());
    }

    const QJsonDocument doc = result.value();
    if (!doc.isObject()) {
        return Result<QJsonObject>::failure(QStringLiteral("Invalid response: expected JSON object."));
    }

    return Result<QJsonObject>::success(doc.object());
}

Result<QJsonObject> GitLabProvider::mergePullRequest(
    const QString& owner,
    const QString& repo,
    int number)
{
    if (!m_httpClient) {
        return Result<QJsonObject>::failure(QStringLiteral("HttpClient is not initialized."));
    }

    applyHeaders();

    // GitLab: PUT /projects/:id/merge_requests/:iid/merge
    const QString path = QStringLiteral("/projects/%1/merge_requests/%2/merge")
                             .arg(projectPath(owner, repo))
                             .arg(number);

    QJsonDocument emptyBody{QJsonObject()};
    const Result<QJsonDocument> result = m_httpClient->put(path, emptyBody);

    if (result.isFailure()) {
        return Result<QJsonObject>::failure(result.error());
    }

    const QJsonDocument doc = result.value();
    if (!doc.isObject()) {
        return Result<QJsonObject>::failure(QStringLiteral("Invalid response: expected JSON object."));
    }

    return Result<QJsonObject>::success(doc.object());
}

PullRequest GitLabProvider::parsePullRequest(const QJsonObject& obj) const
{
    PullRequest pr;
    pr.setPlatform(CodeHostingPlatform::GitLab);

    // GitLab MR 字段映射
    pr.setNumber(obj.value(QStringLiteral("iid")).toInt());
    pr.setTitle(obj.value(QStringLiteral("title")).toString());
    pr.setBody(obj.value(QStringLiteral("description")).toString());   // GitLab: "description"

    // GitLab state: "opened" / "closed" / "merged"
    const QString mrState = obj.value(QStringLiteral("state")).toString();
    if (mrState == QStringLiteral("opened")) {
        pr.setState(QStringLiteral("open"));
    } else {
        pr.setState(mrState);
    }

    pr.setUrl(obj.value(QStringLiteral("web_url")).toString());        // GitLab: "web_url"

    const QJsonObject authorObj = obj.value(QStringLiteral("author")).toObject();
    pr.setAuthor(authorObj.value(QStringLiteral("username")).toString()); // GitLab: "username"

    pr.setHead(obj.value(QStringLiteral("source_branch")).toString());
    pr.setBase(obj.value(QStringLiteral("target_branch")).toString());

    const QString createdStr = obj.value(QStringLiteral("created_at")).toString();
    pr.setCreatedAt(QDateTime::fromString(createdStr, Qt::ISODate));

    const QString updatedStr = obj.value(QStringLiteral("updated_at")).toString();
    pr.setUpdatedAt(QDateTime::fromString(updatedStr, Qt::ISODate));

    return pr;
}

bool GitLabProvider::canHandleRemote(const QString& remoteUrl) const
{
    // 匹配 gitlab.com 或自托管 GitLab 域名
    if (remoteUrl.contains(m_hostDomain)) {
        return true;
    }

    // 也匹配通用 gitlab 关键字
    return remoteUrl.contains(QStringLiteral("gitlab"));
}

Result<ICodeHostingProvider::RepoInfo> GitLabProvider::parseRemoteUrl(
    const QString& remoteUrl) const
{
    if (!canHandleRemote(remoteUrl)) {
        return Result<RepoInfo>::failure(
            QStringLiteral("Not a GitLab remote URL: %1").arg(remoteUrl));
    }

    // 找到域名结束位置
    QString domainToFind = m_hostDomain;
    if (!remoteUrl.contains(domainToFind)) {
        // 尝试匹配 gitlab 关键字 或自定义域名
        domainToFind = QStringLiteral("gitlab.com");
    }

    int hostEnd = remoteUrl.indexOf(domainToFind);
    if (hostEnd < 0) {
        // 通用匹配：查找 : 或 / 之后的内容
        const int colonIdx = remoteUrl.lastIndexOf(QLatin1Char(':'));
        const int slashIdx = remoteUrl.indexOf(QLatin1Char('/'), remoteUrl.indexOf(QStringLiteral("://")) + 3);
        hostEnd = qMax(colonIdx, slashIdx);
    } else {
        hostEnd += domainToFind.length();
    }

    QString path = remoteUrl.mid(hostEnd);
    if (path.startsWith(QLatin1Char('/')) || path.startsWith(QLatin1Char(':')))
        path = path.mid(1);
    if (path.endsWith(QStringLiteral(".git")))
        path.chop(4);

    const int slash = path.indexOf(QLatin1Char('/'));
    if (slash < 0) {
        return Result<RepoInfo>::failure(
            QStringLiteral("Invalid owner/repo format: %1").arg(path));
    }

    RepoInfo info;
    info.owner = path.left(slash);
    info.repoName = path.mid(slash + 1);
    return Result<RepoInfo>::success(info);
}

QMap<QString, QString> GitLabProvider::customHeaders() const
{
    QMap<QString, QString> headers;
    headers.insert(QStringLiteral("Accept"), QStringLiteral("application/json"));
    return headers;
}
