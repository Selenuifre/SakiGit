#include "logger.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QTextStream>

namespace {
QtMessageHandler g_previousMessageHandler = nullptr;
}

Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}

Logger::Logger()
    : m_minimumLevel(Level::Debug),
    m_consoleEnabled(true),
    m_fileEnabled(false)
{
}

void Logger::setLogFilePath(const QString& logFilePath)
{
    QMutexLocker locker(&m_mutex);

    m_logFilePath = QDir::cleanPath(logFilePath.trimmed());
    m_fileEnabled = !m_logFilePath.isEmpty();
}

QString Logger::logFilePath() const
{
    QMutexLocker locker(&m_mutex);
    return m_logFilePath;
}

void Logger::setMinimumLevel(Level level)
{
    QMutexLocker locker(&m_mutex);
    m_minimumLevel = level;
}

Logger::Level Logger::minimumLevel() const
{
    QMutexLocker locker(&m_mutex);
    return m_minimumLevel;
}

void Logger::setConsoleEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_consoleEnabled = enabled;
}

bool Logger::isConsoleEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_consoleEnabled;
}

void Logger::setFileEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_fileEnabled = enabled;
}

bool Logger::isFileEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_fileEnabled;
}

void Logger::debug(const QString& message, const QString& category) const
{
    log(Level::Debug, message, category);
}

void Logger::info(const QString& message, const QString& category) const
{
    log(Level::Info, message, category);
}

void Logger::warning(const QString& message, const QString& category) const
{
    log(Level::Warning, message, category);
}

void Logger::error(const QString& message, const QString& category) const
{
    log(Level::Error, message, category);
}

void Logger::log(Level level, const QString& message, const QString& category) const
{
    if (!shouldLog(level)) {
        return;
    }

    writeLine(formatMessage(level, message, category));
}

bool Logger::clear()
{
    QMutexLocker locker(&m_mutex);

    if (m_logFilePath.isEmpty()) {
        return false;
    }

    QFile file(m_logFilePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }

    file.close();
    return true;
}

void Logger::installQtMessageHandler()
{
    if (!g_previousMessageHandler) {
        g_previousMessageHandler = qInstallMessageHandler(Logger::qtMessageHandler);
    }
}

void Logger::uninstallQtMessageHandler()
{
    if (g_previousMessageHandler) {
        qInstallMessageHandler(g_previousMessageHandler);
        g_previousMessageHandler = nullptr;
    }
}

QString Logger::levelToString(Level level)
{
    switch (level) {
    case Level::Debug:
        return QStringLiteral("DEBUG");
    case Level::Info:
        return QStringLiteral("INFO");
    case Level::Warning:
        return QStringLiteral("WARNING");
    case Level::Error:
        return QStringLiteral("ERROR");
    default:
        return QStringLiteral("UNKNOWN");
    }
}

Logger::Level Logger::levelFromQtMessageType(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return Level::Debug;
    case QtInfoMsg:
        return Level::Info;
    case QtWarningMsg:
        return Level::Warning;
    case QtCriticalMsg:
    case QtFatalMsg:
        return Level::Error;
    default:
        return Level::Info;
    }
}

bool Logger::shouldLog(Level level) const
{
    QMutexLocker locker(&m_mutex);
    return static_cast<int>(level) >= static_cast<int>(m_minimumLevel);
}

QString Logger::formatMessage(Level level, const QString& message, const QString& category) const
{
    const QString time = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
    const QString levelText = levelToString(level);

    if (category.trimmed().isEmpty()) {
        return QStringLiteral("[%1] [%2] %3")
        .arg(time, levelText, message);
    }

    return QStringLiteral("[%1] [%2] [%3] %4")
        .arg(time, levelText, category.trimmed(), message);
}

void Logger::writeLine(const QString& line) const
{
    QMutexLocker locker(&m_mutex);

    if (m_consoleEnabled) {
        QTextStream stream(stderr);
        stream << line << Qt::endl;
    }

    if (!m_fileEnabled || m_logFilePath.isEmpty()) {
        return;
    }

    const QFileInfo fileInfo(m_logFilePath);
    const QDir parentDir = fileInfo.absoluteDir();

    if (!parentDir.exists()) {
        QDir().mkpath(parentDir.absolutePath());
    }

    QFile file(m_logFilePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream << line << Qt::endl;
}

void Logger::qtMessageHandler(QtMsgType type,
                              const QMessageLogContext& context,
                              const QString& message)
{
    QString category;

    if (context.category && QString::fromUtf8(context.category) != QStringLiteral("default")) {
        category = QString::fromUtf8(context.category);
    }

    Logger::instance().log(levelFromQtMessageType(type), message, category);

    if (type == QtFatalMsg) {
        abort();
    }
}
