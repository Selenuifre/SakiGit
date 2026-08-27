#ifndef AISERVICE_H
#define AISERVICE_H

#include "infrastructure/result.h"

#include <QString>

class AIClient;
class SettingsService;

// AI 服务 —— 封装 AIClient 并提供面向业务的 AI 调用接口。
// 负责：从设置中加载 API 配置、管理 prompt 模板、调用 AIClient。
class AIService
{
public:
    AIService(AIClient* client, SettingsService* settingsService);

    // 从 SettingsService 重新加载 AI 配置并同步到 AIClient
    void reloadConfig();

    // 发送文本补全请求
    // systemPrompt: 系统指令（定义 AI 角色和行为）
    // userPrompt:   用户输入（具体任务和内容）
    // maxTokens:    最大生成 token 数，默认 500；Code Review 建议 3000
    Result<QString> complete(const QString& systemPrompt,
                              const QString& userPrompt,
                              int maxTokens = 0);

    // 检查 AI 服务是否已配置（API key、model 等均已设置）
    bool isConfigured() const;

    // 返回配置错误详情（供 CLI / Git Hook 使用）。
    // 若已配置则返回空字符串；否则返回描述缺少哪些配置项的消息。
    QString configurationError() const;

    // 返回当前使用的 AIClient
    AIClient* client() const;

    // 返回当前使用的 SettingsService
    SettingsService* settings() const;

private:
    AIClient*        m_client;    // 不持有所有权
    SettingsService* m_settings;  // 不持有所有权
};

#endif // AISERVICE_H
