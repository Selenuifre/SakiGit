#include "GitHubApiClient.h"

#include "infrastructure/HttpClient.h"

#include <QJsonDocument>
#include <QJsonParseError>

GitHubApiClient::GitHubApiClient(HttpClient* httpClient)
    : m_httpClient(httpClient)
{
}

void GitHubApiClient::setAuthToken(const QString& token)
{
    if (m_httpClient) {
        m_httpClient->setAuthToken(token);
    }
}

int GitHubApiClient::lastStatusCode() const
{
    return m_httpClient ? m_httpClient->lastStatusCode() : 0;
}

Result<QJsonObject> GitHubApiClient::getUserInfo()
{
    if (!m_httpClient) {
        return Result<QJsonObject>::failure(QStringLiteral("HttpClient is not initialized."));
    }

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

Result<QJsonArray> GitHubApiClient::listPullRequests(
    const QString& owner,
    const QString& repo,
    const QString& state)
{
    if (!m_httpClient) {
        return Result<QJsonArray>::failure(QStringLiteral("HttpClient is not initialized."));
    }

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

Result<QJsonObject> GitHubApiClient::getPullRequest(
    const QString& owner,
    const QString& repo,
    int number)
{
    if (!m_httpClient) {
        return Result<QJsonObject>::failure(QStringLiteral("HttpClient is not initialized."));
    }

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

Result<QJsonObject> GitHubApiClient::createPullRequest(
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

Result<QJsonObject> GitHubApiClient::mergePullRequest(
    const QString& owner,
    const QString& repo,
    int number)
{
    if (!m_httpClient) {
        return Result<QJsonObject>::failure(QStringLiteral("HttpClient is not initialized."));
    }

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
