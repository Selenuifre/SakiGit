#ifndef COMMITHISTORYMODEL_H
#define COMMITHISTORYMODEL_H

#include "domain/commit.h"

#include <QAbstractTableModel>
#include <QHash>
#include <QModelIndex>
#include <QVariant>
#include <vector>

// 提交历史列表模型——将 std::vector<Commit> 包装为 QAbstractTableModel，
// 供 HistoryPage 以四列表格（Hash | Message | Author | Date）展示提交历史。
class CommitHistoryModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    // 规范要求的自定义数据角色
    enum Role {
        HashRole = Qt::UserRole + 1, // 完整提交哈希
        ShortHashRole,               // 短提交哈希（默认 7 位）
        MessageRole,                 // 完整提交消息
        AuthorRole,                  // 作者显示文本
        DateRole,                    // 日期显示文本

        // ---- 扩展角色 ----
        CommitRole,           // 完整 Commit 对象
        SummaryRole,          // 提交摘要（message 首行）
        BodyRole,             // 提交正文（message 其余部分）
        DisplayTitleRole,     // 用于界面展示的标题
        AuthorNameRole,       // 作者名称
        AuthorEmailRole,      // 作者邮箱
        AuthorDateRole,       // 作者时间 (QDateTime)
        AuthorDisplayRole,    // 用于界面展示的作者
        AuthorDateTextRole,   // 用于界面展示的作者时间
        CommitterNameRole,    // 提交者名称
        CommitterEmailRole,   // 提交者邮箱
        CommitterDateRole,    // 提交者时间 (QDateTime)
        ParentHashesRole,     // 父提交哈希列表 (QStringList)
        ParentCountRole,      // 父提交数量 (int)
        ChangedFilesRole,     // 变更文件列表 (QStringList)
        ChangedFileCountRole, // 变更文件数量 (int)
        MergeCommitRole,      // 是否为合并提交 (bool)
        HasBodyRole,          // 是否包含正文 (bool)
        ValidRole             // 是否为有效提交 (bool)
    };

    // 表格四列定义
    enum Column {
        HashColumn = 0,  // 哈希列
        MessageColumn,   // 提交消息列
        AuthorColumn,    // 作者列
        DateColumn,      // 日期列
        ColumnCount      // 总列数
    };

public:
    explicit CommitHistoryModel(QObject* parent = nullptr);

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

    // 整体替换提交列表
    void setCommits(const std::vector<Commit>& commits);

    // 在末尾批量追加提交（使用 beginInsertRows 高效插入）
    void appendCommits(const std::vector<Commit>& commits);

    // 获取指定行的提交；行无效时返回空对象
    Commit commitAt(int row) const;

    // 清空提交列表
    void clear();

    // ---- 扩展方法 ----

    std::vector<Commit> commits() const; // 返回当前所有提交

    bool isEmpty() const;  // 判断列表是否为空
    int count() const;     // 返回提交数量

    Commit commitForHash(const QString& hash) const;   // 按哈希查找（支持完整/短哈希匹配）
    QModelIndex indexForHash(const QString& hash) const; // 按哈希获取模型索引
    int indexOfHash(const QString& hash) const;          // 按哈希获取行号
    bool containsHash(const QString& hash) const;        // 是否包含指定哈希的提交

    void addCommit(const Commit& commit);          // 添加；哈希重复则更新
    bool updateCommit(const Commit& commit);       // 按哈希更新
    bool upsertCommit(const Commit& commit);       // 新增或更新，更新返回 true

    bool removeCommitAt(int row);              // 按行号移除
    bool removeCommit(const QString& hash);    // 按哈希移除

    std::vector<Commit> mergeCommits() const;                      // 所有合并提交
    std::vector<Commit> commitsTouchingFile(const QString& filePath) const; // 涉及指定文件的提交

    int mergeCommitCount() const;   // 合并提交数量
    Commit latestCommit() const;    // 最近一条提交；列表为空时返回空对象

signals:
    void commitsChanged();                          // 列表结构或内容变化
    void commitAdded(const Commit& commit);         // 新提交添加
    void commitUpdated(const Commit& commit);       // 提交内容更新
    void commitRemoved(const QString& hash);        // 提交移除

private:
    bool isValidRow(int row) const;            // 判断行号是否有效
    void emitCommitChanged(int row);           // 通知指定行所有列已变化

    static QString normalizedHash(const QString& hash);                        // 规范化哈希
    static bool hashMatches(const QString& commitHash, const QString& queryHash); // 哈希模糊匹配
    static QString normalizedFilePath(const QString& filePath);                // 规范化文件路径

private:
    std::vector<Commit> m_commits; // 当前持有的提交列表
};

#endif // COMMITHISTORYMODEL_H
