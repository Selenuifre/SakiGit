#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include "result.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QMap>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>

// 通用 HTTP 客户端封装——支持调用任意 REST API。
// 支持 GET / POST / PATCH / PUT，自动处理 JSON 请求头与认证。
// 可通过 setCustomHeaders() 设置平台专属请求头。
class HttpClient : public QObject
{
    Q_OBJECT

public:
    explicit HttpClient(QObject* parent = nullptr);
    virtual ~HttpClient() override;

    // 设置认证 token；默认以 Authorization: Bearer <token> 发送
    void setAuthToken(const QString& token);

    // 设置认证头名称（默认 "Authorization"，GitLab 等平台使用 "PRIVATE-TOKEN"）
    void setAuthHeaderName(const QString& headerName);

    // 设置基础 URL（默认为 https://api.github.com，保持向后兼容）
    void setBaseUrl(const QString& baseUrl);

    // 返回当前基础 URL
    QString baseUrl() const;

    // 设置自定义请求头（各平台 API 要求不同，如 Accept / API-Version 等）
    void setCustomHeaders(const QMap<QString, QString>& headers);

    // HTTP 方法（同步阻塞，内部使用 QEventLoop）
    virtual Result<QJsonDocument> get(const QString& path,
                                      const QMap<QString, QString>& params = {});
    virtual Result<QJsonDocument> post(const QString& path,
                                       const QJsonDocument& body);
    virtual Result<QJsonDocument> patch(const QString& path,
                                        const QJsonDocument& body);
    virtual Result<QJsonDocument> put(const QString& path,
                                      const QJsonDocument& body);

    // 上次请求的 HTTP 状态码
    int lastStatusCode() const;
    QString lastResponseBody() const;

    // 网络是否可用（基础连通性检查）
    bool isNetworkAvailable() const;

protected:
    // 供子类覆盖以注入模拟响应
    virtual Result<QByteArray> sendRequest(const QString& method,
                                           const QString& fullUrl,
                                           const QByteArray& bodyData);

private:
    QString buildUrl(const QString& path,
                     const QMap<QString, QString>& params = {}) const;
    QNetworkRequest makeRequest(const QString& fullUrl) const;

    QNetworkAccessManager* m_manager;
    QString m_authToken;
    QString m_authHeaderName;  // 默认 "Authorization"
    QString m_baseUrl;
    QMap<QString, QString> m_customHeaders;
    int m_lastStatusCode;
    QString m_lastResponseBody;
};

#endif // HTTPCLIENT_H
