#include "Account.h"

Account::Account()
    : m_platform(CodeHostingPlatform::Unknown),
    m_loggedIn(false)
{
}

QString Account::login() const
{
    return m_login;
}

void Account::setLogin(const QString& login)
{
    m_login = login.trimmed();
}

QString Account::avatarUrl() const
{
    return m_avatarUrl;
}

void Account::setAvatarUrl(const QString& avatarUrl)
{
    m_avatarUrl = avatarUrl.trimmed();
}

QString Account::token() const
{
    return m_token;
}

void Account::setToken(const QString& token)
{
    m_token = token.trimmed();
}

CodeHostingPlatform Account::platform() const
{
    return m_platform;
}

void Account::setPlatform(CodeHostingPlatform platform)
{
    m_platform = platform;
}

bool Account::isLoggedIn() const
{
    return m_loggedIn;
}

void Account::setLoggedIn(bool loggedIn)
{
    m_loggedIn = loggedIn;
}

QDateTime Account::lastLoginAt() const
{
    return m_lastLoginAt;
}

void Account::setLastLoginAt(const QDateTime& lastLoginAt)
{
    m_lastLoginAt = lastLoginAt;
}

bool Account::isValid() const
{
    return !m_login.isEmpty() && !m_token.isEmpty() && m_platform != CodeHostingPlatform::Unknown;
}

QString Account::displayName() const
{
    if (m_login.isEmpty()) {
        return QStringLiteral("Not logged in");
    }

    return m_login;
}

QString Account::platformDisplayName() const
{
    if (m_login.isEmpty()) {
        return QStringLiteral("Not logged in");
    }

    return QStringLiteral("%1: %2").arg(codeHostingPlatformName(m_platform), m_login);
}

bool Account::operator==(const Account& other) const
{
    return m_login == other.m_login && m_platform == other.m_platform;
}

bool Account::operator!=(const Account& other) const
{
    return !(*this == other);
}
