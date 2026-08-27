#ifndef CONFLICTCONTROLLER_H
#define CONFLICTCONTROLLER_H

#include "domain/ConflictFile.h"
#include "infrastructure/result.h"

#include "BaseController.h"
#include <QString>
#include <vector>

class ConflictFileModel;
class ConflictService;
class GitTaskRunner;

class ConflictController : public BaseController
{
    Q_OBJECT

public:
    explicit ConflictController(ConflictService* conflictService,
                                 ConflictFileModel* conflictFileModel,
                                 GitTaskRunner* taskRunner,
                                 QObject* parent = nullptr);

    // 加载冲突文件列表
    void loadConflicts(const QString& repoPath);

    // 接受 ours / theirs 版本
    Result<void> acceptOurs(const QString& repoPath, const QString& filePath);
    Result<void> acceptTheirs(const QString& repoPath, const QString& filePath);

    // 标记文件为已解决（git add）
    Result<void> markResolved(const QString& repoPath, const QString& filePath);

    // 检查是否有冲突
    Result<bool> hasConflicts(const QString& repoPath) const;

    ConflictFileModel* conflictFileModel() const;

    void clear();

signals:
    void conflictsLoaded(bool success, const QString& errorMessage);
    void conflictResolved(const QString& filePath);

private:
    ConflictService* m_conflictService;
    ConflictFileModel* m_conflictFileModel;
    GitTaskRunner* m_taskRunner;
};

#endif // CONFLICTCONTROLLER_H
