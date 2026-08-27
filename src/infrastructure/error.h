#ifndef ERROR_H
#define ERROR_H

#include <QMap>
#include <QString>

class Error
{
public:
    // 通用错误类型，后续 Git、配置、文件、网络等模块可以逐步复用
    enum class Code {
        None,
        Unknown,
        InvalidArgument,
        NotFound,
        PermissionDenied,
        ProcessFailed,
        Timeout,
        GitError,
        IOError,
        ConfigurationError
    };

public:
    // 创建一个空错误对象，表示没有错误
    Error();

    // 创建一个带错误类型和消息的错误对象
    Error(Code code, const QString& message);

    // 创建一个带错误类型、消息和详情的错误对象
    Error(Code code, const QString& message, const QString& detail);

    // 创建一个空错误对象
    static Error none();

    // 创建一个未知错误对象
    static Error unknown(const QString& message, const QString& detail = QString());

    // 创建一个参数错误对象
    static Error invalidArgument(const QString& message, const QString& detail = QString());

    // 创建一个未找到错误对象
    static Error notFound(const QString& message, const QString& detail = QString());

    // 创建一个 Git 错误对象
    static Error gitError(const QString& message, const QString& detail = QString());

    // 判断是否包含错误
    bool isValid() const;

    // 判断是否为空错误
    bool isEmpty() const;

    // 返回错误类型
    Code code() const;

    // 返回简短错误消息，适合展示给用户
    QString message() const;

    // 返回详细错误信息，适合日志或调试
    QString detail() const;

    // 设置详细错误信息
    void setDetail(const QString& detail);

    // 返回附加上下文，例如文件路径、命令名、模块名等
    QMap<QString, QString> context() const;

    // 获取指定上下文值
    QString contextValue(const QString& key) const;

    // 设置指定上下文值
    void setContextValue(const QString& key, const QString& value);

    // 以链式方式添加上下文值
    Error withContextValue(const QString& key, const QString& value) const;

    // 格式化为一行可读文本，方便日志输出
    QString toString() const;

    // 将错误类型转换为稳定字符串
    static QString codeToString(Code code);

private:
    Code m_code;
    QString m_message;
    QString m_detail;
    QMap<QString, QString> m_context;
};

#endif // ERROR_H
