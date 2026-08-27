#include "CredentialStore.h"

#include "services/settingsservice.h"

CredentialStore::CredentialStore(SettingsService* settingsService)
    : m_settings(settingsService)
{
}

// ---- 平台感知方法 ----

Result<void> CredentialStore::saveToken(CodeHostingPlatform platform, const QString& token) const
{
    if (!m_settings) {
        return Result<void>::failure(QStringLiteral("SettingsService is not initialized."));
    }

    const QString encoded = encodeToken(token.trimmed());
    m_settings->setValue(settingsKey(platform), encoded);
    m_settings->sync();

    return Result<void>::success();
}

Result<QString> CredentialStore::loadToken(CodeHostingPlatform platform) const
{
    if (!m_settings) {
        return Result<QString>::failure(QStringLiteral("SettingsService is not initialized."));
    }

    const QString encoded = m_settings->value(settingsKey(platform)).toString();
    if (encoded.isEmpty()) {
        return Result<QString>::failure(QStringLiteral("No saved token found."));
    }

    return Result<QString>::success(decodeToken(encoded));
}

Result<void> CredentialStore::deleteToken(CodeHostingPlatform platform) const
{
    if (!m_settings) {
        return Result<void>::failure(QStringLiteral("SettingsService is not initialized."));
    }

    m_settings->remove(settingsKey(platform));
    m_settings->sync();

    return Result<void>::success();
}

Result<bool> CredentialStore::hasToken(CodeHostingPlatform platform) const
{
    if (!m_settings) {
        return Result<bool>::failure(QStringLiteral("SettingsService is not initialized."));
    }

    const QString encoded = m_settings->value(settingsKey(platform)).toString();
    return Result<bool>::success(!encoded.isEmpty());
}

// ---- 向后兼容（默认 GitHub 平台） ----

Result<void> CredentialStore::saveToken(const QString& token) const
{
    return saveToken(CodeHostingPlatform::GitHub, token);
}

Result<QString> CredentialStore::loadToken() const
{
    return loadToken(CodeHostingPlatform::GitHub);
}

Result<void> CredentialStore::deleteToken() const
{
    return deleteToken(CodeHostingPlatform::GitHub);
}

Result<bool> CredentialStore::hasToken() const
{
    return hasToken(CodeHostingPlatform::GitHub);
}

// ---- 内部方法 ----

QString CredentialStore::settingsKey(CodeHostingPlatform platform) const
{
    switch (platform) {
    case CodeHostingPlatform::GitHub:
        return QStringLiteral("github/token");
    case CodeHostingPlatform::Gitee:
        return QStringLiteral("gitee/token");
    case CodeHostingPlatform::GitLab:
        return QStringLiteral("gitlab/token");
    default:
        return QStringLiteral("unknown/token");
    }
}

QString CredentialStore::encodeToken(const QString& token) const
{
    return token.toUtf8().toBase64();
}

QString CredentialStore::decodeToken(const QString& encoded) const
{
    return QString::fromUtf8(QByteArray::fromBase64(encoded.toUtf8()));
}
