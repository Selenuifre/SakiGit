#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "domain/CodeHostingPlatform.h"

#include <QDateTime>
#include <QMetaType>
#include <QString>

class Account
{
public:
    Account();

    QString login() const;
    void setLogin(const QString& login);

    QString avatarUrl() const;
    void setAvatarUrl(const QString& avatarUrl);

    QString token() const;
    void setToken(const QString& token);

    CodeHostingPlatform platform() const;
    void setPlatform(CodeHostingPlatform platform);

    bool isLoggedIn() const;
    void setLoggedIn(bool loggedIn);

    QDateTime lastLoginAt() const;
    void setLastLoginAt(const QDateTime& lastLoginAt);

    bool isValid() const;

    QString displayName() const;
    QString platformDisplayName() const;   // 例如 "GitHub: dev1"

    bool operator==(const Account& other) const;
    bool operator!=(const Account& other) const;

private:
    QString m_login;
    QString m_avatarUrl;
    QString m_token;
    CodeHostingPlatform m_platform;
    bool m_loggedIn;
    QDateTime m_lastLoginAt;
};

Q_DECLARE_METATYPE(Account)

#endif // ACCOUNT_H
