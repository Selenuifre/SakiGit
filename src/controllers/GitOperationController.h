#ifndef GITOPERATIONCONTROLLER_H
#define GITOPERATIONCONTROLLER_H

#include "BaseController.h"
#include "domain/GitOperationResult.h"
#include "infrastructure/result.h"

#include <QString>
#include <QtConcurrent/QtConcurrentRun>

// Merge / Rebase 控制器的公共基类。
// 提供异步执行模板和共享的 preconditionsChecked / operationStarted 信号。

class GitOperationController : public BaseController
{
    Q_OBJECT

public:
    explicit GitOperationController(QObject* parent = nullptr)
        : BaseController(parent)
    {
    }

signals:
    // 操作开始（用于 UI 设置 busy 状态）
    void operationStarted(const QString& operation);

    // 操作完成（异步操作的轻量通知，不含 success/errorMessage）
    void operationFinished(const QString& operation);

    // 前置条件检查完成
    void preconditionsChecked(bool canProceed, const QString& warning);

protected:
    // 异步执行 Result<GitOperationResult> 操作（如 merge/rebase）
    // 自动处理 operationStarted/operationFinished 信号和错误/冲突分支。
    template <typename Service, typename ServiceCall>
    void executeGitOperation(
        const QString& opName,
        const QString& repoPath,
        Service* service,
        ServiceCall call,
        std::function<void(const GitOperationResult&)> onSuccess,
        std::function<void(const GitOperationResult&)> onConflict,
        std::function<void(const QString&)> onFailure)
    {
        if (!service) {
            emit errorOccurred(opName,
                QStringLiteral("Service is not available."));
            if (onFailure)
                onFailure(QStringLiteral("Service is not available."));
            return;
        }

        emit operationStarted(opName);

        Service* svc = service;
        const QString repo = repoPath;

        QtConcurrent::run([svc, repo, call]() {
            return call(svc, repo);
        }).then([this, opName, onSuccess, onConflict, onFailure](
            const Result<GitOperationResult>& result)
        {
            emit operationFinished(opName);

            if (result.isFailure()) {
                emit errorOccurred(opName, result.errorMessage());
                if (onFailure)
                    onFailure(result.errorMessage());
                return;
            }

            const GitOperationResult& opResult = result.value();
            if (opResult.hasConflicts) {
                if (onConflict)
                    onConflict(opResult);
            } else if (opResult.success) {
                if (onSuccess)
                    onSuccess(opResult);
            } else {
                emit errorOccurred(opName, opResult.message);
                if (onFailure)
                    onFailure(opResult.message);
            }
        });
    }

    // 异步执行 Result<void> 操作（如 abort/continue）
    template <typename Service, typename ServiceCall>
    void executeVoidOperation(
        const QString& opName,
        const QString& repoPath,
        Service* service,
        ServiceCall call,
        const QString& successMessage,
        const QString& operationType)
    {
        if (!service) {
            emit errorOccurred(opName,
                QStringLiteral("Service is not available."));
            return;
        }

        emit operationStarted(opName);

        Service* svc = service;
        const QString repo = repoPath;

        QtConcurrent::run([svc, repo, call]() {
            return call(svc, repo);
        }).then([this, opName, successMessage, operationType](
            const Result<void>& result)
        {
            emit operationFinished(opName);

            if (result.isFailure()) {
                emit errorOccurred(opName, result.errorMessage());
                return;
            }

            // 构造成功结果通知调用者
            GitOperationResult opResult;
            opResult.operationType = operationType;
            opResult.success = true;
            opResult.message = successMessage;
            // 子类应覆盖 emitXxxCompleted 来发出特定信号
        });
    }
};

#endif // GITOPERATIONCONTROLLER_H
