#ifndef MERGECONTROLLER_H
#define MERGECONTROLLER_H

#include "GitOperationController.h"
#include "domain/GitOperationResult.h"
#include "infrastructure/result.h"

#include <QString>

class GitTaskRunner;
class MergeService;

class MergeController : public GitOperationController
{
    Q_OBJECT

public:
    explicit MergeController(MergeService* mergeService,
                              GitTaskRunner* taskRunner,
                              QObject* parent = nullptr);

    // 检查是否为干净的合并环境
    void checkPreconditions(const QString& repoPath);

signals:
    // 合并操作完成
    void mergeCompleted(const GitOperationResult& result);

    // 合并发生冲突
    void mergeConflict(const GitOperationResult& result);

    // 合并失败
    void mergeFailed(const QString& errorMessage);

public slots:
    // 执行合并（异步）
    void performMerge(const QString& repoPath, const QString& branchName);

    // 中止合并（异步）
    void abortMerge(const QString& repoPath);

private:
    MergeService* m_mergeService;
    GitTaskRunner* m_taskRunner;
};

#endif // MERGECONTROLLER_H
