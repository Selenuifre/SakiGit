#ifndef COMMITMESSAGESUGGESTION_H
#define COMMITMESSAGESUGGESTION_H

#include <QString>

// AI 生成的提交信息建议领域对象。
// 承载 AI 返回的完整消息、类型、范围、主题和正文，
// 以及原始响应文本（用于调试或回退展示）。
class CommitMessageSuggestion
{
public:
    CommitMessageSuggestion();

    // 生成的完整提交信息（如 "feat(auth): add OAuth2 login support"）
    QString fullMessage() const;
    void setFullMessage(const QString& message);

    // Conventional Commits 类型（feat, fix, chore, docs, refactor, test, style, perf, ci, build）
    QString type() const;
    void setType(const QString& type);

    // 影响范围（可为空）
    QString scope() const;
    void setScope(const QString& scope);

    // 标题行（不含 type 和 scope 前缀的部分）
    QString subject() const;
    void setSubject(const QString& subject);

    // 正文（详细描述，可为空）
    QString body() const;
    void setBody(const QString& body);

    // AI API 原始响应文本（用于调试和回退展示）
    QString rawResponse() const;
    void setRawResponse(const QString& rawResponse);

    // 是否成功解析出有效结果
    bool isValid() const;
    void setValid(bool valid);

    // 从 AI API 返回的 JSON 字符串构造建议对象
    static CommitMessageSuggestion fromJson(const QString& json);

    // 从纯文本构造建议对象（JSON 解析失败时的回退方案）
    static CommitMessageSuggestion fromPlainText(const QString& text);

    // 拼接 fullMessage：按照 Conventional Commits 格式生成完整消息
    static QString buildFullMessage(const QString& type,
                                    const QString& scope,
                                    const QString& subject,
                                    const QString& body);

private:
    QString m_fullMessage;
    QString m_type;
    QString m_scope;
    QString m_subject;
    QString m_body;
    QString m_rawResponse;
    bool    m_valid;
};

#endif // COMMITMESSAGESUGGESTION_H
