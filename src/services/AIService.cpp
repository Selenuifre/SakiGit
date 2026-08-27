#include "AIService.h"

#include "infrastructure/AIClient.h"
#include "services/settingsservice.h"

#include <QHash>

namespace {

// 常见 AI 提供商的 API 基础 URL 映射
QHash<QString, QString> providerBaseUrls()
{
    static const QHash<QString, QString> urls = {
        {QStringLiteral("openai"),    QStringLiteral("https://api.openai.com/v1")},
        {QStringLiteral("anthropic"), QStringLiteral("https://api.anthropic.com/v1")},
        {QStringLiteral("azure"),     QStringLiteral("https://YOUR_RESOURCE.openai.azure.com")},
        {QStringLiteral("gemini"),    QStringLiteral("https://generativelanguage.googleapis.com/v1beta")},
        {QStringLiteral("deepseek"),  QStringLiteral("https://api.deepseek.com/v1")},
        {QStringLiteral("moonshot"),  QStringLiteral("https://api.moonshot.cn/v1")},
        {QStringLiteral("zhipu"),     QStringLiteral("https://open.bigmodel.cn/api/paas/v4")},
        {QStringLiteral("qwen"),      QStringLiteral("https://dashscope.aliyuncs.com/compatible-mode/v1")},
        {QStringLiteral("ollama"),    QStringLiteral("http://localhost:11434/v1")},
    };
    return urls;
}

} // anonymous namespace

AIService::AIService(AIClient* client, SettingsService* settingsService)
    : m_client(client)
    , m_settings(settingsService)
{
}

void AIService::reloadConfig()
{
    if (!m_client || !m_settings) {
        return;
    }

    AIClient::Config config;
    config.provider = m_settings->aiProvider();
    config.apiKey   = m_settings->aiApiKey();
    config.model    = m_settings->aiModel();

    // 根据 provider 自动匹配 API base URL
    const auto urls = providerBaseUrls();
    if (urls.contains(config.provider)) {
        config.baseUrl = urls.value(config.provider);
    }

    m_client->setConfig(config);
}

Result<QString> AIService::complete(const QString& systemPrompt,
                                     const QString& userPrompt,
                                     int maxTokens)
{
    if (!m_client) {
        return Result<QString>::failure(QStringLiteral("AI client is not available."));
    }

    if (!m_settings) {
        return Result<QString>::failure(QStringLiteral("Settings service is not available."));
    }

    // 确保配置是最新的
    reloadConfig();

    if (!m_client->isConfigured()) {
        return Result<QString>::failure(
            QStringLiteral("AI is not configured. "
                           "Please set AI provider, API key, and model in Preferences."));
    }

    return m_client->chatComplete(systemPrompt, userPrompt, maxTokens);
}

bool AIService::isConfigured() const
{
    return m_client && m_client->isConfigured();
}

QString AIService::configurationError() const
{
    if (!m_settings) {
        return QStringLiteral("Settings service is not available.");
    }

    const QString provider = m_settings->aiProvider();
    if (provider.isEmpty()) {
        return QStringLiteral("AI provider is not set. "
                              "Please configure AI provider in SakiGit Settings.");
    }

    const QString apiKey = m_settings->aiApiKey();
    if (apiKey.isEmpty()) {
        return QStringLiteral("AI API key is not set. "
                              "Please configure API key in SakiGit Settings.");
    }

    const QString model = m_settings->aiModel();
    if (model.isEmpty()) {
        return QStringLiteral("AI model is not set. "
                              "Please configure model in SakiGit Settings.");
    }

    if (!m_client) {
        return QStringLiteral("AI client is not available.");
    }

    return QString(); // 已配置，无错误
}

AIClient* AIService::client() const
{
    return m_client;
}

SettingsService* AIService::settings() const
{
    return m_settings;
}
