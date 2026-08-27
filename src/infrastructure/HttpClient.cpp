#include "HttpClient.h"

#include <QBuffer>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

HttpClient::HttpClient(QObject* parent)
    : QObject(parent),
    m_manager(new QNetworkAccessManager(this)),
    m_baseUrl(QStringLiteral("https://api.github.com")),
    m_authHeaderName(QStringLiteral("Authorization")),
    m_lastStatusCode(0)
{
}

HttpClient::~HttpClient()
{
}

void HttpClient::setAuthToken(const QString& token)
{
    m_authToken = token.trimmed();
}

void HttpClient::setAuthHeaderName(const QString& headerName)
{
    m_authHeaderName = headerName.trimmed();
    if (m_authHeaderName.isEmpty()) {
        m_authHeaderName = QStringLiteral("Authorization");
    }
}

void HttpClient::setBaseUrl(const QString& baseUrl)
{
    m_baseUrl = baseUrl.trimmed();
}

QString HttpClient::baseUrl() const
{
    return m_baseUrl;
}

void HttpClient::setCustomHeaders(const QMap<QString, QString>& headers)
{
    m_customHeaders = headers;
}

int HttpClient::lastStatusCode() const
{
    return m_lastStatusCode;
}

QString HttpClient::lastResponseBody() const
{
    return m_lastResponseBody;
}

bool HttpClient::isNetworkAvailable() const
{
    return true;
}

QString HttpClient::buildUrl(const QString& path,
                              const QMap<QString, QString>& params) const
{
    QString fullUrl = m_baseUrl;

    if (!path.startsWith(QLatin1Char('/'))) {
        fullUrl += QLatin1Char('/');
    }
    fullUrl += path;

    if (!params.isEmpty()) {
        QUrlQuery query;

        for (auto it = params.begin(); it != params.end(); ++it) {
            query.addQueryItem(it.key(), it.value());
        }

        fullUrl += QLatin1Char('?') + query.toString(QUrl::FullyEncoded);
    }

    return fullUrl;
}

QNetworkRequest HttpClient::makeRequest(const QString& fullUrl) const
{
    QUrl url(fullUrl);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json"));
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("SakiGit/1.0"));

    // 应用平台自定义请求头
    for (auto it = m_customHeaders.begin(); it != m_customHeaders.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }

    if (!m_authToken.isEmpty()) {
        const QString tokenValue = (m_authHeaderName == QStringLiteral("Authorization"))
            ? QStringLiteral("Bearer ") + m_authToken
            : m_authToken;
        request.setRawHeader(m_authHeaderName.toUtf8(), tokenValue.toUtf8());
    }

    return request;
}

Result<QByteArray> HttpClient::sendRequest(const QString& method,
                                            const QString& fullUrl,
                                            const QByteArray& bodyData)
{
    QNetworkRequest request = makeRequest(fullUrl);

    qDebug() << "[Http]" << method << fullUrl;

    QNetworkReply* reply = nullptr;

    if (method == QStringLiteral("GET")) {
        reply = m_manager->get(request);
    } else if (method == QStringLiteral("POST")) {
        reply = m_manager->post(request, bodyData);
    } else if (method == QStringLiteral("PATCH")) {
        QBuffer* buffer = new QBuffer;
        buffer->setData(bodyData);
        reply = m_manager->sendCustomRequest(request, "PATCH", buffer);
        buffer->setParent(reply);
    } else if (method == QStringLiteral("PUT")) {
        reply = m_manager->put(request, bodyData);
    } else {
        return Result<QByteArray>::failure(
            Error::invalidArgument(QStringLiteral("Unsupported HTTP method: %1").arg(method)));
    }

    // 同步等待（使用 QEventLoop）
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(30000); // 30 秒超时

    if (!reply->isFinished()) {
        loop.exec();
    }

    if (timer.isActive()) {
        timer.stop();
    } else {
        // 超时
        reply->abort();
        reply->deleteLater();
        return Result<QByteArray>::failure(
            Error(Error::Code::Timeout,
                  QStringLiteral("HTTP request timed out.")));
    }

    m_lastStatusCode = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray responseBody = reply->readAll();
    m_lastResponseBody = QString::fromUtf8(responseBody);

    qDebug() << "[Http] Response:" << m_lastStatusCode;

    const QNetworkReply::NetworkError networkError = reply->error();
    reply->deleteLater();

    if (networkError != QNetworkReply::NoError) {
        qDebug() << "[Http] Network error:" << m_lastResponseBody;
        return Result<QByteArray>::failure(
            Error::gitError(QStringLiteral("Network error: %1 (HTTP %2)")
                                .arg(m_lastResponseBody)
                                .arg(m_lastStatusCode))
                .withContextValue(QStringLiteral("httpStatusCode"),
                                  QString::number(m_lastStatusCode)));
    }

    return Result<QByteArray>::success(responseBody);
}

Result<QJsonDocument> HttpClient::get(const QString& path,
                                       const QMap<QString, QString>& params)
{
    const QString fullUrl = buildUrl(path, params);
    const Result<QByteArray> result = sendRequest(QStringLiteral("GET"), fullUrl, QByteArray());

    if (result.isFailure()) {
        return Result<QJsonDocument>::failure(result.error());
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(result.value(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        // 若响应体为空或非 JSON，返回空文档
        return Result<QJsonDocument>::success(QJsonDocument());
    }

    return Result<QJsonDocument>::success(doc);
}

Result<QJsonDocument> HttpClient::post(const QString& path,
                                        const QJsonDocument& body)
{
    const QString fullUrl = buildUrl(path);
    const QByteArray bodyData = body.isNull() ? QByteArray("{}") : body.toJson(QJsonDocument::Compact);
    const Result<QByteArray> result = sendRequest(QStringLiteral("POST"), fullUrl, bodyData);

    if (result.isFailure()) {
        return Result<QJsonDocument>::failure(result.error());
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(result.value(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        return Result<QJsonDocument>::success(QJsonDocument());
    }

    return Result<QJsonDocument>::success(doc);
}

Result<QJsonDocument> HttpClient::patch(const QString& path,
                                         const QJsonDocument& body)
{
    const QString fullUrl = buildUrl(path);
    const QByteArray bodyData = body.toJson(QJsonDocument::Compact);
    const Result<QByteArray> result = sendRequest(QStringLiteral("PATCH"), fullUrl, bodyData);

    if (result.isFailure()) {
        return Result<QJsonDocument>::failure(result.error());
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(result.value(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        return Result<QJsonDocument>::success(QJsonDocument());
    }

    return Result<QJsonDocument>::success(doc);
}

Result<QJsonDocument> HttpClient::put(const QString& path,
                                       const QJsonDocument& body)
{
    const QString fullUrl = buildUrl(path);
    const QByteArray bodyData = body.toJson(QJsonDocument::Compact);
    const Result<QByteArray> result = sendRequest(QStringLiteral("PUT"), fullUrl, bodyData);

    if (result.isFailure()) {
        return Result<QJsonDocument>::failure(result.error());
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(result.value(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        return Result<QJsonDocument>::success(QJsonDocument());
    }

    return Result<QJsonDocument>::success(doc);
}
