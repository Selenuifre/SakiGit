#ifndef FILECHANGE_H
#define FILECHANGE_H

#include <QMetaType>
#include <QString>

#include "gittypes.h"
#include "Operators.h"

class FileChange : public EqualityOperators<FileChange>
{
public:
    // 直接复用 GitTypes 命名空间中的统一类型定义，不再重复声明枚举
    using Status = GitTypes::FileStatus;
    using StageState = GitTypes::StageState;

public:
    FileChange();
    explicit FileChange(const QString& path);
    FileChange(const QString& path, Status status);
    FileChange(const QString& path, Status status, StageState stageState);

    QString path() const;
    void setPath(const QString& path);

    QString oldPath() const;
    void setOldPath(const QString& oldPath);

    Status status() const;
    void setStatus(Status status);

    StageState stageState() const;
    void setStageState(StageState stageState);

    QString indexStatus() const;
    void setIndexStatus(const QString& indexStatus);

    QString worktreeStatus() const;
    void setWorktreeStatus(const QString& worktreeStatus);

    bool isValid() const;
    bool isStaged() const;
    bool isUnstaged() const;
    bool isPartiallyStaged() const;
    bool isConflict() const;
    bool isDeleted() const;
    bool isRenamed() const;
    bool isUntracked() const;
    bool hasOldPath() const;

    QString displayPath() const;
    QString statusText() const;
    QString stageStateText() const;
    QString porcelainCode() const;

    bool operator==(const FileChange& other) const;

private:
    QString m_path;
    //当前文件的路径
    QString m_oldPath;
    //重命名前的旧路径

    Status m_status;
    //文件变化类型：新增、修改、删除、重命名等
    StageState m_stageState;
    //暂存状态：已暂存、未暂存、部分暂存、冲突

    QString m_indexStatus;
    //Git porcelain 中第一列状态
    QString m_worktreeStatus;
    //Git porcelain 中第二列状态
};

Q_DECLARE_METATYPE(FileChange)

#endif // FILECHANGE_H
