#include "ReviewFinding.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <algorithm>

ReviewFinding::ReviewFinding()
    : m_lineNumber(-1)
    , m_aiGenerated(true)
{
}

QString ReviewFinding::filePath() const { return m_filePath; }
void ReviewFinding::setFilePath(const QString& filePath) { m_filePath = filePath.trimmed(); }

int ReviewFinding::lineNumber() const { return m_lineNumber; }
void ReviewFinding::setLineNumber(int lineNumber) { m_lineNumber = lineNumber; }

QString ReviewFinding::severity() const { return m_severity; }
void ReviewFinding::setSeverity(const QString& severity) { m_severity = severity.trimmed().toLower(); }

QString ReviewFinding::category() const { return m_category; }
void ReviewFinding::setCategory(const QString& category) { m_category = category.trimmed().toLower(); }

QString ReviewFinding::title() const { return m_title; }
void ReviewFinding::setTitle(const QString& title) { m_title = title.trimmed(); }

QString ReviewFinding::message() const { return m_message; }
void ReviewFinding::setMessage(const QString& message) { m_message = message.trimmed(); }

QString ReviewFinding::suggestion() const { return m_suggestion; }
void ReviewFinding::setSuggestion(const QString& suggestion) { m_suggestion = suggestion.trimmed(); }

QString ReviewFinding::codeSnippet() const { return m_codeSnippet; }
void ReviewFinding::setCodeSnippet(const QString& snippet) { m_codeSnippet = snippet; }

bool ReviewFinding::isAiGenerated() const { return m_aiGenerated; }
void ReviewFinding::setIsAiGenerated(bool aiGenerated) { m_aiGenerated = aiGenerated; }

bool ReviewFinding::isValid() const
{
    return !m_filePath.isEmpty() && !m_title.isEmpty();
}

int ReviewFinding::severityWeight(const QString& severity)
{
    const QString s = severity.trimmed().toLower();
    if (s == QStringLiteral("critical")) return 5;
    if (s == QStringLiteral("high"))     return 4;
    if (s == QStringLiteral("medium"))   return 3;
    if (s == QStringLiteral("low"))      return 2;
    if (s == QStringLiteral("info"))     return 1;
    return 0;
}

ReviewFinding ReviewFinding::fromJsonObject(const QJsonObject& obj)
{
    ReviewFinding finding;

    finding.setFilePath(obj.value(QStringLiteral("file")).toString());
    finding.setTitle(obj.value(QStringLiteral("title")).toString());

    // line 可能是 null（全局级别问题）
    const QJsonValue lineVal = obj.value(QStringLiteral("line"));
    if (lineVal.isDouble()) {
        finding.setLineNumber(lineVal.toInt());
    } else {
        finding.setLineNumber(-1);
    }

    finding.setSeverity(obj.value(QStringLiteral("severity")).toString(QStringLiteral("info")));
    finding.setCategory(obj.value(QStringLiteral("category")).toString());
    finding.setMessage(obj.value(QStringLiteral("message")).toString());
    finding.setSuggestion(obj.value(QStringLiteral("suggestion")).toString());
    finding.setCodeSnippet(obj.value(QStringLiteral("codeSnippet")).toString());
    finding.setIsAiGenerated(true);

    return finding;
}

std::vector<ReviewFinding> ReviewFinding::fromJsonArray(const QString& json)
{
    std::vector<ReviewFinding> findings;

    QString cleaned = json.trimmed();
    if (cleaned.isEmpty()) {
        return findings;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(cleaned.toUtf8(), &parseError);

    // 如果直接解析失败，尝试去掉 markdown 代码块包裹
    if (parseError.error != QJsonParseError::NoError) {
        // 去掉 ```json ... ``` 包裹
        const int codeBlockStart = cleaned.indexOf(QStringLiteral("```json"));
        if (codeBlockStart >= 0) {
            const int jsonStart = cleaned.indexOf(QLatin1Char('['), codeBlockStart);
            const int jsonEnd = cleaned.lastIndexOf(QLatin1Char(']'));
            if (jsonStart >= 0 && jsonEnd > jsonStart) {
                cleaned = cleaned.mid(jsonStart, jsonEnd - jsonStart + 1);
            }
        } else {
            // 尝试找到第一个 [ 和最后一个 ]
            const int firstBracket = cleaned.indexOf(QLatin1Char('['));
            const int lastBracket = cleaned.lastIndexOf(QLatin1Char(']'));
            if (firstBracket >= 0 && lastBracket > firstBracket) {
                cleaned = cleaned.mid(firstBracket, lastBracket - firstBracket + 1);
            }
        }

        doc = QJsonDocument::fromJson(cleaned.toUtf8(), &parseError);
    }

    if (parseError.error != QJsonParseError::NoError) {
        // 无法解析为 JSON 数组，返回空
        return findings;
    }

    // 可能返回的是数组，也可能是包含数组的对象
    if (doc.isArray()) {
        const QJsonArray arr = doc.array();
        findings.reserve(arr.size());
        for (const QJsonValue& val : arr) {
            if (val.isObject()) {
                ReviewFinding f = fromJsonObject(val.toObject());
                if (f.isValid()) {
                    findings.push_back(f);
                }
            }
        }
    } else if (doc.isObject()) {
        // 某些模型可能包装在 {"findings": [...]} 中
        const QJsonObject root = doc.object();
        if (root.contains(QStringLiteral("findings"))) {
            const QJsonArray arr = root.value(QStringLiteral("findings")).toArray();
            findings.reserve(arr.size());
            for (const QJsonValue& val : arr) {
                if (val.isObject()) {
                    ReviewFinding f = fromJsonObject(val.toObject());
                    if (f.isValid()) {
                        findings.push_back(f);
                    }
                }
            }
        } else {
            // 单个 finding 对象
            ReviewFinding f = fromJsonObject(root);
            if (f.isValid()) {
                findings.push_back(f);
            }
        }
    }

    // 按严重度降序排列（critical 在前）
    std::sort(findings.begin(), findings.end(),
              [](const ReviewFinding& a, const ReviewFinding& b) {
                  return severityWeight(a.severity()) > severityWeight(b.severity());
              });

    return findings;
}
