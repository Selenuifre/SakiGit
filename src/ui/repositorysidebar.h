#ifndef REPOSITORYSIDEBAR_H
#define REPOSITORYSIDEBAR_H

#include <QModelIndex>
#include <QString>
#include <QWidget>

class QAction;
class QAbstractItemModel;
class QLabel;
class QListView;
class QLineEdit;
class QMenu;
class QSortFilterProxyModel;
class QToolButton;

class RepositorySidebar : public QWidget
{
    Q_OBJECT

public:
    // 创建仓库侧边栏组件
    explicit RepositorySidebar(QWidget* parent = nullptr);

    // 释放侧边栏持有的 Qt 子对象
    ~RepositorySidebar() override;

    // 设置仓库列表源模型
    void setRepositoryModel(QAbstractItemModel* model);

    // 返回仓库列表源模型
    QAbstractItemModel* repositoryModel() const;

    // 返回仓库列表代理模型
    QSortFilterProxyModel* proxyModel() const;

    // 返回仓库列表视图
    QListView* listView() const;

    // 设置当前选中的仓库索引
    void setCurrentRepositoryIndex(const QModelIndex& sourceIndex);

    // 返回当前选中的源模型索引
    QModelIndex currentRepositoryIndex() const;

    // 设置过滤关键字
    void setFilterText(const QString& text);

    // 返回过滤关键字
    QString filterText() const;

    // 设置侧边栏忙碌状态
    void setBusy(bool busy);

    // 返回侧边栏是否处于忙碌状态
    bool isBusy() const;

    // 设置空列表提示文本
    void setEmptyText(const QString& text);

    // 返回空列表提示文本
    QString emptyText() const;

signals:
    // 请求打开本地仓库
    void openRepositoryRequested();

    // 请求克隆远程仓库
    void cloneRepositoryRequested();

    // 请求刷新仓库列表
    void refreshRequested();

    // 仓库被激活
    void repositoryActivated(const QModelIndex& sourceIndex);

    // 当前仓库选择发生变化
    void repositorySelectionChanged(const QModelIndex& sourceIndex);

    // 请求移除指定仓库
    void removeRepositoryRequested(const QModelIndex& sourceIndex);

    // 过滤关键字发生变化
    void filterTextChanged(const QString& text);

private slots:
    // 处理过滤输入变化
    void handleFilterTextChanged(const QString& text);

    // 处理列表激活事件
    void handleRepositoryActivated(const QModelIndex& proxyIndex);

    // 处理当前列表选择变化
    void handleCurrentChanged(const QModelIndex& current, const QModelIndex& previous);

    // 显示仓库列表右键菜单
    void showContextMenu(const QPoint& position);

    // 处理移除仓库动作
    void handleRemoveAction();

    // 更新空状态提示
    void updateEmptyState();

private:
    // 创建界面控件
    void setupUi();

    // 创建动作和信号连接
    void setupActions();

    // 将代理模型索引转换为源模型索引
    QModelIndex mapToSourceIndex(const QModelIndex& proxyIndex) const;

    // 将源模型索引转换为代理模型索引
    QModelIndex mapFromSourceIndex(const QModelIndex& sourceIndex) const;

    // 返回当前右键菜单对应的源模型索引
    QModelIndex contextSourceIndex() const;

    // 根据忙碌状态更新操作可用性
    void updateBusyState();

private:
    // 空列表提示标签
    QLabel* m_emptyLabel;

    // 过滤输入框
    QLineEdit* m_filterEdit;

    // 仓库列表视图
    QListView* m_listView;

    // 右键菜单
    QMenu* m_contextMenu;

    // 打开仓库动作
    QAction* m_openAction;

    // 克隆仓库动作
    QAction* m_cloneAction;

    // 刷新仓库动作
    QAction* m_refreshAction;

    // 移除仓库动作
    QAction* m_removeAction;

    // 仓库过滤代理模型
    QSortFilterProxyModel* m_proxyModel;

    // 当前右键菜单对应的代理索引
    QModelIndex m_contextProxyIndex;

    // 空列表提示文本
    QString m_emptyText;

    // 当前是否处于忙碌状态
    bool m_busy;
};

#endif // REPOSITORYSIDEBAR_H
