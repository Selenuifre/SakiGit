#ifndef AUTHCONTROLLER_H
#define AUTHCONTROLLER_H

#include "BaseController.h"
#include "domain/Account.h"
#include "infrastructure/result.h"

#include <QString>

class AuthService;
class ICodeHostingProvider;

class AuthController : public BaseController
{
    Q_OBJECT

public:
    explicit AuthController(AuthService* authService,
                            QObject* parent = nullptr);

    // 登录指定平台
    Result<Account> login(ICodeHostingProvider* provider, const QString& token);
    Result<void> logout(ICodeHostingProvider* provider);
    Result<Account> restoreLogin(ICodeHostingProvider* provider);
    bool isLoggedIn() const;
    Account currentAccount() const;

signals:
    void loginSucceeded(const Account& account);
    void loginFailed(const QString& errorMessage);
    void logoutCompleted();

private:
    AuthService* m_authService;
    Account m_currentAccount;
};

#endif // AUTHCONTROLLER_H
