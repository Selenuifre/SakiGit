#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include "domain/Account.h"
#include "domain/CodeHostingPlatform.h"
#include "infrastructure/result.h"

#include <QString>

class CredentialStore;
class ICodeHostingProvider;

// 认证服务——管理登录/登出/恢复登录。
// 支持多平台（GitHub / Gitee / GitLab），通过 ICodeHostingProvider 接口调用对应平台的 API。
class AuthService
{
public:
    explicit AuthService(CredentialStore* credentialStore);

    // 使用 token 登录指定平台，返回 Account 信息
    Result<Account> login(ICodeHostingProvider* provider,
                          const QString& token) const;

    // 登出并清除已保存的 token
    Result<void> logout(ICodeHostingProvider* provider) const;

    // 尝试恢复之前保存的登录状态（需要指定平台）
    Result<Account> restoreLogin(ICodeHostingProvider* provider) const;

    // 是否已登录
    bool isLoggedIn() const;

    // 当前登录账户
    Account currentAccount() const;
    void setCurrentAccount(const Account& account);

private:
    CredentialStore* m_credentialStore;
    Account m_currentAccount;
};

#endif // AUTHSERVICE_H
