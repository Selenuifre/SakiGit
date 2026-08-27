#ifndef MERGEFILECONTENT_H
#define MERGEFILECONTENT_H

#include <QString>

// 三方合并文件内容：base（共同祖先）、ours（本地/HEAD）、theirs（远程/合并来源）
class MergeFileContent
{
public:
    MergeFileContent();

    QString filePath;     // 冲突文件路径
    QString baseContent;  // 共同祖先版本的内容
    QString oursContent;  // 本地版本（HEAD）的内容
    QString theirsContent;// 远程版本（合并来源）的内容

    bool isValid() const;
    bool hasAllVersions() const;
};

#endif // MERGEFILECONTENT_H
