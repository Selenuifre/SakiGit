#ifndef REBASECONTROLLER_H
#define REBASECONTROLLER_H

#include "GitOperationController.h"
#include "domain/GitOperationResult.h"
#include "infrastructure/result.h"

#include <QString>

class GitTaskRunner;
class RebaseService;

class RebaseController : public GitOperationController
{
    Q_OBJECT

public:
    explicit RebaseController(RebaseService* rebaseService,
                               GitTaskRunner* taskRunner,
                               QObject* parent = nullptr);

    // 检查是否为干净的变基环境
    void checkPreconditions(const QString& repoPath);

signals:
    // 变基操作完成
    void rebaseCompleted(const GitOperationResult& result);

    // 变基发生冲突
    void rebaseConflict(const GitOperationResult& result);

    // 变基失败
    void rebaseFailed(const QString& errorMessage);

public slots:
    // 执行变基
    void performRebase(const QString& repoPath, const QString& branchName);

    // 继续变基（冲突解决后）
    void continueRebase(const QString& repoPath);

    // 中止变基
    void abortRebase(const QString& repoPath);

private:
    RebaseService* m_rebaseService;
    GitTaskRunner* m_taskRunner;
};

#endif // REBASECONTROLLER_H
