#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QString>

class QAction;
class QAbstractItemModel;
class QLabel;
class QListView;
class QPlainTextEdit;
class QPushButton;
class QSplitter;
class QStackedWidget;
class QTabBar;
class QWidget;

// 子组件（前向声明）
class RepositorySidebar;
class ChangesPage;
class ConflictPage;
class HistoryPage;
class StashPage;
class TerminalWidget;
class PullRequestsPage;
class GitService;
class SettingsService;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // 主窗口中心区域页面
    enum Page {
        ChangesPageIdx = 0,
        HistoryPageIdx,
        StashesPageIdx,
        ConflictPageIdx,
        SettingsPageIdx,
        PullRequestsPageIdx
    };
    Q_ENUM(Page)

    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    // -- 规范 Model setter --

    // 设置左侧仓库列表模型
    void setRepositoryModel(QAbstractItemModel* model);
    QAbstractItemModel* repositoryModel() const;

    // 设置工作区文件变更列表模型
    void setFileChangeModel(QAbstractItemModel* model);
    QAbstractItemModel* fileChangeModel() const;

    // 设置 diff 行数据模型（被 ChangesPage 和 HistoryPage 共享）
    void setDiffLineModel(QAbstractItemModel* model);
    QAbstractItemModel* diffLineModel() const;

    // 设置提交历史列表模型
    void setCommitHistoryModel(QAbstractItemModel* model);
    QAbstractItemModel* commitHistoryModel() const;

    // 设置分支列表模型（供顶栏分支下拉菜单使用）
    void setBranchModel(QAbstractItemModel* model);
    QAbstractItemModel* branchModel() const;

    // 设置 stash 列表模型
    void setStashListModel(QAbstractItemModel* model);
    QAbstractItemModel* stashListModel() const;
    // -- 子页面访问（供 Application 层使用） --

    ChangesPage* changesPage() const;
    ConflictPage* conflictPage() const;

    // 设置远程仓库列表模型（供顶栏远程下拉菜单使用）
    void setRemoteModel(QAbstractItemModel* model);
    QAbstractItemModel* remoteModel() const;

    // 设置终端输出模型
    void setTerminalModel(QAbstractItemModel* model);
    // 设置 Pull Request 列表模型
    void setPullRequestModel(QAbstractItemModel* model);
    QAbstractItemModel* pullRequestModel() const;

    // 返回 PullRequestsPage 供 Application 层使用
    PullRequestsPage* pullRequestsPage() const;
    // 获取 HistoryPage（供 Application 绑定模型 / 接入右键菜单）
    HistoryPage* historyPage() const;

    // -- 仓库信息 --

    void setCurrentRepository(const QString& name,
                              const QString& path = QString(),
                              const QString& branch = QString(),
                              const QString& stateHint = QString());
    QString currentRepositoryName() const;
    QString currentRepositoryPath() const;
    QString selectedRemote() const;
    void setSelectedRemote(const QString& remote);

    // -- 页面与状态 --

    void setCurrentPage(Page page);
    Page currentPage() const;
    void setStatusMessage(const QString& message, int timeoutMs = 0);
    void setBusy(bool busy);
    bool isBusy() const;

    // 设置登录状态用户显示
    void setLoggedInUser(const QString& login);
    void clearLoggedInUser();

    // 设置配置服务（供设置页面使用）
    void setSettingsService(SettingsService* settings);
    // 设置 Git 服务（供设置页面读写 git config）
    void setGitService(GitService* service);
    void clearCommitMessage();
    void setCommitDetail(const QString& detail);

signals:
    // 仓库操作
    void openRepositoryRequested();
    void cloneRepositoryRequested();
    void initRepositoryRequested();
    void refreshRequested();
    void fetchRequested();
    void pullRequested();
    void pushRequested();
    void commitRequested(const QString& message);

    // 列表激活
    void repositoryActivated(const QModelIndex& index);
    void fileChangeActivated(const QModelIndex& index, bool staged);
    void commitActivated(const QModelIndex& index);

    // 仓库列表右键
    void removeRepositoryRequested(const QModelIndex& index);

    // 分支操作（由顶栏分支下拉菜单发起）
    void createBranchRequested(const QString& name);
    void deleteBranchRequested(const QString& name);
    void checkoutRequested(const QString& name);
    void mergeRequested(const QString& name);
    void rebaseRequested(const QString& name);

    // Stash 操作（从 StashPage 转发）
    void saveStashRequested(const QString& message);
    void applyStashRequested(int index);
    void dropStashRequested(int index);
    void showStashDiffRequested(int index);

    // Remote 操作（从 RemoteSettingsPage 转发）
    void addRemoteRequested(const QString& name, const QString& url);
    void removeRemoteRequested(const QString& name);
    void renameRemoteRequested(const QString& oldName, const QString& newName);
    void setRemoteUrlRequested(const QString& name, const QString& url);

    // 登录操作
    void loginRequested();

    // PR 操作（从 PullRequestsPage 转发）
    void createPullRequestRequested(const QString& title, const QString& body,
                                    const QString& head, const QString& base);
    void mergePullRequestRequested(int prNumber);

    // 暂存操作（从 ChangesPage 转发）
    void stageRequested(const QModelIndex& index);
    void unstageRequested(const QModelIndex& index);
    void stageAllRequested();
    void unstageAllRequested();

    // 文件变更右键菜单操作（从 ChangesPage 转发）
    void discardRequested(const QModelIndex& index);
    void ignoreRequested(const QModelIndex& index);
    void renameRequested(const QModelIndex& index);

    // 用户请求打开偏好设置对话框
    void preferencesRequested();

    void pageChanged(int pageIndex);

    void checkoutCommitRequested(const QString& hash);
    void createBranchAtCommitRequested(const QString& hash);
    void resetToCommitRequested(const QString& hash, const QString& mode);

private slots:
    void handlePageTabChanged(int index);

private:
    void setupUi();
    void setupActions();
    void setupCentralArea();
    void setupStatusBar();

    QWidget* createRepositorySidebar();
    QWidget* createSettingsPage();

    void updateRepositoryHeader();
    void updateBusyState();

    // -- 成员变量 --

    // 动作
    QAction* m_openRepositoryAction;
    QAction* m_cloneRepositoryAction;
    QAction* m_initRepositoryAction;
    QAction* m_refreshAction;
    QAction* m_fetchAction;
    QAction* m_pullAction;
    QAction* m_pushAction;
    QAction* m_preferencesAction;
    QAction* m_loginAction;

    // 标题区
    QLabel* m_repositoryNameLabel;
    QLabel* m_repositoryBranchLabel;

    // 状态栏
    QLabel* m_statusLabel;
    QLabel* m_busyLabel;
    QLabel* m_loginStatusLabel;

    // 主布局
    QSplitter* m_rootSplitter;

    // 左侧
    RepositorySidebar* m_repositorySidebar;

    // 页面
    QTabBar* m_pageTabs;
    QStackedWidget* m_pageStack;

    // 页面实例（新组件）
    ChangesPage* m_changesPage;
    ConflictPage* m_conflictPage;
    HistoryPage* m_historyPage;
    StashPage* m_stashPage;
    TerminalWidget* m_terminalWidget;
    PullRequestsPage* m_pullRequestsPage;

    // 兼容引用（旧代码使用的成员）
    QListView* m_fileChangeListView;
    QListView* m_commitHistoryListView;
    QPlainTextEdit* m_diffPreview;
    QPlainTextEdit* m_commitDetailView;
    QPlainTextEdit* m_commitMessageEdit;
    QPushButton* m_commitButton;

    // 状态
    QString m_currentRepositoryName;
    QString m_currentRepositoryPath;
    QString m_currentRepositoryBranch;
    QString m_selectedRemote;
    bool m_busy;

    // 分支模型（供顶栏下拉菜单使用）
    QAbstractItemModel* m_branchModel;

    // 配置服务（供设置页面使用）
    SettingsService* m_settingsService;
    // Git 服务（供设置页面读写 git config）
    GitService* m_gitService = nullptr;

    // 远程仓库模型（供顶栏远程下拉菜单使用）
    QAbstractItemModel* m_remoteModel;
};

#endif // MAINWINDOW_H
