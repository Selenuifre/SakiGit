#ifndef BRANCHESPAGE_H
#define BRANCHESPAGE_H

#include <QWidget>

class QAbstractItemModel;
class QLineEdit;
class QListView;
class QPushButton;
class QToolButton;

// 分支管理页面——分支列表 + 创建/切换/删除/合并操作。
// 规范接口：setModel(BranchListModel*)
// 信号：branchSelected, createBranchRequested, deleteBranchRequested,
//        checkoutRequested, mergeRequested
class BranchesPage : public QWidget
{
    Q_OBJECT

public:
    explicit BranchesPage(QWidget* parent = nullptr);

    // 设置分支列表数据源
    void setModel(QAbstractItemModel* model);

    // 返回分支列表模型
    QAbstractItemModel* model() const;

    // 返回分支列表视图
    QListView* listView() const;

signals:
    // 分支列表项被选中
    void branchSelected(const QModelIndex& index);

    // 请求创建新分支（name 来自输入框）
    void createBranchRequested(const QString& name);

    // 请求删除选中分支
    void deleteBranchRequested(const QString& name);

    // 请求切换到选中分支
    void checkoutRequested(const QString& name);

    // 请求将选中分支合并到当前分支
    void mergeRequested(const QString& name);

    // 请求将当前分支变基到选中分支
    void rebaseRequested(const QString& name);

private slots:
    void handleCreateClicked();
    void handleContextMenu(const QPoint& pos);

private:
    void setupUi();

    QListView* m_branchListView;
    QLineEdit* m_newBranchEdit;
    QPushButton* m_createButton;
    QToolButton* m_refreshButton;
};

#endif // BRANCHESPAGE_H
