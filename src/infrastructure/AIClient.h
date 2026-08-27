#ifndef AICLIENT_H
#define AICLIENT_H

#include "result.h"

#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

// AI API 通信客户端。
// 使用 QNetworkAccessManager 发起 HTTPS 请求，
// 支持 OpenAI Chat Completions 兼容 API 格式。
class AIClient : public QObject
{
    Q_OBJECT

public:
    // AI 服务连接配置
    struct Config {
        QString provider;      // "openai" / "anthropic" / "custom"
        QString apiKey;        // API 密钥
        QString model;         // 模型名称，如 "gpt-4o", "gpt-4o-mini"
        QString baseUrl;       // API 基础 URL，默认 https://api.openai.com/v1
        int     timeoutMs = 120000; // 请求超时，单位毫秒
    };

    explicit AIClient(QObject* parent = nullptr);
    ~AIClient() override;

    // 设置连接配置
    void setConfig(const Config& config);
    Config config() const;

    // 发送聊天补全请求（同步风格，内部使用事件循环等待）
    // systemPrompt: 系统指令
    // userMessage:  用户消息（diff 内容）
    // maxTokens:    最大生成 token 数，默认 500
    // 返回 AI 响应文本；失败时返回错误信息
    Result<QString> chatComplete(const QString& systemPrompt,
                                  const QString& userMessage,
                                  int maxTokens = 0);

    // 检查配置是否有效（provider、apiKey、model 均非空）
    bool isConfigured() const;

private:
    // 构建 OpenAI 兼容的请求体 JSON
    QByteArray buildRequestBody(const QString& systemPrompt,
                                 const QString& userMessage,
                                 int maxTokens) const;

    // 构建请求 URL（baseUrl + /chat/completions）
    QString buildRequestUrl() const;

    Config m_config;
    QNetworkAccessManager* m_networkManager;
};

#endif // AICLIENT_H
