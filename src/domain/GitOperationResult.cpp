#include "GitOperationResult.h"

GitOperationResult::GitOperationResult() = default;

bool GitOperationResult::isFastForward() const
{
    if (message.contains(QStringLiteral("Fast-forward"), Qt::CaseInsensitive)) {
        return true;
    }
    if (rawOutput.contains(QStringLiteral("Fast-forward"), Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

bool GitOperationResult::isAlreadyUpToDate() const
{
    if (message.contains(QStringLiteral("Already up to date"), Qt::CaseInsensitive)) {
        return true;
    }
    if (rawOutput.contains(QStringLiteral("Already up to date"), Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

QString GitOperationResult::summary() const
{
    if (!success && !hasConflicts) {
        return QStringLiteral("操作失败: ") + message;
    }

    if (hasConflicts) {
        return QStringLiteral("%1 到 %2 时发生冲突，%3 个文件需要手动解决")
            .arg(operationType == QStringLiteral("rebase") ? QStringLiteral("变基") : QStringLiteral("合并"))
            .arg(targetBranch)
            .arg(conflictFiles.size());
    }

    if (isAlreadyUpToDate()) {
        return QStringLiteral("已是最新，无需%1")
            .arg(operationType == QStringLiteral("rebase") ? QStringLiteral("变基") : QStringLiteral("合并"));
    }

    if (isFastForward()) {
        return QStringLiteral("已通过快进将 %1 %2 到当前分支")
            .arg(targetBranch)
            .arg(operationType == QStringLiteral("rebase") ? QStringLiteral("变基") : QStringLiteral("合并"));
    }

    return QStringLiteral("已成功将 %1 %2 到当前分支")
        .arg(targetBranch)
        .arg(operationType == QStringLiteral("rebase") ? QStringLiteral("变基") : QStringLiteral("合并"));
}

bool GitOperationResult::isValid() const
{
    return !operationType.isEmpty();
}
