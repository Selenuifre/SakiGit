#ifndef GITOPERATIONRESULT_H
#define GITOPERATIONRESULT_H

#include <QString>
#include <QStringList>

class GitOperationResult
{
public:
    GitOperationResult();

    // --- 基本状态 ---
    bool success = false;
    QString operationType;    // "merge" / "rebase"
    QString targetBranch;     // 操作的目标分支名

    // --- 冲突信息 ---
    bool hasConflicts = false;
    QStringList conflictFiles;

    // --- 摘要信息 ---
    QString message;          // "Fast-forward merge" / "Already up to date" / ...
    QString rawOutput;        // git 命令原始标准输出
    QString rawError;         // git 命令原始标准错误

    // --- 便捷判断 ---
    bool isFastForward() const;
    bool isAlreadyUpToDate() const;

    // --- 辅助方法 ---
    // 生成一行面向用户的中文摘要
    QString summary() const;

    // 判断结果是否有效（至少设置了 operationType）
    bool isValid() const;
};

#endif // GITOPERATIONRESULT_H
