#ifndef DIFF_H
#define DIFF_H

#include "gittypes.h"

#include <QList>
#include <QMetaType>
#include <QString>

class DiffLine
{
public:
    // 创建空 diff 行
    DiffLine();

    // 使用行类型和文本创建 diff 行
    DiffLine(GitTypes::DiffLineType type, const QString& text);

    // 使用完整信息创建 diff 行
    DiffLine(GitTypes::DiffLineType type, const QString& text, int oldLineNumber, int newLineNumber);

    // 返回行类型
    GitTypes::DiffLineType type() const;

    // 设置行类型
    void setType(GitTypes::DiffLineType type);

    // 返回行文本
    QString text() const;

    // 设置行文本
    void setText(const QString& text);

    // 返回旧文件中的行号
    int oldLineNumber() const;

    // 设置旧文件中的行号
    void setOldLineNumber(int oldLineNumber);

    // 返回新文件中的行号
    int newLineNumber() const;

    // 设置新文件中的行号
    void setNewLineNumber(int newLineNumber);

    // 判断是否是新增行
    bool isAdded() const;

    // 判断是否是删除行
    bool isRemoved() const;

    // 判断是否是上下文行
    bool isContext() const;

    // 判断是否是 hunk 头部行
    bool isHunkHeader() const;

    // 返回用于显示的前缀字符
    QString displayPrefix() const;

private:
    // 当前 diff 行类型
    GitTypes::DiffLineType m_type;

    // 当前 diff 行文本
    QString m_text;

    // 当前行在旧文件中的行号，-1 表示没有
    int m_oldLineNumber;

    // 当前行在新文件中的行号，-1 表示没有
    int m_newLineNumber;
};

class DiffHunk
{
public:
    // 创建空 hunk
    DiffHunk();

    // 使用 hunk 头部创建 hunk
    explicit DiffHunk(const QString& header);

    // 返回 hunk 头部文本
    QString header() const;

    // 设置 hunk 头部文本
    void setHeader(const QString& header);

    // 返回旧文件起始行号
    int oldStart() const;

    // 设置旧文件起始行号
    void setOldStart(int oldStart);

    // 返回旧文件影响行数
    int oldLineCount() const;

    // 设置旧文件影响行数
    void setOldLineCount(int oldLineCount);

    // 返回新文件起始行号
    int newStart() const;

    // 设置新文件起始行号
    void setNewStart(int newStart);

    // 返回新文件影响行数
    int newLineCount() const;

    // 设置新文件影响行数
    void setNewLineCount(int newLineCount);

    // 返回 hunk 内的所有 diff 行
    QList<DiffLine> lines() const;

    // 设置 hunk 内的所有 diff 行
    void setLines(const QList<DiffLine>& lines);

    // 添加一行 diff
    void addLine(const DiffLine& line);

    // 判断 hunk 是否有效
    bool isValid() const;

    // 返回新增行数量
    int addedLineCount() const;

    // 返回删除行数量
    int removedLineCount() const;

    // 从 hunk 头部解析行号信息
    void parseHeader();

    // 从 hunk 头部解析出 hunk 对象
    static DiffHunk fromHeader(const QString& header);

private:
    // hunk 头部，例如 @@ -1,3 +1,4 @@
    QString m_header;

    // 旧文件起始行号
    int m_oldStart;

    // 旧文件影响行数
    int m_oldLineCount;

    // 新文件起始行号
    int m_newStart;

    // 新文件影响行数
    int m_newLineCount;

    // hunk 内的行级 diff
    QList<DiffLine> m_lines;
};

class FileDiff
{
public:
    // 创建空文件 diff
    FileDiff();

    // 使用新文件路径创建文件 diff
    explicit FileDiff(const QString& path);

    // 返回旧文件路径
    QString oldPath() const;

    // 设置旧文件路径
    void setOldPath(const QString& oldPath);

    // 返回新文件路径
    QString newPath() const;

    // 设置新文件路径
    void setNewPath(const QString& newPath);

    // 返回 UI 中显示的文件路径
    QString displayPath() const;

    // 判断是否是二进制文件 diff
    bool isBinary() const;

    // 设置是否是二进制文件 diff
    void setBinary(bool binary);

    // 判断是否是新文件
    bool isNewFile() const;

    // 设置是否是新文件
    void setNewFile(bool newFile);

    // 判断是否是删除文件
    bool isDeletedFile() const;

    // 设置是否是删除文件
    void setDeletedFile(bool deletedFile);

    // 判断是否是重命名文件
    bool isRenamed() const;

    // 设置是否是重命名文件
    void setRenamed(bool renamed);

    // 返回该文件的所有 hunk
    QList<DiffHunk> hunks() const;

    // 设置该文件的所有 hunk
    void setHunks(const QList<DiffHunk>& hunks);

    // 添加一个 hunk
    void addHunk(const DiffHunk& hunk);

    // 判断文件 diff 是否有效
    bool isValid() const;

    // 返回新增行数量
    int addedLineCount() const;

    // 返回删除行数量
    int removedLineCount() const;

    // 清理 git diff 路径前缀
    static QString cleanGitPath(const QString& path);

private:
    // 旧文件路径
    QString m_oldPath;

    // 新文件路径
    QString m_newPath;

    // 是否是二进制文件
    bool m_binary;

    // 是否是新文件
    bool m_newFile;

    // 是否是删除文件
    bool m_deletedFile;

    // 是否是重命名文件
    bool m_renamed;

    // 文件内的 hunk 列表
    QList<DiffHunk> m_hunks;
};

class Diff
{
public:
    // 创建空 diff
    Diff();

    // 使用原始 diff 文本创建 diff
    explicit Diff(const QString& rawText);

    // 返回原始 diff 文本
    QString rawText() const;

    // 设置原始 diff 文本
    void setRawText(const QString& rawText);

    // 返回所有文件 diff
    QList<FileDiff> files() const;

    // 设置所有文件 diff
    void setFiles(const QList<FileDiff>& files);

    // 添加一个文件 diff
    void addFile(const FileDiff& fileDiff);

    // 清空 diff 内容
    void clear();

    // 判断 diff 是否为空
    bool isEmpty() const;

    // 返回文件数量
    int fileCount() const;

    // 返回总新增行数
    int addedLineCount() const;

    // 返回总删除行数
    int removedLineCount() const;

    // 从 unified diff 文本解析 Diff 对象
    static Diff fromUnifiedDiff(const QString& text);

private:
    // git diff 的原始文本
    QString m_rawText;

    // 文件级 diff 列表
    QList<FileDiff> m_files;
};

Q_DECLARE_METATYPE(DiffLine)
Q_DECLARE_METATYPE(DiffHunk)
Q_DECLARE_METATYPE(FileDiff)
Q_DECLARE_METATYPE(Diff)

#endif // DIFF_H
