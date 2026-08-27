#ifndef RESULTUTILS_H
#define RESULTUTILS_H

#include "infrastructure/gitcommandexecutor.h"
#include "infrastructure/result.h"

// 共享错误消息常量
namespace ServiceError {
    constexpr auto NotInitialized = "GitService is not initialized.";
    constexpr auto RepositoryPathEmpty = "Repository path cannot be empty.";
    constexpr auto BranchNameEmpty = "Branch name cannot be empty.";
} // namespace ServiceError

// 将 CommandResult 的 Result 转换为 Result<void>，
// 提取自 gitservice.cpp 匿名命名空间，供所有 Service 复用。
inline Result<void> toVoidResult(const Result<GitCommandExecutor::CommandResult>& result)
{
    if (result.isFailure()) {
        return Result<void>::failure(result.errorMessage());
    }

    if (!result.value().isSuccess()) {
        // 在错误消息中附带 exit code 便于调试
        const auto& cmdResult = result.value();
        const QString message = QStringLiteral("git exited with code %1: %2")
            .arg(cmdResult.exitCode)
            .arg(cmdResult.message());
        return Result<void>::failure(message);
    }

    return Result<void>::success();
}

#endif // RESULTUTILS_H
