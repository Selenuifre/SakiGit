#ifndef LOGGER_H
#define LOGGER_H

#include <QMessageLogContext>
#include <QMutex>
#include <QString>
#include <QtGlobal>

class Logger
{
public:
    // 日志等级，等级越高表示问题越严重
    enum class Level {
        Debug,
        Info,
        Warning,
        Error
    };

public:
    // 返回全局唯一 Logger 实例
    static Logger& instance();

    // 设置日志文件路径
    void setLogFilePath(const QString& logFilePath);

    // 返回当前日志文件路径
    QString logFilePath() const;

    // 设置最低输出等级，低于该等级的日志会被忽略
    void setMinimumLevel(Level level);

    // 返回当前最低输出等级
    Level minimumLevel() const;

    // 设置是否输出到控制台
    void setConsoleEnabled(bool enabled);

    // 判断当前是否输出到控制台
    bool isConsoleEnabled() const;

    // 设置是否输出到文件
    void setFileEnabled(bool enabled);

    // 判断当前是否输出到文件
    bool isFileEnabled() const;

    // 输出 Debug 级别日志
    void debug(const QString& message, const QString& category = QString()) const;

    // 输出 Info 级别日志
    void info(const QString& message, const QString& category = QString()) const;

    // 输出 Warning 级别日志
    void warning(const QString& message, const QString& category = QString()) const;

    // 输出 Error 级别日志
    void error(const QString& message, const QString& category = QString()) const;

    // 输出指定级别的日志
    void log(Level level, const QString& message, const QString& category = QString()) const;

    // 清空当前日志文件内容
    bool clear();

    // 安装 Qt 消息处理器，用于接管 qDebug/qWarning/qCritical 输出
    static void installQtMessageHandler();

    // 卸载 Qt 消息处理器，恢复 Qt 默认消息处理逻辑
    static void uninstallQtMessageHandler();

    // 将日志等级转换为字符串
    static QString levelToString(Level level);

    // 将 Qt 消息类型转换为日志等级
    static Level levelFromQtMessageType(QtMsgType type);

private:
    // 创建 Logger，默认输出到控制台，不主动写文件
    Logger();

    // 禁止拷贝构造，保证只有一个 Logger 实例
    Logger(const Logger&) = delete;

    // 禁止赋值，保证只有一个 Logger 实例
    Logger& operator=(const Logger&) = delete;

    // 判断指定等级是否应该被输出
    bool shouldLog(Level level) const;

    // 格式化一条日志文本
    QString formatMessage(Level level, const QString& message, const QString& category) const;

    // 将格式化后的日志写入目标位置
    void writeLine(const QString& line) const;

    // Qt 全局消息处理函数
    static void qtMessageHandler(QtMsgType type,
                                 const QMessageLogContext& context,
                                 const QString& message);

private:
    // 保护日志配置和文件写入的互斥锁
    mutable QMutex m_mutex;

    // 日志文件路径
    QString m_logFilePath;

    // 最低输出日志等级
    Level m_minimumLevel;

    // 是否输出日志到控制台
    bool m_consoleEnabled;

    // 是否输出日志到文件
    bool m_fileEnabled;
};

#endif // LOGGER_H
