#ifndef CHANGESPAGE_H
#define CHANGESPAGE_H

#include <QWidget>

class QAbstractItemModel;
class QListView;
class QModelIndex;
class QPoint;
class QSortFilterProxyModel;
class QToolButton;
class CommitPanel;
class ConflictResolver;
class DiffViewer;
class ReviewPanel;
class QSplitter;
class QStackedWidget;

// 文件变更页面——文件列表 + diff 查看器 + 提交面板。
// 规范接口：setModel(FileChangeModel*)
// 信号：fileActivated, stageRequested, unstageRequested, stageAllRequested, unstageAllRequested
class ChangesPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChangesPage(QWidget* parent = nullptr);

    // 设置文件变更列表数据源
    void setModel(QAbstractItemModel* model);

    // 返回文件变更模型
    QAbstractItemModel* model() const;

    // 设置 diff 数据源
    void setDiffModel(QAbstractItemModel* model);

    // 返回 DiffViewer、ConflictResolver、ReviewPanel 和 CommitPanel（供外部访问）
    DiffViewer* diffViewer() const;
    ConflictResolver* conflictResolver() const;
    ReviewPanel* reviewPanel() const;
    CommitPanel* commitPanel() const;

    // 返回文件列表视图
    QListView* listView() const;

    // 切换到冲突文件编辑模式（右侧显示 ConflictResolver 而非 DiffViewer）
    void showConflictFile(const QString& repoPath, const QString& filePath);

    // 切换回普通 diff 查看模式
    void showDiffView();

    // 切换到 Code Review 面板（Phase 6）
    void showReviewPanel();

signals:
    // 文件变更列表项被激活（点击/回车）；staged 标识来自暂存区还是未暂存区
    void fileActivated(const QModelIndex& index, bool staged);

    // 请求暂存指定文件
    void stageRequested(const QModelIndex& index);

    // 请求取消暂存指定文件
    void unstageRequested(const QModelIndex& index);

    // 请求丢弃指定文件的更改
    void discardRequested(const QModelIndex& index);

    // 请求将指定文件添加到 .gitignore
    void ignoreRequested(const QModelIndex& index);

    // 请求重命名指定文件
    void renameRequested(const QModelIndex& index);

    // 请求暂存全部文件
    void stageAllRequested();

    // 请求取消暂存全部文件
    void unstageAllRequested();

private slots:
    void handleStagedContextMenu(const QPoint& pos);
    void handleUnstagedContextMenu(const QPoint& pos);

private:
    void setupUi();
    QModelIndex toSourceIndex(const QModelIndex& index) const;
    void emitFileActivated(const QModelIndex& index, bool staged);

    QListView* m_stagedListView;
    QListView* m_unstagedListView;
    QSortFilterProxyModel* m_stagedProxyModel;
    QSortFilterProxyModel* m_unstagedProxyModel;
    QToolButton* m_stageAllButton;
    QToolButton* m_unstageAllButton;
    QSplitter* m_workArea;
    QStackedWidget* m_rightPanelStack;
    DiffViewer* m_diffViewer;
    ConflictResolver* m_conflictResolver;
    ReviewPanel* m_reviewPanel;
    CommitPanel* m_commitPanel;
};

#endif // CHANGESPAGE_H
