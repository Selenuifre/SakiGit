#ifndef FILECHANGEMODEL_H
#define FILECHANGEMODEL_H

#include "domain/filechange.h"

#include <QAbstractTableModel>
#include <QHash>
#include <QModelIndex>
#include <QVariant>
#include <vector>

// 文件变更列表模型——将 std::vector<FileChange> 包装为 QAbstractTableModel，
// 供 ChangesPage 以三列表格（File | Status | Staged）展示工作区文件变更。
class FileChangeModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    // 规范要求的自定义数据角色
    enum Role {
        FilePathRole = Qt::UserRole + 1, // 文件路径
        StatusRole,                      // 文件变更状态枚举值
        StagedRole,                      // 是否已暂存 (bool)

        // ---- 扩展角色 ----
        FileChangeRole,      // 完整 FileChange 对象
        OldPathRole,         // 重命名前的旧路径
        DisplayPathRole,     // 用于界面展示的路径（含重命名箭头）
        StatusTextRole,      // 文件变更状态文本
        StageStateRole,      // 暂存状态枚举值
        StageStateTextRole,  // 暂存状态文本
        IndexStatusRole,     // Git porcelain 第一列状态码
        WorktreeStatusRole,  // Git porcelain 第二列状态码
        PorcelainCodeRole,   // Git porcelain 两字符状态码
        UnstagedRole,        // 是否未暂存 (bool)
        PartiallyStagedRole, // 是否部分暂存 (bool)
        ConflictRole,        // 是否冲突 (bool)
        DeletedRole,         // 是否已删除 (bool)
        RenamedRole,         // 是否已重命名 (bool)
        UntrackedRole,       // 是否未追踪 (bool)
        HasOldPathRole,      // 是否有旧路径 (bool)
        ValidRole            // 是否为有效变更项 (bool)
    };

    // 表格三列定义
    enum Column {
        FilePathColumn = 0, // 文件路径列
        StatusColumn,       // 状态列
        StagedColumn,       // 暂存状态列
        ColumnCount         // 总列数
    };

public:
    explicit FileChangeModel(QObject* parent = nullptr);

    // ---- QAbstractTableModel 接口 ----
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QHash<int, QByteArray> roleNames() const override;

    // ---- 规范要求的方法 ----

    // 整体替换文件变更列表
    void setChanges(const std::vector<FileChange>& changes);

    // 获取指定行的文件变更；行无效时返回空对象
    FileChange changeAt(int row) const;

    // 获取指定行的文件路径
    QString filePathAt(int row) const;

    // 清空文件变更列表
    void clear();

    // ---- 扩展方法 ----

    // 返回当前所有文件变更
    std::vector<FileChange> fileChanges() const;

    // 整体替换文件变更列表（与 setChanges 等价）
    void setFileChanges(const std::vector<FileChange>& fileChanges);

    bool isEmpty() const;  // 判断列表是否为空
    int count() const;     // 返回文件变更数量

    FileChange fileChangeAt(int row) const;               // 获取指定行
    FileChange fileChangeForPath(const QString& path) const; // 按路径查找
    QModelIndex indexForPath(const QString& path) const;    // 按路径获取模型索引
    int indexOfPath(const QString& path) const;             // 按路径获取行号
    bool containsPath(const QString& path) const;           // 是否包含指定路径

    void addFileChange(const FileChange& fileChange);          // 添加；路径重复则更新
    bool updateFileChange(const FileChange& fileChange);       // 按路径更新
    bool upsertFileChange(const FileChange& fileChange);       // 新增或更新，更新返回 true

    bool removeFileChangeAt(int row);             // 按行号移除
    bool removeFileChange(const QString& path);   // 按路径移除

    std::vector<FileChange> stagedChanges() const;     // 所有已暂存变更
    std::vector<FileChange> unstagedChanges() const;   // 所有未暂存变更
    std::vector<FileChange> conflictedChanges() const; // 所有冲突变更

    int stagedCount() const;      // 已暂存数量
    int unstagedCount() const;    // 未暂存数量
    int conflictedCount() const;  // 冲突数量

signals:
    void fileChangesChanged();                          // 列表结构或内容变化
    void fileChangeAdded(const FileChange& fileChange); // 新变更添加
    void fileChangeUpdated(const FileChange& fileChange);// 变更内容更新
    void fileChangeRemoved(const QString& path);        // 变更移除

private:
    bool isValidRow(int row) const;            // 判断行号是否有效
    void emitFileChangeChanged(int row);       // 通知指定行所有列已变化

private:
    std::vector<FileChange> m_fileChanges; // 当前持有的文件变更列表
};

#endif // FILECHANGEMODEL_H
