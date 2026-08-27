#ifndef CONFLICTFILE_H
#define CONFLICTFILE_H

#include <QString>

// 表示一个处于冲突状态的文件
class ConflictFile
{
public:
    ConflictFile();
    explicit ConflictFile(const QString& path);

    QString path() const;
    void setPath(const QString& path);

    // 冲突类型：merge 产生的冲突 或 rebase 产生的冲突
    QString conflictType() const;   // "merge" / "rebase"
    void setConflictType(const QString& type);

    // 文件是否存在于工作区
    bool exists() const;
    void setExists(bool exists);

    bool isValid() const;

private:
    QString m_path;
    QString m_conflictType;
    bool m_exists = true;
};

#endif // CONFLICTFILE_H
