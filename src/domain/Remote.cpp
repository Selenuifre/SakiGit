#include "Remote.h"

Remote::Remote()
{
}

Remote::Remote(const QString& name)
    : m_name(name.trimmed())
{
}

Remote::Remote(const QString& name, const QString& url)
    : m_name(name.trimmed()),
    m_url(url.trimmed())
{
}

QString Remote::name() const
{
    return m_name;
}

void Remote::setName(const QString& name)
{
    m_name = name.trimmed();
}

QString Remote::url() const
{
    return m_url;
}

void Remote::setUrl(const QString& url)
{
    m_url = url.trimmed();
}

QString Remote::pushUrl() const
{
    return m_pushUrl;
}

void Remote::setPushUrl(const QString& pushUrl)
{
    m_pushUrl = pushUrl.trimmed();
}

bool Remote::isValid() const
{
    return !m_name.isEmpty();
}

QString Remote::displayName() const
{
    if (m_name.isEmpty()) {
        return QStringLiteral("Unnamed remote");
    }

    return m_name;
}

QString Remote::displayUrl() const
{
    if (m_url.isEmpty()) {
        return QStringLiteral("(no URL)");
    }

    return m_url;
}

bool Remote::operator==(const Remote& other) const
{
    return m_name == other.m_name;
}

bool Remote::operator!=(const Remote& other) const
{
    return !(*this == other);
}
