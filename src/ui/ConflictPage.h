#ifndef CONFLICTPAGE_H
#define CONFLICTPAGE_H

#include <QWidget>

class QAbstractItemModel;
class QListView;
class QPushButton;
class QSplitter;
class ConflictResolver;
class PageHeaderWidget;

// 冲突管理页面——左侧冲突文件列表 + 右侧冲突编辑器
class ConflictPage : public QWidget
{
    Q_OBJECT

public:
    explicit ConflictPage(QWidget* parent = nullptr);

    void setModel(QAbstractItemModel* model);
    QAbstractItemModel* model() const;
    QListView* listView() const;

    // 加载指定冲突文件到右侧编辑器
    void showConflictFile(const QString& repoPath, const QString& filePath);

    // 返回内部 ConflictResolver（供外部连接 markResolvedRequested 等信号）
    ConflictResolver* conflictResolver() const;

    // 返回刷新按钮（通过 PageHeaderWidget）
    QPushButton* refreshButton() const;

    // 设置冲突状态信息
    void setConflictStatus(const QString& text);

signals:
    void conflictFileSelected(const QString& filePath);

private slots:
    void onFileActivated(const QModelIndex& index);

private:
    void setupUi();

    PageHeaderWidget* m_headerWidget;
    QListView* m_conflictListView;
    ConflictResolver* m_conflictResolver;
    QSplitter* m_splitter;
};

#endif // CONFLICTPAGE_H
