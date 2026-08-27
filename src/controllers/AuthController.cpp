#include "AuthController.h"

#include "services/AuthService.h"
#include "infrastructure/ICodeHostingProvider.h"

AuthController::AuthController(AuthService* authService,
                               QObject* parent)
    : BaseController(parent),
    m_authService(authService)
{
}

Result<Account> AuthController::login(ICodeHostingProvider* provider,
                                       const QString& token)
{
    qDebug() << "[AuthController] Login requested...";

    if (!m_authService) {
        const QString errorMessage = QStringLiteral("AuthService is not available.");
        emit loginFailed(errorMessage);
        return Result<Account>::failure(errorMessage);
    }

    if (!provider) {
        const QString errorMessage = QStringLiteral("Provider is not available.");
        emit loginFailed(errorMessage);
        return Result<Account>::failure(errorMessage);
    }

    const Result<Account> result = m_authService->login(provider, token);

    if (result.isFailure()) {
        qDebug() << "[AuthController] Login failed:" << result.errorMessage();
        emit loginFailed(result.errorMessage());
        emit errorOccurred(QStringLiteral("login"), result.errorMessage());
        return result;
    }

    m_currentAccount = result.value();
    qDebug() << "[AuthController] Login complete — account:" << m_currentAccount.platformDisplayName();
    emit loginSucceeded(m_currentAccount);
    return result;
}

Result<void> AuthController::logout(ICodeHostingProvider* provider)
{
    if (!m_authService) {
        const QString errorMessage = QStringLiteral("AuthService is not available.");
        emit errorOccurred(QStringLiteral("logout"), errorMessage);
        return Result<void>::failure(errorMessage);
    }

    const Result<void> result = m_authService->logout(provider);

    if (result.isFailure()) {
        emit errorOccurred(QStringLiteral("logout"), result.errorMessage());
        return result;
    }

    m_currentAccount = Account();
    emit logoutCompleted();
    return Result<void>::success();
}

Result<Account> AuthController::restoreLogin(ICodeHostingProvider* provider)
{
    qDebug() << "[AuthController] Restoring saved login...";

    if (!m_authService) {
        return Result<Account>::failure(QStringLiteral("AuthService is not available."));
    }

    const Result<Account> result = m_authService->restoreLogin(provider);

    if (result.isSuccess()) {
        m_currentAccount = result.value();
        qDebug() << "[AuthController] Restored login — user:"
                 << m_currentAccount.platformDisplayName();
        emit loginSucceeded(m_currentAccount);
    } else {
        qDebug() << "[AuthController] No saved login:" << result.errorMessage();
    }

    return result;
}

bool AuthController::isLoggedIn() const
{
    return m_currentAccount.isLoggedIn();
}

Account AuthController::currentAccount() const
{
    return m_currentAccount;
}
