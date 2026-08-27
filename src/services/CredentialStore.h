#ifndef CREDENTIALSTORE_H
#define CREDENTIALSTORE_H

#include "domain/CodeHostingPlatform.h"
#include "infrastructure/result.h"

#include <QString>

class SettingsService;

// 凭据存储——安全保存/读取各平台 Personal Access Token。
// 使用 QSettings 基础存储，token 经过 Base64 混淆（非强加密，本地工具级别保护）。
// 支持 GitHub / Gitee / GitLab 等多平台的独立凭据存储。
class CredentialStore
{
public:
    explicit CredentialStore(SettingsService* settingsService);

    // 保存 token（指定平台）
    Result<void> saveToken(CodeHostingPlatform platform, const QString& token) const;

    // 读取 token（指定平台）
    Result<QString> loadToken(CodeHostingPlatform platform) const;

    // 删除 token（指定平台）
    Result<void> deleteToken(CodeHostingPlatform platform) const;

    // 是否存在已保存的 token（指定平台）
    Result<bool> hasToken(CodeHostingPlatform platform) const;

    // === 向后兼容（默认使用 GitHub 平台） ===
    Result<void> saveToken(const QString& token) const;
    Result<QString> loadToken() const;
    Result<void> deleteToken() const;
    Result<bool> hasToken() const;

private:
    QString settingsKey(CodeHostingPlatform platform) const;
    QString encodeToken(const QString& token) const;
    QString decodeToken(const QString& encoded) const;

    SettingsService* m_settings;
};

#endif // CREDENTIALSTORE_H
