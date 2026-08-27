#include "CommitMessageSuggestion.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

CommitMessageSuggestion::CommitMessageSuggestion()
    : m_valid(false)
{
}

QString CommitMessageSuggestion::fullMessage() const
{
    return m_fullMessage;
}

void CommitMessageSuggestion::setFullMessage(const QString& message)
{
    m_fullMessage = message.trimmed();
}

QString CommitMessageSuggestion::type() const
{
    return m_type;
}

void CommitMessageSuggestion::setType(const QString& type)
{
    m_type = type.trimmed();
}

QString CommitMessageSuggestion::scope() const
{
    return m_scope;
}

void CommitMessageSuggestion::setScope(const QString& scope)
{
    m_scope = scope.trimmed();
}

QString CommitMessageSuggestion::subject() const
{
    return m_subject;
}

void CommitMessageSuggestion::setSubject(const QString& subject)
{
    m_subject = subject.trimmed();
}

QString CommitMessageSuggestion::body() const
{
    return m_body;
}

void CommitMessageSuggestion::setBody(const QString& body)
{
    m_body = body.trimmed();
}

QString CommitMessageSuggestion::rawResponse() const
{
    return m_rawResponse;
}

void CommitMessageSuggestion::setRawResponse(const QString& rawResponse)
{
    m_rawResponse = rawResponse;
}

bool CommitMessageSuggestion::isValid() const
{
    return m_valid;
}

void CommitMessageSuggestion::setValid(bool valid)
{
    m_valid = valid;
}

CommitMessageSuggestion CommitMessageSuggestion::fromJson(const QString& json)
{
    CommitMessageSuggestion suggestion;
    suggestion.setRawResponse(json);

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        // JSON 解析失败：尝试从文本中提取 JSON 块（AI 可能在 JSON 前后包了 markdown 代码块）
        QString cleaned = json.trimmed();

        // 去掉 ```json ... ``` 包裹
        const int codeBlockStart = cleaned.indexOf(QStringLiteral("```json"));
        if (codeBlockStart >= 0) {
            const int jsonStart = cleaned.indexOf(QLatin1Char('{'), codeBlockStart);
            const int jsonEnd = cleaned.lastIndexOf(QLatin1Char('}'));
            if (jsonStart >= 0 && jsonEnd > jsonStart) {
                cleaned = cleaned.mid(jsonStart, jsonEnd - jsonStart + 1);
            }
        } else {
            // 尝试找到第一个 { 和最后一个 }
            const int firstBrace = cleaned.indexOf(QLatin1Char('{'));
            const int lastBrace = cleaned.lastIndexOf(QLatin1Char('}'));
            if (firstBrace >= 0 && lastBrace > firstBrace) {
                cleaned = cleaned.mid(firstBrace, lastBrace - firstBrace + 1);
            }
        }

        // 重试解析
        if (cleaned != json) {
            const QJsonDocument doc2 = QJsonDocument::fromJson(cleaned.toUtf8(), &parseError);
            if (parseError.error == QJsonParseError::NoError && doc2.isObject()) {
                const QJsonObject obj = doc2.object();
                const QString type = obj.value(QStringLiteral("type")).toString();
                const QString scope = obj.value(QStringLiteral("scope")).toString();
                const QString subject = obj.value(QStringLiteral("subject")).toString();
                const QString body = obj.value(QStringLiteral("body")).toString();

                if (!type.isEmpty() || !subject.isEmpty()) {
                    suggestion.setType(type);
                    suggestion.setScope(scope);
                    suggestion.setSubject(subject);
                    suggestion.setBody(body);
                    suggestion.setFullMessage(buildFullMessage(type, scope, subject, body));
                    suggestion.setValid(true);
                    return suggestion;
                }
            }
        }

        // 彻底无法解析 JSON，返回纯文本回退
        return fromPlainText(json);
    }

    const QJsonObject obj = doc.object();
    const QString type = obj.value(QStringLiteral("type")).toString();
    const QString scope = obj.value(QStringLiteral("scope")).toString();
    const QString subject = obj.value(QStringLiteral("subject")).toString();
    const QString body = obj.value(QStringLiteral("body")).toString();

    if (type.isEmpty() && subject.isEmpty()) {
        return fromPlainText(json);
    }

    suggestion.setType(type);
    suggestion.setScope(scope);
    suggestion.setSubject(subject);
    suggestion.setBody(body);
    suggestion.setFullMessage(buildFullMessage(type, scope, subject, body));
    suggestion.setValid(true);

    return suggestion;
}

CommitMessageSuggestion CommitMessageSuggestion::fromPlainText(const QString& text)
{
    CommitMessageSuggestion suggestion;
    const QString cleaned = text.trimmed();
    suggestion.setRawResponse(cleaned);
    suggestion.setFullMessage(cleaned);

    // 尝试从纯文本第一行推断类型
    if (!cleaned.isEmpty()) {
        const int colonIdx = cleaned.indexOf(QLatin1Char(':'));
        if (colonIdx > 0) {
            // 尝试提取 type(scope): subject 格式
            const QString prefix = cleaned.left(colonIdx);
            const int parenOpen = prefix.indexOf(QLatin1Char('('));
            if (parenOpen > 0) {
                suggestion.setType(prefix.left(parenOpen));
                const int parenClose = prefix.indexOf(QLatin1Char(')'), parenOpen);
                if (parenClose > parenOpen) {
                    suggestion.setScope(prefix.mid(parenOpen + 1, parenClose - parenOpen - 1));
                }
            } else {
                suggestion.setType(prefix);
            }
            suggestion.setSubject(cleaned.mid(colonIdx + 1).trimmed());
        } else {
            suggestion.setSubject(cleaned);
        }
        suggestion.setValid(!cleaned.isEmpty());
    }

    return suggestion;
}

QString CommitMessageSuggestion::buildFullMessage(const QString& type,
                                                   const QString& scope,
                                                   const QString& subject,
                                                   const QString& body)
{
    QString message;

    if (!type.isEmpty()) {
        message += type;
        if (!scope.isEmpty()) {
            message += QStringLiteral("(%1)").arg(scope);
        }
        message += QStringLiteral(": ");
    }

    message += subject;

    if (!body.isEmpty()) {
        message += QStringLiteral("\n\n");
        message += body;
    }

    return message;
}
