#include "error.h"

#include <QStringList>

Error::Error()
    : m_code(Code::None)
{
}

Error::Error(Code code, const QString& message)
    : m_code(code),
    m_message(message.trimmed())
{
}

Error::Error(Code code, const QString& message, const QString& detail)
    : m_code(code),
    m_message(message.trimmed()),
    m_detail(detail.trimmed())
{
}

Error Error::none()
{
    return Error();
}

Error Error::unknown(const QString& message, const QString& detail)
{
    return Error(Code::Unknown, message, detail);
}

Error Error::invalidArgument(const QString& message, const QString& detail)
{
    return Error(Code::InvalidArgument, message, detail);
}

Error Error::notFound(const QString& message, const QString& detail)
{
    return Error(Code::NotFound, message, detail);
}

Error Error::gitError(const QString& message, const QString& detail)
{
    return Error(Code::GitError, message, detail);
}

bool Error::isValid() const
{
    return m_code != Code::None;
}

bool Error::isEmpty() const
{
    return !isValid();
}

Error::Code Error::code() const
{
    return m_code;
}

QString Error::message() const
{
    return m_message;
}

QString Error::detail() const
{
    return m_detail;
}

void Error::setDetail(const QString& detail)
{
    m_detail = detail.trimmed();
}

QMap<QString, QString> Error::context() const
{
    return m_context;
}

QString Error::contextValue(const QString& key) const
{
    return m_context.value(key.trimmed());
}

void Error::setContextValue(const QString& key, const QString& value)
{
    const QString cleanKey = key.trimmed();

    if (cleanKey.isEmpty()) {
        return;
    }

    const QString cleanValue = value.trimmed();

    if (cleanValue.isEmpty()) {
        m_context.remove(cleanKey);
        return;
    }

    m_context.insert(cleanKey, cleanValue);
}

Error Error::withContextValue(const QString& key, const QString& value) const
{
    Error error = *this;
    error.setContextValue(key, value);
    return error;
}

QString Error::toString() const
{
    if (isEmpty()) {
        return QString();
    }

    QStringList parts;
    parts << QStringLiteral("[%1]").arg(codeToString(m_code));

    if (!m_message.isEmpty()) {
        parts << m_message;
    }

    if (!m_detail.isEmpty()) {
        parts << QStringLiteral("Detail: %1").arg(m_detail);
    }

    if (!m_context.isEmpty()) {
        QStringList contextParts;

        for (auto it = m_context.constBegin(); it != m_context.constEnd(); ++it) {
            contextParts << QStringLiteral("%1=%2").arg(it.key(), it.value());
        }

        parts << QStringLiteral("Context: %1").arg(contextParts.join(QStringLiteral(", ")));
    }

    return parts.join(QStringLiteral(" "));
}

QString Error::codeToString(Code code)
{
    switch (code) {
    case Code::None:
        return QStringLiteral("None");
    case Code::Unknown:
        return QStringLiteral("Unknown");
    case Code::InvalidArgument:
        return QStringLiteral("InvalidArgument");
    case Code::NotFound:
        return QStringLiteral("NotFound");
    case Code::PermissionDenied:
        return QStringLiteral("PermissionDenied");
    case Code::ProcessFailed:
        return QStringLiteral("ProcessFailed");
    case Code::Timeout:
        return QStringLiteral("Timeout");
    case Code::GitError:
        return QStringLiteral("GitError");
    case Code::IOError:
        return QStringLiteral("IOError");
    case Code::ConfigurationError:
        return QStringLiteral("ConfigurationError");
    default:
        return QStringLiteral("Unknown");
    }
}
