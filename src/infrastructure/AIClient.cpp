#include "AIClient.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslSocket>
#include <QTimer>
#include <QUrl>

AIClient::AIClient(QObject* parent)
    : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_config.baseUrl = QStringLiteral("https://api.openai.com/v1");
    m_config.timeoutMs = 120000;
}

AIClient::~AIClient() = default;

void AIClient::setConfig(const Config& config)
{
    m_config = config;

    if (m_config.baseUrl.trimmed().isEmpty()) {
        m_config.baseUrl = QStringLiteral("https://api.openai.com/v1");
    }

    if (m_config.timeoutMs <= 0) {
        m_config.timeoutMs = 120000;
    }
}

AIClient::Config AIClient::config() const
{
    return m_config;
}

Result<QString> AIClient::chatComplete(const QString& systemPrompt,
                                        const QString& userMessage,
                                        int maxTokens)
{
    if (!isConfigured()) {
        return Result<QString>::failure(
            QStringLiteral("AI client is not configured. "
                           "Please set provider, API key, and model in Preferences."));
    }

    // HTTPS 请求前检测 SSL 支持（Windows 下缺少 OpenSSL DLL 时最常见超时原因）
    if (buildRequestUrl().startsWith(QStringLiteral("https"))
        && !QSslSocket::supportsSsl()) {
        return Result<QString>::failure(
            QStringLiteral("HTTPS/SSL is not available on this system. "
                           "Please install OpenSSL runtime DLLs (libssl-3-x64.dll, libcrypto-3-x64.dll) "
                           "alongside the executable, or use the Qt maintenance tool to install "
                           "OpenSSL binaries. See https://wiki.qt.io/SSL for details."));
    }

    const QUrl url(buildRequestUrl());
    if (!url.isValid()) {
        return Result<QString>::failure(
            QStringLiteral("Invalid AI API URL: %1").arg(buildRequestUrl()));
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json"));

    if (!m_config.apiKey.isEmpty()) {
        const QString authHeader = QStringLiteral("Bearer ") + m_config.apiKey;
        request.setRawHeader("Authorization", authHeader.toUtf8());
    }

    const QByteArray body = buildRequestBody(systemPrompt, userMessage, maxTokens);

    QNetworkReply* reply = m_networkManager->post(request, body);

    // 使用事件循环等待响应（同步风格）
    QEventLoop loop;
    QTimer timer;

    // 设置超时
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    // 某些 Qt 版本在 SSL 握手失败时先发 errorOccurred 而非 finished
    QObject::connect(reply, &QNetworkReply::errorOccurred, &loop, &QEventLoop::quit);

    timer.start(m_config.timeoutMs);
    loop.exec();

    // 检查超时
    if (!timer.isActive()) {
        reply->abort();
        reply->deleteLater();
        return Result<QString>::failure(
            QStringLiteral("AI API request timed out after %1 ms").arg(m_config.timeoutMs));
    }

    timer.stop();

    // 检查网络错误
    if (reply->error() != QNetworkReply::NoError) {
        const QString errorMsg = reply->errorString();
        const QByteArray errorBody = reply->readAll();
        reply->deleteLater();

        QString detail = errorMsg;
        if (!errorBody.isEmpty()) {
            detail += QStringLiteral(" — ") + QString::fromUtf8(errorBody);
        }

        return Result<QString>::failure(
            QStringLiteral("AI API request failed: %1").arg(detail));
    }

    // 检查 HTTP 状态码
    const int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray responseData = reply->readAll();
    reply->deleteLater();

    if (statusCode < 200 || statusCode >= 300) {
        QString detail = QString::fromUtf8(responseData);
        return Result<QString>::failure(
            QStringLiteral("AI API returned HTTP %1: %2").arg(statusCode).arg(detail));
    }

    // 解析 OpenAI 兼容响应 JSON
    const QJsonDocument doc = QJsonDocument::fromJson(responseData);
    if (!doc.isObject()) {
        return Result<QString>::failure(
            QStringLiteral("AI API returned invalid JSON response."));
    }

    const QJsonObject root = doc.object();

    // 检查是否有错误字段
    if (root.contains(QStringLiteral("error"))) {
        const QJsonObject errorObj = root.value(QStringLiteral("error")).toObject();
        const QString errorMsg = errorObj.value(QStringLiteral("message")).toString(
            QStringLiteral("Unknown API error"));
        return Result<QString>::failure(
            QStringLiteral("AI API error: %1").arg(errorMsg));
    }

    // 提取 choices[0].message.content
    const QJsonArray choices = root.value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        return Result<QString>::failure(
            QStringLiteral("AI API returned empty choices array."));
    }

    const QJsonObject firstChoice = choices.first().toObject();
    const QJsonObject message = firstChoice.value(QStringLiteral("message")).toObject();

    QString content = message.value(QStringLiteral("content")).toString();
    if (content.isEmpty()) {
        content = message.value(QStringLiteral("reasoning_content")).toString();
    }

    if (content.isEmpty()) {
        return Result<QString>::failure(
            QStringLiteral("AI API returned empty message content."));
    }

    return Result<QString>::success(content);
}

bool AIClient::isConfigured() const
{
    return !m_config.provider.trimmed().isEmpty()
           && !m_config.apiKey.trimmed().isEmpty()
           && !m_config.model.trimmed().isEmpty();
}

QByteArray AIClient::buildRequestBody(const QString& systemPrompt,
                                       const QString& userMessage,
                                       int maxTokens) const
{
    QJsonObject root;

    root[QStringLiteral("model")] = m_config.model;

    QJsonArray messages;

    // system message
    {
        QJsonObject sysMsg;
        sysMsg[QStringLiteral("role")] = QStringLiteral("system");
        sysMsg[QStringLiteral("content")] = systemPrompt;
        messages.append(sysMsg);
    }

    // user message
    {
        QJsonObject userMsg;
        userMsg[QStringLiteral("role")] = QStringLiteral("user");
        userMsg[QStringLiteral("content")] = userMessage;
        messages.append(userMsg);
    }

    root[QStringLiteral("messages")] = messages;

    // max_tokens > 0 时才设限，0 表示不限制（由 API 使用模型默认值）
    if (maxTokens > 0) {
        root[QStringLiteral("max_tokens")] = maxTokens;
    }
    root[QStringLiteral("temperature")] = 0.3;

    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

QString AIClient::buildRequestUrl() const
{
    QString url = m_config.baseUrl;
    // 去掉末尾斜杠
    while (url.endsWith(QLatin1Char('/'))) {
        url.chop(1);
    }

    // 判断是否已是完整的 chat/completions 路径
    if (url.endsWith(QStringLiteral("/chat/completions"))) {
        return url;
    }

    return url + QStringLiteral("/chat/completions");
}
