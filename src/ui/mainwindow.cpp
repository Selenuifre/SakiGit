#include "mainwindow.h"
#include "changespage.h"
#include "commitpanel.h"
#include "ConflictPage.h"
#include "diffviewer.h"
#include "historypage.h"
#include "repositorysidebar.h"
#include "PullRequestsPage.h"
#include "StashPage.h"
#include "services/gitservice.h"
#include "services/settingsservice.h"
#include "TerminalWidget.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QListView>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimer>
#include <QToolButton>
#include <QStatusBar>
#include <QStyle>
#include <QTabBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QWidgetAction>

namespace {

// Two-line header button: label on top, value below
QPushButton* makeHeaderButton(const QString& label, const QString& value,
                               const QString& objName, int width, QWidget* parent)
{
    auto* btn = new QPushButton(label + QStringLiteral("\n") + value, parent);
    btn->setObjectName(objName);
    btn->setFixedWidth(width);
    btn->setFlat(true);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

void updateHeaderValue(QPushButton* btn, const QString& value)
{
    if (auto* tl = btn->findChild<QLabel*>(QStringLiteral("headerBtnText"))) {
        QString label = btn->property("headerLabel").toString();
        tl->setText(QStringLiteral(
            "<span style='color:#8B949E;'>%1</span><br>"
            "<span style='color:#FFFFFF; font-weight:bold;'>%2</span>"
        ).arg(label, value));
    }
}

QLabel* createSectionLabel(const QString& text)
{
    auto* label = new QLabel(text);
    label->setObjectName(QStringLiteral("sectionLabel"));
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    return label;
}

QFrame* createSeparator()
{
    auto* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}
}

// ========== 构造 / 析构 ==========

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    m_openRepositoryAction(nullptr),
    m_cloneRepositoryAction(nullptr),
    m_initRepositoryAction(nullptr),
    m_refreshAction(nullptr),
    m_fetchAction(nullptr),
    m_pullAction(nullptr),
    m_pushAction(nullptr),
    m_preferencesAction(nullptr),
    m_loginAction(nullptr),
    m_repositoryNameLabel(nullptr),
    m_repositoryBranchLabel(nullptr),
    m_statusLabel(nullptr),
    m_busyLabel(nullptr),
    m_loginStatusLabel(nullptr),
    m_rootSplitter(nullptr),
    m_repositorySidebar(nullptr),
    m_pageTabs(nullptr),
    m_pageStack(nullptr),
    m_changesPage(nullptr),
    m_conflictPage(nullptr),
    m_historyPage(nullptr),
    m_stashPage(nullptr),
    m_pullRequestsPage(nullptr),
    m_fileChangeListView(nullptr),
    m_commitHistoryListView(nullptr),
    m_diffPreview(nullptr),
    m_commitDetailView(nullptr),
    m_commitMessageEdit(nullptr),
    m_commitButton(nullptr),
    m_busy(false),
    m_settingsService(nullptr)
{
    setupUi();
}

MainWindow::~MainWindow()
{
}

// ========== Model setter ==========

void MainWindow::setRepositoryModel(QAbstractItemModel* model)
{
    m_repositorySidebar->setRepositoryModel(model);
}

QAbstractItemModel* MainWindow::repositoryModel() const
{
    return m_repositorySidebar->repositoryModel();
}

void MainWindow::setFileChangeModel(QAbstractItemModel* model)
{
    m_changesPage->setModel(model);
    // 保持旧成员同步（兼容外部可能通过指针直接使用的情况）
    m_fileChangeListView = m_changesPage->listView();
}

QAbstractItemModel* MainWindow::fileChangeModel() const
{
    return m_changesPage->model();
}

void MainWindow::setDiffLineModel(QAbstractItemModel* model)
{
    // DiffLineModel 被 ChangesPage 和 HistoryPage 共享
    m_changesPage->setDiffModel(model);
    m_historyPage->setDiffModel(model);
}

QAbstractItemModel* MainWindow::diffLineModel() const
{
    return m_changesPage->diffViewer()->model();
}

void MainWindow::setCommitHistoryModel(QAbstractItemModel* model)
{
    m_historyPage->setModel(model);
    // 保持旧成员同步
    m_commitHistoryListView = m_historyPage->listView();
}

QAbstractItemModel* MainWindow::commitHistoryModel() const
{
    return m_historyPage->model();
}

void MainWindow::setBranchModel(QAbstractItemModel* model)
{
    m_branchModel = model;
}

QAbstractItemModel* MainWindow::branchModel() const
{
    return m_branchModel;
}

void MainWindow::setStashListModel(QAbstractItemModel* model)
{
    m_stashPage->setModel(model);
}

QAbstractItemModel* MainWindow::stashListModel() const
{
    return m_stashPage->model();
}

void MainWindow::setRemoteModel(QAbstractItemModel* model)
{
    m_remoteModel = model;
}

QAbstractItemModel* MainWindow::remoteModel() const
{
    return m_remoteModel;
}

void MainWindow::setTerminalModel(QAbstractItemModel* model)
{
    if (m_terminalWidget) {
        m_terminalWidget->setModel(model);
    }
}

ChangesPage* MainWindow::changesPage() const
{
    return m_changesPage;
}

ConflictPage* MainWindow::conflictPage() const
{
    return m_conflictPage;
}

void MainWindow::setPullRequestModel(QAbstractItemModel* model)
{
    if (m_pullRequestsPage) {
        m_pullRequestsPage->setModel(model);
    }
}

QAbstractItemModel* MainWindow::pullRequestModel() const
{
    return m_pullRequestsPage ? m_pullRequestsPage->model() : nullptr;
}

PullRequestsPage* MainWindow::pullRequestsPage() const
{
    return m_pullRequestsPage;
}
HistoryPage* MainWindow::historyPage() const
{
    return m_historyPage;
}

// ========== 仓库信息 ==========

void MainWindow::setCurrentRepository(const QString& name, const QString& path,
                                       const QString& branch, const QString& stateHint)
{
    m_currentRepositoryName = name.trimmed();
    m_currentRepositoryPath = path.trimmed();
    m_currentRepositoryBranch = branch.trimmed();

    // 如果传入了状态提示（如 "merging" / "rebasing"），追加到分支名后
    if (!stateHint.trimmed().isEmpty()) {
        m_currentRepositoryBranch = m_currentRepositoryBranch
            + QStringLiteral(" | ") + stateHint.trimmed();
    }

    updateRepositoryHeader();
}

QString MainWindow::currentRepositoryName() const
{
    return m_currentRepositoryName;
}

QString MainWindow::currentRepositoryPath() const
{
    return m_currentRepositoryPath;
}

QString MainWindow::selectedRemote() const
{
    return m_selectedRemote;
}

void MainWindow::setSelectedRemote(const QString& remote)
{
    m_selectedRemote = remote.trimmed();
}

// ========== 页面与状态 ==========

void MainWindow::setCurrentPage(Page page)
{
    const int index = static_cast<int>(page);
    if (index < 0 || index >= m_pageStack->count()) {
        return;
    }

    if (m_pageTabs->currentIndex() != index) {
        m_pageTabs->setCurrentIndex(index);
    }
    if (m_pageStack->currentIndex() != index) {
        m_pageStack->setCurrentIndex(index);
        emit pageChanged(index);
    }
}

MainWindow::Page MainWindow::currentPage() const
{
    return static_cast<Page>(m_pageStack->currentIndex());
}

void MainWindow::setStatusMessage(const QString& message, int timeoutMs)
{
    const QString cleanMessage = message.trimmed();
    m_statusLabel->setText(cleanMessage.isEmpty() ? tr("Ready") : cleanMessage);

    if (timeoutMs > 0) {
        statusBar()->showMessage(cleanMessage, timeoutMs);
    }
}

void MainWindow::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }
    m_busy = busy;
    updateBusyState();
}

bool MainWindow::isBusy() const
{
    return m_busy;
}

void MainWindow::setLoggedInUser(const QString& login)
{
    if (login.isEmpty()) {
        m_loginStatusLabel->setText(QString());
        m_loginStatusLabel->setVisible(false);
        // Also update settings page status
        auto* sl = findChild<QLabel*>(QStringLiteral("settingsLoginStatus"));
        if (sl) sl->setText(tr("Not logged in"));
        return;
    }
    m_loginStatusLabel->setText(QStringLiteral("GitHub: %1").arg(login));
    m_loginStatusLabel->setStyleSheet(QStringLiteral("color: #2da44e; font-weight: bold; padding-right: 12px;"));
    m_loginStatusLabel->setVisible(true);
    // Also update settings page status
    auto* sl = findChild<QLabel*>(QStringLiteral("settingsLoginStatus"));
    if (sl) sl->setText(tr("Logged in as: %1").arg(login));
}

void MainWindow::clearLoggedInUser()
{
    m_loginStatusLabel->setVisible(false);
}

void MainWindow::setSettingsService(SettingsService* settings)
{
    m_settingsService = settings;
}

void MainWindow::setGitService(GitService* service)
{
    m_gitService = service;
}

// ========== 内部槽 ==========

void MainWindow::clearCommitMessage()
{
    if (m_changesPage && m_changesPage->commitPanel()) {
        m_changesPage->commitPanel()->clear();
    }
}

void MainWindow::setCommitDetail(const QString& detail)
{
    if (m_historyPage) {
        m_historyPage->setCommitDetail(detail);
    }
}

void MainWindow::handlePageTabChanged(int index)
{
    if (index < 0 || index >= m_pageStack->count()) {
        return;
    }
    if (m_pageStack->currentIndex() == index) {
        return;
    }

    m_pageStack->setCurrentIndex(index);
    emit pageChanged(index);
}

// ========== UI 搭建 ==========

void MainWindow::setupUi()
{
    setObjectName(QStringLiteral("MainWindow"));
    setWindowTitle(tr("SakiGit"));
    resize(1180, 760);

    setupActions();
    setupCentralArea();
    setupStatusBar();
    updateRepositoryHeader();
    updateBusyState();
}

void MainWindow::setupActions()
{
    m_openRepositoryAction = new QAction(
        QIcon(QStringLiteral(":/icons/action_open.svg")), tr("Open"), this);
    m_openRepositoryAction->setShortcut(QKeySequence::Open);

    m_cloneRepositoryAction = new QAction(
        QIcon(QStringLiteral(":/icons/action_clone.svg")), tr("Clone"), this);

    m_initRepositoryAction = new QAction(
        QIcon(QStringLiteral(":/icons/action_open.svg")), tr("Init..."), this);
    m_initRepositoryAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));

    m_refreshAction = new QAction(
        QIcon(QStringLiteral(":/icons/action_refresh.svg")), tr("Refresh"), this);
    m_refreshAction->setShortcut(QKeySequence::Refresh);

    m_fetchAction = new QAction(
        QIcon(QStringLiteral(":/icons/action_fetch.svg")), tr("Fetch"), this);

    m_pullAction = new QAction(
        QIcon(QStringLiteral(":/icons/action_pull.svg")), tr("Pull"), this);

    m_pushAction = new QAction(
        QIcon(QStringLiteral(":/icons/action_push.svg")), tr("Push"), this);

    m_preferencesAction = new QAction(
        QIcon(QStringLiteral(":/icons/action_preferences.svg")), tr("Preferences"), this);
    m_loginAction = new QAction(
        QIcon(QStringLiteral(":/icons/action_login.svg")), tr("GitHub Login"), this);

    connect(m_openRepositoryAction, &QAction::triggered,
            this, &MainWindow::openRepositoryRequested);
    connect(m_cloneRepositoryAction, &QAction::triggered,
            this, &MainWindow::cloneRepositoryRequested);
    connect(m_initRepositoryAction, &QAction::triggered,
            this, &MainWindow::initRepositoryRequested);
    connect(m_refreshAction, &QAction::triggered,
            this, &MainWindow::refreshRequested);
    connect(m_fetchAction, &QAction::triggered,
            this, &MainWindow::fetchRequested);
    connect(m_pullAction, &QAction::triggered,
            this, &MainWindow::pullRequested);
    connect(m_pushAction, &QAction::triggered,
            this, &MainWindow::pushRequested);
    connect(m_preferencesAction, &QAction::triggered,
            this, &MainWindow::preferencesRequested);
    connect(m_loginAction, &QAction::triggered,
            this, &MainWindow::loginRequested);

    QMenu* fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(m_initRepositoryAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_openRepositoryAction);
    fileMenu->addAction(m_cloneRepositoryAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_preferencesAction);

    QMenu* repositoryMenu = menuBar()->addMenu(tr("Repository"));
    repositoryMenu->addAction(m_refreshAction);
    repositoryMenu->addSeparator();
    repositoryMenu->addAction(m_fetchAction);
    repositoryMenu->addAction(m_pullAction);
    repositoryMenu->addAction(m_pushAction);

    QMenu* gitHubMenu = menuBar()->addMenu(tr("GitHub"));
    gitHubMenu->addAction(m_loginAction);

    // Hide the traditional menu bar — actions are accessible via hamburger menu
    menuBar()->setVisible(false);
}

void MainWindow::setupCentralArea()
{
    auto* central = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ===== GitHub Desktop-style Unified Top Bar (56px) =====
    auto* topBar = new QWidget(central);
    topBar->setObjectName(QStringLiteral("topHeaderBar"));
    topBar->setFixedHeight(68);

    auto* topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(4, 0, 12, 0);
    topBarLayout->setSpacing(4);

    // Helper: create a header button with two QLabel lines inside
    auto makeHeaderBtn = [](QWidget* parent, const QString& objName,
                             const QString& label, const QString& value,
                             int width) -> QPushButton* {
        auto* btn = new QPushButton(parent);
        btn->setObjectName(objName);
        btn->setFixedWidth(width);
        btn->setFlat(true);
        btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        btn->setProperty("headerLabel", label);
        auto* lay = new QVBoxLayout(btn);
        lay->setContentsMargins(14, 6, 14, 6);
        auto* text = new QLabel(btn);
        text->setObjectName(QStringLiteral("headerBtnText"));
        text->setTextFormat(Qt::RichText);
        text->setText(QStringLiteral(
            "<span style='color:#8B949E;'>%1</span><br>"
            "<span style='color:#FFFFFF; font-weight:bold;'>%2</span>"
        ).arg(label, value));
        lay->addWidget(text);
        return btn;
    };
    // Hamburger menu button (replaces menubar)
    m_repositoryNameLabel = nullptr; // replaced by repoBtn below
    auto* repoBtn = makeHeaderBtn(topBar,
        QStringLiteral("repoMenuButton"),
        tr("Repository"), tr("No repository"), 260);
    QMenu* repoMenu = new QMenu(repoBtn);
    repoBtn->setMenu(repoMenu);

    connect(repoMenu, &QMenu::aboutToShow, this, [this, repoMenu]() {
        repoMenu->clear();
        QAbstractItemModel* repoModel = m_repositorySidebar
            ? m_repositorySidebar->repositoryModel() : nullptr;
        const int rowCount = repoModel ? repoModel->rowCount() : 0;

        // Container: filter + header + list
        auto* container = new QWidget;
        auto* cl = new QVBoxLayout(container);
        cl->setContentsMargins(6, 4, 6, 4);
        cl->setSpacing(4);

        // Filter input
        auto* filterEdit = new QLineEdit;
        filterEdit->setPlaceholderText(tr("Filter repositories..."));
        filterEdit->setClearButtonEnabled(true);
        filterEdit->setObjectName(QStringLiteral("repoFilterEdit"));
        cl->addWidget(filterEdit);

        // Section label + count
        auto* headerLabel = new QLabel(tr("Recent Repositories"));
        headerLabel->setObjectName(QStringLiteral("repoPopupHeader"));
        cl->addWidget(headerLabel);

        // Repository list with proxy for filtering
        auto* repoList = new QListView;
        repoList->setObjectName(QStringLiteral("repoPopupList"));
        auto* proxyModel = new QSortFilterProxyModel(repoList);
        if (repoModel) {
            proxyModel->setSourceModel(repoModel);
            proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
            proxyModel->setFilterRole(Qt::DisplayRole);
        }
        repoList->setModel(proxyModel);
        repoList->setEditTriggers(QListView::NoEditTriggers);
        repoList->setSelectionMode(QListView::SingleSelection);
        repoList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        repoList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        repoList->setUniformItemSizes(true);
        const int vis = qMin(qMax(rowCount, 1), 8);
        repoList->setFixedHeight(vis * 28 + 4);
        cl->addWidget(repoList);

        connect(filterEdit, &QLineEdit::textChanged, proxyModel,
                &QSortFilterProxyModel::setFilterFixedString);

        auto* ca = new QWidgetAction(repoMenu);
        ca->setDefaultWidget(container);
        repoMenu->addAction(ca);

        if (rowCount == 0) {
            repoList->setVisible(false);
        }

        // Left-click → activate repository
        connect(repoList, &QListView::clicked, this,
                [this, proxyModel](const QModelIndex& proxyIdx) {
            const QModelIndex srcIdx = proxyModel->mapToSource(proxyIdx);
            if (srcIdx.isValid()) emit repositoryActivated(srcIdx);
        });

        // Right-click context menu: Remove
        repoList->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(repoList, &QListView::customContextMenuRequested, this,
                [this, proxyModel, repoList](const QPoint& pos) {
            const QModelIndex proxyIdx = repoList->indexAt(pos);
            if (!proxyIdx.isValid()) return;
            const QModelIndex srcIdx = proxyModel->mapToSource(proxyIdx);

            QMenu ctxMenu(this);
            QAction* openAct = ctxMenu.addAction(tr("Open"));
            QAction* cloneAct = ctxMenu.addAction(tr("Clone"));
            QAction* initAct = ctxMenu.addAction(tr("Init New..."));
            QAction* refreshAct = ctxMenu.addAction(tr("Refresh"));
            ctxMenu.addSeparator();
            QAction* removeAct = ctxMenu.addAction(tr("Remove"));

            QAction* chosen = ctxMenu.exec(repoList->viewport()->mapToGlobal(pos));
            if (chosen == openAct) {
                emit openRepositoryRequested();
            } else if (chosen == cloneAct) {
                emit cloneRepositoryRequested();
            } else if (chosen == initAct) {
                emit initRepositoryRequested();
            } else if (chosen == refreshAct) {
                emit refreshRequested();
            } else if (chosen == removeAct) {
                emit removeRepositoryRequested(srcIdx);
            }
        });

        // Auto-focus filter
        QTimer::singleShot(0, filterEdit, [filterEdit]() { filterEdit->setFocus(); });

        // Bottom actions
        repoMenu->addSeparator();
        repoMenu->addAction(m_initRepositoryAction);
        repoMenu->addAction(m_openRepositoryAction);
        repoMenu->addAction(m_cloneRepositoryAction);
    });

    // Branch dropdown button
    auto* branchBtn = makeHeaderBtn(topBar,
        QStringLiteral("branchDropdownButton"),
        QStringLiteral("Branch"), QStringLiteral("-"), 250);

    m_repositoryBranchLabel = nullptr; // no longer a QLabel
    QMenu* branchMenu = new QMenu(branchBtn);
    branchBtn->setMenu(branchMenu);

    // Build branch menu with scrollable QListView + action buttons
    connect(branchMenu, &QMenu::aboutToShow, this, [this, branchMenu]() {
        branchMenu->clear();
        if (!m_branchModel || m_currentRepositoryPath.isEmpty()) {
            branchMenu->addAction(tr("No repository"))->setEnabled(false);
            return;
        }

        // Container widget: filter input + branch list
        auto* container = new QWidget;
        container->setObjectName(QStringLiteral("branchPopupContainer"));
        auto* containerLayout = new QVBoxLayout(container);
        containerLayout->setContentsMargins(6, 4, 6, 4);
        containerLayout->setSpacing(4);

        // Filter input
        auto* filterEdit = new QLineEdit;
        filterEdit->setPlaceholderText(tr("Filter branches..."));
        filterEdit->setClearButtonEnabled(true);
        filterEdit->setObjectName(QStringLiteral("branchFilterEdit"));
        containerLayout->addWidget(filterEdit);

        // Branch list with proxy model for filtering
        auto* branchList = new QListView;
        branchList->setObjectName(QStringLiteral("branchPopupList"));
        auto* proxyModel = new QSortFilterProxyModel(branchList);
        proxyModel->setSourceModel(m_branchModel);
        proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        proxyModel->setFilterRole(Qt::DisplayRole);
        branchList->setModel(proxyModel);
        branchList->setEditTriggers(QListView::NoEditTriggers);
        branchList->setSelectionMode(QListView::SingleSelection);
        branchList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        branchList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        branchList->setUniformItemSizes(true);

        // Connect filter
        connect(filterEdit, &QLineEdit::textChanged, proxyModel,
                &QSortFilterProxyModel::setFilterFixedString);

        // Fixed height: show ~8 branches, scroll beyond
        const int rowCount = m_branchModel->rowCount();
        const int visibleRows = qMin(rowCount, 8);
        branchList->setFixedHeight(visibleRows * 28 + 4);
        containerLayout->addWidget(branchList);

        // Pre-select current branch (find in source, map to proxy)
        for (int r = 0; r < proxyModel->rowCount(); ++r) {
            const QModelIndex srcIdx = proxyModel->mapToSource(proxyModel->index(r, 0));
            if (m_branchModel->data(srcIdx,
                    static_cast<int>(Qt::UserRole + 2)).toBool()) {
                branchList->setCurrentIndex(proxyModel->index(r, 0));
                break;
            }
        }

        auto* containerAction = new QWidgetAction(branchMenu);
        containerAction->setDefaultWidget(container);
        branchMenu->addAction(containerAction);

        // Auto-focus the filter input when menu opens
        QTimer::singleShot(0, filterEdit, [filterEdit]() {
            filterEdit->setFocus();
        });

        // Left-click → checkout (map through proxy model)
        connect(branchList, &QListView::clicked, this,
                [this, branchList, proxyModel](const QModelIndex& proxyIdx) {
            const QModelIndex srcIdx = proxyModel->mapToSource(proxyIdx);
            const QString name = m_branchModel->data(srcIdx, Qt::DisplayRole).toString();
            if (!name.isEmpty()) emit checkoutRequested(name);
        });

        // Right-click → context menu with branch operations (map through proxy)
        branchList->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(branchList, &QListView::customContextMenuRequested,
                this, [this, branchList, proxyModel](const QPoint& pos) {
            const QModelIndex proxyIdx = branchList->indexAt(pos);
            if (!proxyIdx.isValid()) return;
            const QModelIndex srcIdx = proxyModel->mapToSource(proxyIdx);
            const QString name = m_branchModel->data(srcIdx, Qt::DisplayRole).toString();
            const bool isCurrent = m_branchModel->data(srcIdx,
                static_cast<int>(Qt::UserRole + 2)).toBool();

            QMenu ctxMenu(this);
            QAction* checkoutAct = ctxMenu.addAction(tr("Checkout"));
            checkoutAct->setEnabled(!isCurrent);
            ctxMenu.addSeparator();
            QAction* mergeAct = ctxMenu.addAction(tr("Merge into current branch..."));
            QAction* rebaseAct = ctxMenu.addAction(tr("Rebase current branch onto..."));
            ctxMenu.addSeparator();
            QAction* deleteAct = ctxMenu.addAction(tr("Delete"));
            deleteAct->setEnabled(!isCurrent);

            QAction* chosen = ctxMenu.exec(branchList->viewport()->mapToGlobal(pos));
            if (chosen == checkoutAct) {
                emit checkoutRequested(name);
            } else if (chosen == mergeAct) {
                emit mergeRequested(name);
            } else if (chosen == rebaseAct) {
                emit rebaseRequested(name);
            } else if (chosen == deleteAct) {
                emit deleteBranchRequested(name);
            }
        });

        // Separator + action buttons below the list
        branchMenu->addSeparator();
        QAction* createAct = branchMenu->addAction(tr("Create Branch..."));
        connect(createAct, &QAction::triggered, this, [this]() {
            bool ok;
            const QString name = QInputDialog::getText(this, tr("Create Branch"),
                tr("Branch name:"), QLineEdit::Normal, QString(), &ok);
            if (ok && !name.trimmed().isEmpty())
                emit createBranchRequested(name.trimmed());
        });
        QAction* mergeAct = branchMenu->addAction(tr("Merge into current branch..."));
        connect(mergeAct, &QAction::triggered, this, [this]() {
            emit mergeRequested(QString());
        });
        QAction* rebaseAct = branchMenu->addAction(tr("Rebase current branch onto..."));
        connect(rebaseAct, &QAction::triggered, this, [this]() {
            emit rebaseRequested(QString());
        });
    });

    // Update button text when branch changes
    connect(branchMenu, &QMenu::aboutToHide, this, [this, branchBtn]() {
        if (!m_currentRepositoryBranch.isEmpty())
            updateHeaderValue(branchBtn, m_currentRepositoryBranch);
    });

    topBarLayout->addWidget(repoBtn);
    topBarLayout->addWidget(branchBtn);

    // Remote dropdown button (scrollable QListView + context menu)
    auto* remoteBtn = makeHeaderBtn(topBar,
        QStringLiteral("remoteDropdownButton"),
        tr("Remote"), tr("remote"), 250);
    QMenu* remoteMenu = new QMenu(remoteBtn);
    remoteBtn->setMenu(remoteMenu);

    connect(remoteMenu, &QMenu::aboutToShow, this, [this, remoteBtn, remoteMenu]() {
        remoteMenu->clear();
        if (!m_remoteModel || m_currentRepositoryPath.isEmpty()) {
            remoteMenu->addAction(tr("No repository"))->setEnabled(false);
            return;
        }
        const int rowCount = m_remoteModel->rowCount();
        if (rowCount == 0) {
            remoteMenu->addAction(tr("No remotes configured"))->setEnabled(false);
        }

        // Scrollable remote list
        auto* container = new QWidget;
        container->setObjectName(QStringLiteral("remotePopupContainer"));
        auto* containerLayout = new QVBoxLayout(container);
        containerLayout->setContentsMargins(6, 4, 6, 4);
        containerLayout->setSpacing(4);

        auto* remoteList = new QListView;
        remoteList->setObjectName(QStringLiteral("remotePopupList"));
        remoteList->setModel(m_remoteModel);
        remoteList->setEditTriggers(QListView::NoEditTriggers);
        remoteList->setSelectionMode(QListView::SingleSelection);
        remoteList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        remoteList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        remoteList->setUniformItemSizes(true);
        const int visRows = qMin(qMax(rowCount, 1), 6);
        remoteList->setFixedHeight(visRows * 28 + 4);
        containerLayout->addWidget(remoteList);

        auto* containerAction = new QWidgetAction(remoteMenu);
        containerAction->setDefaultWidget(container);
        remoteMenu->addAction(containerAction);

        // Left-click → switch current remote (update button text + store selection)
        connect(remoteList, &QListView::clicked, this, [this, remoteBtn](const QModelIndex& idx) {
            const QString name = idx.data(Qt::DisplayRole).toString();
            if (!name.isEmpty()) {
                updateHeaderValue(remoteBtn, name);
                setSelectedRemote(name);
            }
        });

        // Right-click context menu: Rename, Change URL, Remove
        remoteList->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(remoteList, &QListView::customContextMenuRequested,
                this, [this, remoteList](const QPoint& pos) {
            const QModelIndex idx = remoteList->indexAt(pos);
            if (!idx.isValid()) return;
            const QString name = m_remoteModel->data(idx, Qt::DisplayRole).toString();
            const QString url = m_remoteModel->data(idx, static_cast<int>(Qt::UserRole + 2)).toString();

            QMenu ctxMenu(this);
            QAction* renameAct = ctxMenu.addAction(tr("Rename..."));
            QAction* setUrlAct = ctxMenu.addAction(tr("Change URL..."));
            ctxMenu.addSeparator();
            QAction* removeAct = ctxMenu.addAction(tr("Remove"));

            QAction* chosen = ctxMenu.exec(remoteList->viewport()->mapToGlobal(pos));
            if (chosen == renameAct) {
                bool ok;
                const QString newName = QInputDialog::getText(this, tr("Rename Remote"),
                    tr("New name for '%1':").arg(name), QLineEdit::Normal, name, &ok);
                if (ok && !newName.trimmed().isEmpty() && newName.trimmed() != name)
                    emit renameRemoteRequested(name, newName.trimmed());
            } else if (chosen == setUrlAct) {
                bool ok;
                const QString newUrl = QInputDialog::getText(this, tr("Change Remote URL"),
                    tr("New URL for '%1':").arg(name), QLineEdit::Normal, url, &ok);
                if (ok && !newUrl.trimmed().isEmpty())
                    emit setRemoteUrlRequested(name, newUrl.trimmed());
            } else if (chosen == removeAct) {
                emit removeRemoteRequested(name);
            }
        });

        // Add Remote button at bottom
        remoteMenu->addSeparator();
        QAction* addAct = remoteMenu->addAction(tr("Add Remote..."));
        connect(addAct, &QAction::triggered, this, [this]() {
            QDialog dlg(this);
            dlg.setWindowTitle(tr("Add Remote"));
            dlg.setMinimumWidth(400);
            auto* form = new QFormLayout(&dlg);
            form->setContentsMargins(16, 12, 16, 12);
            form->setSpacing(10);

            auto* nameEdit = new QLineEdit(&dlg);
            nameEdit->setPlaceholderText(tr("e.g. origin"));
            nameEdit->setText(QStringLiteral("origin"));
            form->addRow(tr("Name:"), nameEdit);

            auto* urlEdit = new QLineEdit(&dlg);
            urlEdit->setPlaceholderText(tr("e.g. https://github.com/user/repo.git"));
            form->addRow(tr("URL:"), urlEdit);

            auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            form->addRow(btnBox);
            connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
            connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

            if (dlg.exec() == QDialog::Accepted) {
                const QString name = nameEdit->text().trimmed();
                const QString url = urlEdit->text().trimmed();
                if (!name.isEmpty() && !url.isEmpty())
                    emit addRemoteRequested(name, url);
            }
        });
    });

    // Update remote button text on open (default if none selected)
    connect(remoteMenu, &QMenu::aboutToShow, this, [this, remoteBtn]() {
        auto* txtLabel = remoteBtn->findChild<QLabel*>(QStringLiteral("headerBtnText"));
        if (!m_remoteModel || m_remoteModel->rowCount() == 0) {
            updateHeaderValue(remoteBtn, tr("remote"));
            return;
        }
        // Only set default if button still shows placeholder
        if (txtLabel && txtLabel->text().contains(tr("remote"))) {
            const QString first = m_remoteModel->data(m_remoteModel->index(0, 0), Qt::DisplayRole).toString();
            if (!first.isEmpty()) updateHeaderValue(remoteBtn, first);
        }
    });

    // One-click Fetch button next to remote dropdown
    auto* remoteFetchBtn = new QToolButton(topBar);
    remoteFetchBtn->setDefaultAction(m_fetchAction);
    remoteFetchBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    remoteFetchBtn->setObjectName(QStringLiteral("remoteFetchButton"));

    topBarLayout->addWidget(remoteBtn);
    topBarLayout->addWidget(remoteFetchBtn);
    topBarLayout->addStretch(1);

    // Action buttons (reuse QActions from setupActions)
    auto* fetchBtn = new QToolButton(topBar);
    fetchBtn->setDefaultAction(m_fetchAction);
    fetchBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);

    auto* pullBtn = new QToolButton(topBar);
    pullBtn->setDefaultAction(m_pullAction);
    pullBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    pullBtn->setObjectName(QStringLiteral("topBarActionBtn"));

    auto* pushBtn = new QToolButton(topBar);
    pushBtn->setDefaultAction(m_pushAction);
    pushBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    pushBtn->setObjectName(QStringLiteral("topBarActionBtn"));

    auto* refreshBtn = new QToolButton(topBar);
    refreshBtn->setDefaultAction(m_refreshAction);
    refreshBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    refreshBtn->setObjectName(QStringLiteral("topBarActionBtn"));

    fetchBtn->setObjectName(QStringLiteral("topBarActionBtn"));

    topBarLayout->addWidget(fetchBtn);
    topBarLayout->addWidget(pullBtn);
    topBarLayout->addWidget(pushBtn);
    topBarLayout->addWidget(refreshBtn);

    // ===== Content area (full width, sidebar removed) =====
    // Sidebar still created for its model, but not added to layout
    createRepositorySidebar()->setVisible(false);

    auto* content = new QWidget(central);
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(8, 8, 8, 8);
    contentLayout->setSpacing(6);

    m_pageTabs = new QTabBar(content);
    m_pageTabs->setObjectName(QStringLiteral("pageTabs"));
    m_pageTabs->setExpanding(false);
    m_pageTabs->addTab(QIcon(QStringLiteral(":/icons/tab_changes.svg")), tr("Changes"));
    m_pageTabs->addTab(QIcon(QStringLiteral(":/icons/tab_history.svg")), tr("History"));
    m_pageTabs->addTab(QIcon(QStringLiteral(":/icons/tab_stashes.svg")), tr("Stashes"));
    m_pageTabs->addTab(QIcon(QStringLiteral(":/icons/tab_conflicts.svg")), tr("Conflicts"));
    m_pageTabs->addTab(QIcon(QStringLiteral(":/icons/tab_pr.svg")), tr("Pull Requests"));
    m_pageTabs->addTab(QIcon(QStringLiteral(":/icons/tab_settings.svg")), tr("Settings"));

    // ---- 使用独立页面组件 ----
    m_pageStack = new QStackedWidget(content);

    m_changesPage = new ChangesPage(m_pageStack);
    m_pageStack->addWidget(m_changesPage);

    m_historyPage = new HistoryPage(m_pageStack);
    m_pageStack->addWidget(m_historyPage);

    m_stashPage = new StashPage(m_pageStack);
    m_pageStack->addWidget(m_stashPage);
    m_conflictPage = new ConflictPage(m_pageStack);
    m_pageStack->addWidget(m_conflictPage);

    m_pullRequestsPage = new PullRequestsPage(m_pageStack);
    m_pageStack->addWidget(m_pullRequestsPage);

    m_pageStack->addWidget(createSettingsPage());

    // 保持兼容成员引用
    m_fileChangeListView = m_changesPage->listView();
    m_commitHistoryListView = m_historyPage->listView();
    m_diffPreview = nullptr;              // 不再使用 QPlainTextEdit 显示 diff
    m_commitDetailView = nullptr;         // 由 HistoryPage 内部管理
    m_commitMessageEdit = nullptr;        // 由 CommitPanel 内部管理
    m_commitButton = nullptr;             // 由 CommitPanel 内部管理

    contentLayout->addWidget(m_pageTabs);
    contentLayout->addWidget(m_pageStack, 1);

    // Terminal panel — fixed on the right side
    m_terminalWidget = new TerminalWidget(central);
    m_terminalWidget->setObjectName(QStringLiteral("rightTerminalPanel"));
    m_terminalWidget->setMinimumWidth(200);
    m_terminalWidget->setMaximumWidth(500);

    auto* bodySplitter = new QSplitter(Qt::Horizontal, central);
    bodySplitter->setChildrenCollapsible(false);
    bodySplitter->addWidget(content);
    bodySplitter->addWidget(m_terminalWidget);
    bodySplitter->setStretchFactor(0, 1);
    bodySplitter->setStretchFactor(1, 0);
    bodySplitter->setSizes(QList<int>() << 900 << 300);

    rootLayout->addWidget(topBar);
    rootLayout->addWidget(createSeparator());
    rootLayout->addWidget(bodySplitter, 1);

    setCentralWidget(central);

    // ---- 页面切换 ----
    connect(m_pageTabs, &QTabBar::currentChanged,
            this, &MainWindow::handlePageTabChanged);

    // ---- 从子组件转发信号到 MainWindow ----

    // ChangesPage → MainWindow
    connect(m_changesPage, &ChangesPage::fileActivated,
            this, &MainWindow::fileChangeActivated);

    connect(m_changesPage, &ChangesPage::stageRequested,
            this, &MainWindow::stageRequested);
    connect(m_changesPage, &ChangesPage::unstageRequested,
            this, &MainWindow::unstageRequested);
    connect(m_changesPage, &ChangesPage::discardRequested,
            this, &MainWindow::discardRequested);
    connect(m_changesPage, &ChangesPage::ignoreRequested,
            this, &MainWindow::ignoreRequested);
    connect(m_changesPage, &ChangesPage::renameRequested,
            this, &MainWindow::renameRequested);
    connect(m_changesPage, &ChangesPage::stageAllRequested,
            this, &MainWindow::stageAllRequested);
    connect(m_changesPage, &ChangesPage::unstageAllRequested,
            this, &MainWindow::unstageAllRequested);

    connect(m_changesPage->commitPanel(), &CommitPanel::commitRequested,
            this, &MainWindow::commitRequested);

    // HistoryPage → MainWindow
    connect(m_historyPage, &HistoryPage::commitSelected,
            this, &MainWindow::commitActivated);

    // StashPage → MainWindow
    connect(m_stashPage, &StashPage::saveStashRequested,
            this, &MainWindow::saveStashRequested);
    connect(m_stashPage, &StashPage::applyStashRequested,
            this, &MainWindow::applyStashRequested);
    connect(m_stashPage, &StashPage::dropStashRequested,
            this, &MainWindow::dropStashRequested);
    connect(m_stashPage, &StashPage::showDiffRequested,
            this, &MainWindow::showStashDiffRequested);
    connect(m_stashPage, &StashPage::refreshRequested,
            this, &MainWindow::refreshRequested);

    // PullRequestsPage → MainWindow
    connect(m_pullRequestsPage, &PullRequestsPage::createPullRequestRequested,
            this, &MainWindow::createPullRequestRequested);
    connect(m_pullRequestsPage, &PullRequestsPage::mergeRequested,
            this, &MainWindow::mergePullRequestRequested);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel(tr("Ready"), this);
    m_busyLabel = new QLabel(tr("Ready"), this);
    m_loginStatusLabel = new QLabel(this);
    m_loginStatusLabel->setVisible(false);

    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_loginStatusLabel);
    statusBar()->addPermanentWidget(m_busyLabel);
}

QWidget* MainWindow::createRepositorySidebar()
{
    m_repositorySidebar = new RepositorySidebar(this);
    m_repositorySidebar->setObjectName(QStringLiteral("repositorySidebar"));
    m_repositorySidebar->setMinimumWidth(180);
    m_repositorySidebar->setMaximumWidth(300);

    connect(m_repositorySidebar, &RepositorySidebar::openRepositoryRequested,
            m_openRepositoryAction, &QAction::trigger);
    connect(m_repositorySidebar, &RepositorySidebar::cloneRepositoryRequested,
            m_cloneRepositoryAction, &QAction::trigger);
    connect(m_repositorySidebar, &RepositorySidebar::refreshRequested,
            m_refreshAction, &QAction::trigger);
    connect(m_repositorySidebar, &RepositorySidebar::repositoryActivated,
            this, &MainWindow::repositoryActivated);
    connect(m_repositorySidebar, &RepositorySidebar::removeRepositoryRequested,
            this, &MainWindow::removeRepositoryRequested);
    return m_repositorySidebar;
}

QWidget* MainWindow::createSettingsPage()
{
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(8);

    // ===== Git Identity =====
    auto* gitLabel = new QLabel(tr("Git Identity"));
    gitLabel->setObjectName(QStringLiteral("sectionLabel"));
    layout->addWidget(gitLabel);

    auto* gitBox = new QGroupBox(page);
    gitBox->setObjectName(QStringLiteral("settingsGroupBox"));
    auto* gitForm = new QFormLayout(gitBox);
    gitForm->setSpacing(6);

    auto* userNameEdit = new QLineEdit;
    userNameEdit->setPlaceholderText(tr("e.g. Zhang San"));
    gitForm->addRow(tr("User Name:"), userNameEdit);

    auto* userEmailEdit = new QLineEdit;
    userEmailEdit->setPlaceholderText(tr("e.g. zhangsan@example.com"));
    gitForm->addRow(tr("Email:"), userEmailEdit);

    layout->addWidget(gitBox);

    // ===== Default Path =====
    auto* pathLabel = new QLabel(tr("Default Path"));
    pathLabel->setObjectName(QStringLiteral("sectionLabel"));
    layout->addWidget(pathLabel);

    auto* pathBox = new QGroupBox(page);
    pathBox->setObjectName(QStringLiteral("settingsGroupBox"));
    auto* pathForm = new QFormLayout(pathBox);
    pathForm->setSpacing(6);

    auto* pathLayout = new QHBoxLayout;
    auto* defaultPathEdit = new QLineEdit;
    defaultPathEdit->setPlaceholderText(tr("Default directory for cloning repositories"));
    auto* browseBtn = new QPushButton(tr("Browse..."));
    browseBtn->setFixedWidth(90);
    pathLayout->addWidget(defaultPathEdit, 1);
    pathLayout->addWidget(browseBtn);
    pathForm->addRow(tr("Clone Path:"), pathLayout);

    connect(browseBtn, &QPushButton::clicked, this, [defaultPathEdit, this]() {
        const QString dir = QFileDialog::getExistingDirectory(
            this, tr("Select Default Clone Path"), defaultPathEdit->text());
        if (!dir.isEmpty()) defaultPathEdit->setText(dir);
    });

    layout->addWidget(pathBox);

    // ===== AI Configuration =====
    auto* aiLabel = new QLabel(tr("AI Configuration"));
    aiLabel->setObjectName(QStringLiteral("sectionLabel"));
    layout->addWidget(aiLabel);

    auto* aiBox = new QGroupBox(page);
    aiBox->setObjectName(QStringLiteral("settingsGroupBox"));
    auto* aiForm = new QFormLayout(aiBox);
    aiForm->setSpacing(6);

    auto* aiProviderCombo = new QComboBox;
    aiProviderCombo->setEditable(true);
    aiProviderCombo->addItem(QStringLiteral("OpenAI"),           QStringLiteral("openai"));
    aiProviderCombo->addItem(QStringLiteral("Anthropic"),        QStringLiteral("anthropic"));
    aiProviderCombo->addItem(QStringLiteral("Azure OpenAI"),     QStringLiteral("azure"));
    aiProviderCombo->addItem(QStringLiteral("Google Gemini"),    QStringLiteral("gemini"));
    aiProviderCombo->addItem(QStringLiteral("DeepSeek"),         QStringLiteral("deepseek"));
    aiProviderCombo->addItem(QStringLiteral("Moonshot (Kimi)"),  QStringLiteral("moonshot"));
    aiProviderCombo->addItem(QStringLiteral("Zhipu (GLM)"),      QStringLiteral("zhipu"));
    aiProviderCombo->addItem(QStringLiteral("Qwen (Tongyi)"),    QStringLiteral("qwen"));
    aiProviderCombo->addItem(QStringLiteral("Ollama (Local)"),   QStringLiteral("ollama"));
    aiProviderCombo->addItem(QStringLiteral("Custom / Other"),   QStringLiteral("custom"));
    aiForm->addRow(tr("Provider:"), aiProviderCombo);

    auto* aiApiKeyEdit = new QLineEdit;
    aiApiKeyEdit->setEchoMode(QLineEdit::Password);
    aiApiKeyEdit->setPlaceholderText(tr("API key (e.g. sk-...)"));
    aiForm->addRow(tr("API Key:"), aiApiKeyEdit);

    auto* aiModelEdit = new QLineEdit;
    aiModelEdit->setPlaceholderText(tr("Model name (e.g. gpt-4o)"));
    aiForm->addRow(tr("Model:"), aiModelEdit);

    layout->addWidget(aiBox);

    // ===== Account =====
    auto* acctLabel = new QLabel(tr("Code Hosting Account"));
    acctLabel->setObjectName(QStringLiteral("sectionLabel"));
    layout->addWidget(acctLabel);

    auto* acctBox = new QGroupBox(page);
    acctBox->setObjectName(QStringLiteral("settingsGroupBox"));
    auto* acctLayout = new QVBoxLayout(acctBox);
    acctLayout->setSpacing(6);

    auto* loginStatusLabel = new QLabel(tr("Not logged in"));
    loginStatusLabel->setObjectName(QStringLiteral("settingsLoginStatus"));
    acctLayout->addWidget(loginStatusLabel);

    auto* loginBtn = new QPushButton(tr("GitHub / Gitee / GitLab Login..."));
    loginBtn->setObjectName(QStringLiteral("settingsLoginButton"));
    connect(loginBtn, &QPushButton::clicked, this, &MainWindow::loginRequested);
    acctLayout->addWidget(loginBtn);

    layout->addWidget(acctBox);

    // ===== Save Button =====
    auto* saveBtn = new QPushButton(tr("Save Settings"));
    saveBtn->setObjectName(QStringLiteral("settingsSaveButton"));
    layout->addWidget(saveBtn);

    // Connect save: read all fields and write to SettingsService + git config
    connect(saveBtn, &QPushButton::clicked, saveBtn, [=]() {
        if (!m_settingsService) return;

        const QString userName = userNameEdit->text().trimmed();
        const QString userEmail = userEmailEdit->text().trimmed();

        // 写入 QSettings 作为持久化缓存
        m_settingsService->setValue(QStringLiteral("git/userName"), userName);
        m_settingsService->setValue(QStringLiteral("git/userEmail"), userEmail);
        m_settingsService->setDefaultClonePath(defaultPathEdit->text().trimmed());
        m_settingsService->setAIProvider(aiProviderCombo->currentData().toString().isEmpty()
            ? aiProviderCombo->currentText().trimmed()
            : aiProviderCombo->currentData().toString());
        m_settingsService->setAIApiKey(aiApiKeyEdit->text().trimmed());
        m_settingsService->setAIModel(aiModelEdit->text().trimmed());
        m_settingsService->sync();

        // 写入 git config --global，让 git commit 能识别用户身份
        if (m_gitService) {
            if (!userName.isEmpty()) {
                const auto r = m_gitService->writeConfig(
                    QStringLiteral("user.name"), userName, QString(), true);
                if (r.isFailure()) {
                    setStatusMessage(tr("Warning: failed to set git user.name - %1")
                        .arg(r.errorMessage()), 4000);
                }
            }
            if (!userEmail.isEmpty()) {
                const auto r = m_gitService->writeConfig(
                    QStringLiteral("user.email"), userEmail, QString(), true);
                if (r.isFailure()) {
                    setStatusMessage(tr("Warning: failed to set git user.email - %1")
                        .arg(r.errorMessage()), 4000);
                }
            }
        }

        setStatusMessage(tr("Settings saved."), 3000);
        saveBtn->setText(tr("✓ Saved"));
        QTimer::singleShot(2000, saveBtn, [saveBtn]() {
            saveBtn->setText(QObject::tr("Save Settings"));
        });
    });

    // Populate fields from existing settings (deferred — m_settingsService is
    // set after MainWindow construction, so we wait for the event loop)
    QTimer::singleShot(0, saveBtn, [this, userNameEdit, userEmailEdit, defaultPathEdit,
                                     aiProviderCombo, aiApiKeyEdit, aiModelEdit]() {
        if (!m_settingsService) return;

        // 优先从 git config --global 读取（Git 的真正配置来源）
        QString gitUserName;
        QString gitUserEmail;
        if (m_gitService) {
            const auto nameResult = m_gitService->readConfig(
                QStringLiteral("user.name"), QString(), true);
            if (nameResult.isSuccess() && !nameResult.value().isEmpty()) {
                gitUserName = nameResult.value();
            }
            const auto emailResult = m_gitService->readConfig(
                QStringLiteral("user.email"), QString(), true);
            if (emailResult.isSuccess() && !emailResult.value().isEmpty()) {
                gitUserEmail = emailResult.value();
            }
        }

        // 以 git config 为准，QSettings 作为 fallback
        userNameEdit->setText(!gitUserName.isEmpty()
            ? gitUserName
            : m_settingsService->value(QStringLiteral("git/userName")).toString());
        userEmailEdit->setText(!gitUserEmail.isEmpty()
            ? gitUserEmail
            : m_settingsService->value(QStringLiteral("git/userEmail")).toString());
        defaultPathEdit->setText(m_settingsService->defaultClonePath());
        const int idx = aiProviderCombo->findData(m_settingsService->aiProvider());
        if (idx >= 0) aiProviderCombo->setCurrentIndex(idx);
        else aiProviderCombo->setEditText(m_settingsService->aiProvider());
        aiApiKeyEdit->setText(m_settingsService->aiApiKey());
        aiModelEdit->setText(m_settingsService->aiModel());
    });

    layout->addStretch(1);
    scroll->setWidget(page);
    return scroll;
}

// ========== 内部辅助 ==========

void MainWindow::updateRepositoryHeader()
{
    const bool hasRepository = !m_currentRepositoryName.isEmpty()
                               || !m_currentRepositoryPath.isEmpty();

    const QString name = !m_currentRepositoryName.isEmpty()
                             ? m_currentRepositoryName
                             : tr("No repository selected");

    const QString branch = hasRepository
                               ? m_currentRepositoryBranch.isEmpty()
                                     ? tr("detached HEAD") : m_currentRepositoryBranch
                               : QStringLiteral("-");

    auto* repoBtn = findChild<QPushButton*>(QStringLiteral("repoMenuButton"));
    if (repoBtn) {
        updateHeaderValue(repoBtn, name);
        repoBtn->setToolTip(m_currentRepositoryPath);
    }

    // Update branch dropdown button text
    auto* branchBtn = findChild<QPushButton*>(QStringLiteral("branchDropdownButton"));
    if (branchBtn)
        updateHeaderValue(branchBtn, branch);

    m_refreshAction->setEnabled(hasRepository && !m_busy);
    m_fetchAction->setEnabled(hasRepository && !m_busy);
    m_pullAction->setEnabled(hasRepository && !m_busy);
    m_pushAction->setEnabled(hasRepository && !m_busy);
}

void MainWindow::updateBusyState()
{
    const bool hasRepository = !m_currentRepositoryName.isEmpty()
                               || !m_currentRepositoryPath.isEmpty();

    m_openRepositoryAction->setEnabled(!m_busy);
    m_cloneRepositoryAction->setEnabled(!m_busy);
    m_initRepositoryAction->setEnabled(!m_busy);
    m_refreshAction->setEnabled(hasRepository && !m_busy);
    m_fetchAction->setEnabled(hasRepository && !m_busy);
    m_pullAction->setEnabled(hasRepository && !m_busy);
    m_pushAction->setEnabled(hasRepository && !m_busy);

    if (m_repositorySidebar) {
        m_repositorySidebar->setBusy(m_busy);
    }

    if (m_busyLabel) {
        m_busyLabel->setText(m_busy ? tr("Busy") : tr("Ready"));
    }
}
