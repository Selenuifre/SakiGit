#include "GitHubProvider.h"

#include "infrastructure/HttpClient.h"

#include <QJsonDocument>
#include <QJsonParseError>

GitHubProvider::GitHubProvider(HttpClient* httpClient)
    : m_httpClient(httpClient)
{
    applyHeaders();
}

void GitHubProvider::applyHeaders()
{
    if (!m_httpClient) return;

    m_httpClient->setBaseUrl(QStringLiteral("https://api.github.com"));
    m_httpClient->setAuthHeaderName(QStringLiteral("Authorization"));
    m_httpClient->setAuthToken(m_authToken);

    QMap<QString, QString> headers;
    headers.insert(QStringLiteral("Accept"), QStringLiteral("application/vnd.github+json"));
    headers.insert(QStringLiteral("X-GitHub-Api-Version"), QStringLiteral("2022-11-28"));
    m_httpClient->setCustomHeaders(headers);
}

CodeHostingPlatform GitHubProvider::platform() const
{
    return CodeHostingPlatform::GitHub;
}

QString GitHubProvider::apiBaseUrl() const
{
    return QStringLiteral("https://api.github.com");
}

void GitHubProvider::setAuthToken(const QString& token)
{
    m_authToken = token.trimmed();
    applyHeaders();
}

QString GitHubProvider::authToken() const
{
    return m_authToken;
}

int GitHubProvider::lastStatusCode() const
{
    return m_httpClient ? m_httpClient->lastStatusCode() : 0;
}

Result<QJsonObject> GitHubProvider::getUserInfo()
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

Result<QJsonArray> GitHubProvider::listPullRequests(
    const QString& owner,
    const QString& repo,
    const QString& state)
{
    if (!m_httpClient) {
        return Result<QJsonArray>::failure(QStringLiteral("HttpClient is not initialized."));
    }

    applyHeaders();

    const QString path = QStringLiteral("/repos/%1/%2/pulls").arg(owner, repo);
    QMap<QString, QString> params;
    params.insert(QStringLiteral("state"), state);
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

Result<QJsonObject> GitHubProvider::getPullRequest(
    const QString& owner,
    const QString& repo,
    int number)
{
    if (!m_httpClient) {
        return Result<QJsonObject>::failure(QStringLiteral("HttpClient is not initialized."));
    }

    applyHeaders();

    const QString path = QStringLiteral("/repos/%1/%2/pulls/%3").arg(owner, repo).arg(number);
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

Result<QJsonObject> GitHubProvider::createPullRequest(
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

    QJsonObject prData;
    prData.insert(QStringLiteral("title"), title);
    prData.insert(QStringLiteral("body"), body);
    prData.insert(QStringLiteral("head"), head);
    prData.insert(QStringLiteral("base"), base);

    const QJsonDocument bodyDoc(prData);
    const QString path = QStringLiteral("/repos/%1/%2/pulls").arg(owner, repo);
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

Result<QJsonObject> GitHubProvider::mergePullRequest(
    const QString& owner,
    const QString& repo,
    int number)
{
    if (!m_httpClient) {
        return Result<QJsonObject>::failure(QStringLiteral("HttpClient is not initialized."));
    }

    applyHeaders();

    QJsonObject mergeData;
    mergeData.insert(QStringLiteral("merge_method"), QStringLiteral("merge"));

    const QJsonDocument bodyDoc(mergeData);
    const QString path = QStringLiteral("/repos/%1/%2/pulls/%3/merge").arg(owner, repo).arg(number);
    const Result<QJsonDocument> result = m_httpClient->put(path, bodyDoc);

    if (result.isFailure()) {
        return Result<QJsonObject>::failure(result.error());
    }

    const QJsonDocument doc = result.value();
    if (!doc.isObject()) {
        return Result<QJsonObject>::failure(QStringLiteral("Invalid response: expected JSON object."));
    }

    return Result<QJsonObject>::success(doc.object());
}

PullRequest GitHubProvider::parsePullRequest(const QJsonObject& obj) const
{
    PullRequest pr;
    pr.setPlatform(CodeHostingPlatform::GitHub);
    pr.setNumber(obj.value(QStringLiteral("number")).toInt());
    pr.setTitle(obj.value(QStringLiteral("title")).toString());
    pr.setBody(obj.value(QStringLiteral("body")).toString());
    pr.setState(obj.value(QStringLiteral("state")).toString());
    pr.setUrl(obj.value(QStringLiteral("html_url")).toString());

    const QJsonObject userObj = obj.value(QStringLiteral("user")).toObject();
    pr.setAuthor(userObj.value(QStringLiteral("login")).toString());

    const QJsonObject headObj = obj.value(QStringLiteral("head")).toObject();
    pr.setHead(headObj.value(QStringLiteral("label")).toString());

    const QJsonObject baseObj = obj.value(QStringLiteral("base")).toObject();
    pr.setBase(baseObj.value(QStringLiteral("label")).toString());

    const QString createdStr = obj.value(QStringLiteral("created_at")).toString();
    pr.setCreatedAt(QDateTime::fromString(createdStr, Qt::ISODate));

    const QString updatedStr = obj.value(QStringLiteral("updated_at")).toString();
    pr.setUpdatedAt(QDateTime::fromString(updatedStr, Qt::ISODate));

    return pr;
}

bool GitHubProvider::canHandleRemote(const QString& remoteUrl) const
{
    return remoteUrl.contains(QStringLiteral("github.com"));
}

Result<ICodeHostingProvider::RepoInfo> GitHubProvider::parseRemoteUrl(
    const QString& remoteUrl) const
{
    if (!canHandleRemote(remoteUrl)) {
        return Result<RepoInfo>::failure(
            QStringLiteral("Not a GitHub remote URL: %1").arg(remoteUrl));
    }

    // HTTPS: https://github.com/owner/repo.git
    // SSH:   git@github.com:owner/repo.git
    const int hostEnd = remoteUrl.indexOf(QStringLiteral("github.com")) + 10;
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

QMap<QString, QString> GitHubProvider::customHeaders() const
{
    QMap<QString, QString> headers;
    headers.insert(QStringLiteral("Accept"), QStringLiteral("application/vnd.github+json"));
    headers.insert(QStringLiteral("X-GitHub-Api-Version"), QStringLiteral("2022-11-28"));
    return headers;
}
