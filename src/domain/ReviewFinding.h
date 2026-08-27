#ifndef REVIEWFINDING_H
#define REVIEWFINDING_H

#include <QString>
#include <vector>

class QJsonObject;

// AI Code Review 发现的问题领域对象。
// 承载 AI 返回的单个审查发现：文件路径、行号、严重度、分类、
// 标题、详细描述、修改建议和代码片段。
class ReviewFinding
{
public:
    ReviewFinding();

    // 文件路径（相对于仓库根目录）
    QString filePath() const;
    void setFilePath(const QString& filePath);

    // 行号（-1 表示全局级别问题，非特定行）
    int lineNumber() const;
    void setLineNumber(int lineNumber);

    // 严重度：critical / high / medium / low / info
    QString severity() const;
    void setSeverity(const QString& severity);

    // 分类：bug / security / performance / style /
    //       maintainability / best-practice / typo
    QString category() const;
    void setCategory(const QString& category);

    // 简短标题（一行概述）
    QString title() const;
    void setTitle(const QString& title);

    // 详细问题描述
    QString message() const;
    void setMessage(const QString& message);

    // 修改建议（可为空）
    QString suggestion() const;
    void setSuggestion(const QString& suggestion);

    // 相关代码片段（便于 UI 展示上下文，可为空）
    QString codeSnippet() const;
    void setCodeSnippet(const QString& snippet);

    // 是否为 AI 生成（预留静态分析工具扩展点）
    bool isAiGenerated() const;
    void setIsAiGenerated(bool aiGenerated);

    // 是否有效（至少 filePath 和 title 非空）
    bool isValid() const;

    // 严重度转整数用于排序（值越大越严重）
    static int severityWeight(const QString& severity);

    // 从 AI 返回的 JSON 数组解析发现列表
    static std::vector<ReviewFinding> fromJsonArray(const QString& json);

    // 从单个 JSON 对象解析一个发现
    static ReviewFinding fromJsonObject(const QJsonObject& obj);

private:
    QString m_filePath;
    int     m_lineNumber;
    QString m_severity;
    QString m_category;
    QString m_title;
    QString m_message;
    QString m_suggestion;
    QString m_codeSnippet;
    bool    m_aiGenerated;
};

#endif // REVIEWFINDING_H
