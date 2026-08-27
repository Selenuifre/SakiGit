#include "AuthService.h"

#include "CredentialStore.h"
#include "infrastructure/ICodeHostingProvider.h"

AuthService::AuthService(CredentialStore* credentialStore)
    : m_credentialStore(credentialStore)
{
}

Result<Account> AuthService::login(ICodeHostingProvider* provider,
                                    const QString& token) const
{
    if (!provider) {
        return Result<Account>::failure(QStringLiteral("Provider is not initialized."));
    }

    if (token.trimmed().isEmpty()) {
        return Result<Account>::failure(QStringLiteral("Token cannot be empty."));
    }

    qDebug() << "[Auth] Attempting login on" << codeHostingPlatformName(provider->platform()) << "...";

    // 设置 token 并用 /user 接口验证
    provider->setAuthToken(token.trimmed());

    const Result<QJsonObject> userResult = provider->getUserInfo();

    if (userResult.isFailure()) {
        qDebug() << "[Auth] Login failed:" << userResult.errorMessage();
        return Result<Account>::failure(userResult.errorMessage());
    }

    const int statusCode = provider->lastStatusCode();
    if (statusCode == 401 || statusCode == 403) {
        qDebug() << "[Auth] Invalid token (HTTP" << statusCode << ")";
        return Result<Account>::failure(
            QStringLiteral("Invalid token. Please check your %1 Personal Access Token.")
                .arg(codeHostingPlatformName(provider->platform())));
    }

    const QJsonObject userObj = userResult.value();

    Account account;
    account.setPlatform(provider->platform());
    account.setToken(token.trimmed());
    account.setLoggedIn(true);
    account.setLastLoginAt(QDateTime::currentDateTime());

    // 各平台的用户名字段可能不同
    if (provider->platform() == CodeHostingPlatform::GitLab) {
        account.setLogin(userObj.value(QStringLiteral("username")).toString());
        account.setAvatarUrl(userObj.value(QStringLiteral("avatar_url")).toString());
    } else {
        // GitHub / Gitee 使用 "login"
        account.setLogin(userObj.value(QStringLiteral("login")).toString());
        account.setAvatarUrl(userObj.value(QStringLiteral("avatar_url")).toString());
    }

    qDebug() << "[Auth] Login succeeded — platform:"
             << codeHostingPlatformName(account.platform())
             << "user:" << account.login();

    // 登录成功，保存 token
    if (m_credentialStore) {
        const Result<void> saveResult = m_credentialStore->saveToken(
            provider->platform(), token.trimmed());
        if (saveResult.isFailure()) {
            qDebug() << "[Auth] Warning: token save failed:" << saveResult.errorMessage();
        } else {
            qDebug() << "[Auth] Token saved for platform:"
                     << codeHostingPlatformName(provider->platform());
        }
    }

    return Result<Account>::success(account);
}

Result<void> AuthService::logout(ICodeHostingProvider* provider) const
{
    if (provider && m_credentialStore) {
        const Result<void> deleteResult = m_credentialStore->deleteToken(provider->platform());
        if (deleteResult.isFailure()) {
            return deleteResult;
        }
    }

    if (provider) {
        provider->setAuthToken(QString());
    }

    return Result<void>::success();
}

Result<Account> AuthService::restoreLogin(ICodeHostingProvider* provider) const
{
    if (!provider) {
        return Result<Account>::failure(QStringLiteral("Provider is not initialized."));
    }

    if (!m_credentialStore) {
        return Result<Account>::failure(QStringLiteral("CredentialStore is not initialized."));
    }

    const Result<bool> hasTokenResult = m_credentialStore->hasToken(provider->platform());

    if (hasTokenResult.isFailure()) {
        return Result<Account>::failure(hasTokenResult.errorMessage());
    }

    if (!hasTokenResult.value()) {
        return Result<Account>::failure(QStringLiteral("No saved credentials for %1.")
                                            .arg(codeHostingPlatformName(provider->platform())));
    }

    const Result<QString> tokenResult = m_credentialStore->loadToken(provider->platform());

    if (tokenResult.isFailure()) {
        return Result<Account>::failure(tokenResult.errorMessage());
    }

    return login(provider, tokenResult.value());
}

bool AuthService::isLoggedIn() const
{
    return m_currentAccount.isLoggedIn();
}

Account AuthService::currentAccount() const
{
    return m_currentAccount;
}

void AuthService::setCurrentAccount(const Account& account)
{
    m_currentAccount = account;
}
