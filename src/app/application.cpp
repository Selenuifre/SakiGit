#include "application.h"

#include "appcontext.h"
#include "controllers/AICommitController.h"
#include "controllers/branchcontroller.h"
#include "services/AIService.h"
#include "controllers/changescontroller.h"
#include "controllers/CodeReviewController.h"
#include "controllers/historycontroller.h"
#include "controllers/maincontroller.h"
#include "controllers/MergeController.h"
#include "controllers/RebaseController.h"
#include "controllers/repositorycontroller.h"
#include "controllers/RemoteController.h"
#include "controllers/StashController.h"
#include "controllers/TerminalController.h"
#include "controllers/synccontroller.h"
#include "models/TerminalOutputModel.h"
#include "domain/commit.h"
#include "domain/diff.h"
#include "domain/filechange.h"
#include "domain/repository.h"
#include "infrastructure/logger.h"
#include "theme/ThemeManager.h"
#include "models/branchlistmodel.h"
#include "models/commithistorymodel.h"
#include "models/ConflictFileModel.h"
#include "models/difflinemodel.h"
#include "models/filechangemodel.h"
#include "models/repositorylistmodel.h"
#include "models/PullRequestModel.h"
#include "models/RemoteListModel.h"
#include "models/StashListModel.h"
#include "controllers/AuthController.h"
#include "controllers/PullRequestController.h"
#include "infrastructure/ICodeHostingProvider.h"
#include "services/gitservice.h"
#include "services/ProviderFactory.h"
#include "services/settingsservice.h"
#include "ui/changespage.h"
#include "ui/clonedialog.h"
#include "ui/commitpanel.h"
#include "ui/ConflictPage.h"
#include "ui/diffviewer.h"
#include "models/difflinemodel.h"
#include "ui/LoginDialog.h"
#include "ui/ConflictResolver.h"
#include "ui/historypage.h"
#include "ui/mainwindow.h"
#include "ui/TerminalWidget.h"
#include "ui/PullRequestsPage.h"
#include "ui/preferencesdialog.h"
#include "ui/ReviewPanel.h"
#include "models/ReviewFindingModel.h"
#include "ui/toastmanager.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QModelIndex>
#include <QPushButton>
#include <QStandardPaths>
#include <QStringList>

namespace {
const char* OrganizationName = "SakiGit";

// 用于从 git remote URL 解析 owner / repo / provider 的辅助结构
struct RemoteRepoInfo {
    QString owner;
    QString repoName;
    ICodeHostingProvider* provider = nullptr;

    bool isValid() const { return provider != nullptr && !owner.isEmpty(); }
};

// 根据仓库路径从 origin remote URL 解析出平台、owner 和 repo 名称
RemoteRepoInfo resolveRemoteRepo(AppContext* ctx, const QString& repoPath)
{
    RemoteRepoInfo info;
    if (!ctx || !ctx->gitService()) return info;

    const auto urlResult = ctx->gitService()->executor().remoteUrl(repoPath);
    if (urlResult.isFailure()) return info;

    const QString remoteUrl = urlResult.value();
    info.provider = ctx->providerForRemote(remoteUrl);
    if (!info.provider) return info;

    const auto parseResult = info.provider->parseRemoteUrl(remoteUrl);
    if (parseResult.isSuccess()) {
        info.owner = parseResult.value().owner;
        info.repoName = parseResult.value().repoName;
    }

    return info;
}

const char* ApplicationName = "SakiGit";
const char* ApplicationVersion = "0.1.0";

QString defaultLogFilePath()
{
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

    if (basePath.trimmed().isEmpty()) {
        basePath = QDir::homePath() + QStringLiteral("/.sakigit");
    }

    return QDir(basePath).filePath(QStringLiteral("logs/sakigit.log"));
}

QString formatCommitDetail(const Commit& commit)
{
    QStringList lines;

    lines << QStringLiteral("Commit: %1").arg(commit.hash());
    lines << QStringLiteral("Author: %1 <%2>").arg(commit.authorName(), commit.authorEmail());
    lines << QStringLiteral("Author Date: %1").arg(commit.authorDate().toString(Qt::ISODate));
    lines << QStringLiteral("Committer: %1 <%2>").arg(commit.committerName(), commit.committerEmail());
    lines << QStringLiteral("Committer Date: %1").arg(commit.committerDate().toString(Qt::ISODate));

    if (!commit.parentHashes().isEmpty()) {
        lines << QStringLiteral("Parents: %1").arg(commit.parentHashes().join(QStringLiteral(" ")));
    }

    lines << QString();
    lines << commit.summary();

    if (commit.hasBody()) {
        lines << QString();
        lines << commit.body();
    }

    if (!commit.changedFiles().isEmpty()) {
        lines << QString();
        lines << QStringLiteral("Changed files:");
        for (const QString& filePath : commit.changedFiles()) {
            lines << QStringLiteral("  %1").arg(filePath);
        }
    }

    return lines.join(QLatin1Char('\n'));
}
}

Application::Application(QApplication& qtApplication)
    : m_qtApplication(qtApplication),
    m_initialized(false),
    m_loggerInstalled(false)
{
}

Application::~Application()
{
    shutdown();
}

Result<void> Application::initialize()
{
    if (m_initialized) {
        return Result<void>::success();
    }

    configureApplicationMetadata();

    // Initialize theme system (Light theme by default)
    ThemeManager::instance()->initialize(ThemeManager::Light);

    initializeLogger();
    initializeAppContext();
    initializeMainWindow();
    connectApplicationSignals();

    m_initialized = true;
    m_lastError.clear();

    Logger::instance().info(QStringLiteral("Application initialized"), QStringLiteral("Application"));
    return Result<void>::success();
}

Result<void> Application::showMainWindow()
{
    const Result<void> initializeResult = initialize();

    if (initializeResult.isFailure()) {
        setLastError(initializeResult.errorMessage());
        return initializeResult;
    }

    if (!m_mainWindow) {
        const QString errorMessage = QStringLiteral("Main window is not available.");
        setLastError(errorMessage);
        return Result<void>::failure(errorMessage);
    }

    m_mainWindow->show();
    m_mainWindow->raise();
    m_mainWindow->activateWindow();

    return Result<void>::success();
}

int Application::run()
{
    const Result<void> showResult = showMainWindow();

    if (showResult.isFailure()) {
        Logger::instance().error(showResult.errorMessage(), QStringLiteral("Application"));
        return 1;
    }

    return m_qtApplication.exec();
}

void Application::shutdown()
{
    if (!m_initialized && !m_loggerInstalled) {
        return;
    }

    saveMainWindowState();

    if (m_aboutToQuitConnection) {
        QObject::disconnect(m_aboutToQuitConnection);
        m_aboutToQuitConnection = QMetaObject::Connection();
    }

    // 先销毁主窗口（UI 层），再销毁 AppContext（Service / Model 层）
    m_mainWindow.reset();

    if (m_appContext) {
        m_appContext->shutdown();
        m_appContext.reset();
    }

    if (m_loggerInstalled) {
        Logger::uninstallQtMessageHandler();
        m_loggerInstalled = false;
    }

    m_initialized = false;
}

bool Application::isInitialized() const
{
    return m_initialized;
}

QApplication& Application::qtApplication() const
{
    return m_qtApplication;
}

SettingsService* Application::settingsService() const
{
    return m_appContext ? m_appContext->settingsService() : nullptr;
}

GitService* Application::gitService() const
{
    return m_appContext ? m_appContext->gitService() : nullptr;
}

GitTaskRunner* Application::gitTaskRunner() const
{
    return m_appContext ? m_appContext->gitTaskRunner() : nullptr;
}

MainWindow* Application::mainWindow() const
{
    return m_mainWindow.get();
}

AppContext* Application::appContext() const
{
    return m_appContext.get();
}

QString Application::lastError() const
{
    return m_lastError;
}

void Application::updateGitExecutablePath(const QString& gitExecutablePath)
{
    if (m_appContext) {
        m_appContext->updateGitExecutablePath(gitExecutablePath.trimmed());
    }
}

void Application::configureApplicationMetadata()
{
    QCoreApplication::setOrganizationName(QString::fromLatin1(OrganizationName));
    QCoreApplication::setApplicationName(QString::fromLatin1(ApplicationName));
    QCoreApplication::setApplicationVersion(QString::fromLatin1(ApplicationVersion));
}

void Application::initializeLogger()
{
    Logger& logger = Logger::instance();
    logger.setConsoleEnabled(true);
    logger.setLogFilePath(defaultLogFilePath());
    logger.setFileEnabled(true);
    logger.setMinimumLevel(Logger::Level::Debug);

    if (!m_loggerInstalled) {
        Logger::installQtMessageHandler();
        m_loggerInstalled = true;
    }
}

void Application::initializeAppContext()
{
    m_appContext = std::make_unique<AppContext>();
    m_appContext->initialize();

    Logger::instance().info(QStringLiteral("AppContext initialized"), QStringLiteral("Application"));
}

void Application::initializeMainWindow()
{
    m_mainWindow = std::make_unique<MainWindow>();

    if (m_appContext) {
        m_appContext->setMainWindow(m_mainWindow.get());
    }

    // Pass SettingsService to MainWindow for inline settings page
    if (m_appContext && m_appContext->settingsService()) {
        m_mainWindow->setSettingsService(m_appContext->settingsService());
    }
    // Pass GitService to MainWindow for settings page git config read/write
    if (m_appContext && m_appContext->gitService()) {
        m_mainWindow->setGitService(m_appContext->gitService());
    }

    bindMainWindowModels();
    connectMainWindowSignals();
    connectControllerSignals();

    // 尝试恢复上次登录状态（依次尝试各平台，失败时静默忽略，用户可手动登录）
    if (m_appContext->providerFactory()) {
        const auto providers = m_appContext->providerFactory()->allProviders();
        for (auto* p : providers) {
            const auto result = m_appContext->authController()->restoreLogin(p);
            if (result.isSuccess()) break;
        }
    }

    restoreMainWindowState();
}

void Application::bindMainWindowModels()
{
    if (!m_appContext || !m_mainWindow) {
        return;
    }

    m_mainWindow->setRepositoryModel(m_appContext->repositoryListModel());
    m_mainWindow->setFileChangeModel(m_appContext->fileChangeModel());
    m_mainWindow->setDiffLineModel(m_appContext->diffLineModel());
    m_mainWindow->setCommitHistoryModel(m_appContext->commitHistoryModel());
    m_mainWindow->setBranchModel(m_appContext->branchListModel());
    m_mainWindow->setStashListModel(m_appContext->stashListModel());
    m_mainWindow->setRemoteModel(m_appContext->remoteListModel());
    m_mainWindow->setTerminalModel(m_appContext->terminalOutputModel());
    m_mainWindow->setPullRequestModel(m_appContext->pullRequestModel());

    // 提交树形图模型绑定
    if (m_mainWindow->historyPage() && m_appContext->commitGraphModel()) {
        m_mainWindow->historyPage()->setGraphModel(m_appContext->commitGraphModel());
    }

    if (m_mainWindow->conflictPage()) {
        m_mainWindow->conflictPage()->setModel(m_appContext->conflictFileModel());
    }
}

void Application::connectMainWindowSignals()
{
    if (!m_appContext || !m_mainWindow) {
        return;
    }

    MainWindow* window = m_mainWindow.get();

    QObject::connect(window, &MainWindow::openRepositoryRequested, window, [this]() {
        SettingsService* settings = m_appContext ? m_appContext->settingsService() : nullptr;
        const QString startPath = settings && !settings->lastOpenedRepositoryPath().isEmpty()
                                      ? settings->lastOpenedRepositoryPath()
                                      : QDir::homePath();

        const QString path = QFileDialog::getExistingDirectory(
            m_mainWindow.get(),
            QStringLiteral("Open Repository"),
            startPath);

        if (path.isEmpty()) {
            return;
        }

        if (settings) {
            settings->setLastOpenedRepositoryPath(path);
        }

        showInfo(QStringLiteral("Opening repository..."));
        m_appContext->repositoryController()->openRepository(path);
    });

    QObject::connect(window, &MainWindow::cloneRepositoryRequested, window, [this]() {
        CloneDialog dialog(m_mainWindow.get());

        SettingsService* settings = m_appContext ? m_appContext->settingsService() : nullptr;
        const QString defaultPath = settings && !settings->defaultClonePath().isEmpty()
                                        ? settings->defaultClonePath()
                                        : QDir::homePath();
        dialog.setDefaultPath(defaultPath);

        if (dialog.exec() != QDialog::Accepted) {
            return;
        }

        if (settings) {
            settings->setDefaultClonePath(dialog.targetPath());
        }

        showInfo(QStringLiteral("Cloning repository..."));
        m_appContext->repositoryController()->cloneRepository(dialog.url(), dialog.targetPath());
    });

    // ---- Init Repository ----
    QObject::connect(window, &MainWindow::initRepositoryRequested, window, [this]() {
        SettingsService* settings = m_appContext ? m_appContext->settingsService() : nullptr;
        const QString startPath = settings && !settings->lastOpenedRepositoryPath().isEmpty()
                                      ? settings->lastOpenedRepositoryPath()
                                      : QDir::homePath();

        const QString path = QFileDialog::getExistingDirectory(
            m_mainWindow.get(),
            QStringLiteral("Select Directory to Initialize"),
            startPath);

        if (path.isEmpty()) {
            return;
        }

        if (settings) {
            settings->setLastOpenedRepositoryPath(path);
        }

        showInfo(QStringLiteral("Initializing repository..."));
        m_appContext->repositoryController()->initRepository(path);
    });

    // ---- 偏好设置 → 切换到内嵌设置页面 ----
    QObject::connect(window, &MainWindow::preferencesRequested, window, [this]() {
        m_mainWindow->setCurrentPage(MainWindow::SettingsPageIdx);
    });

    QObject::connect(window, &MainWindow::refreshRequested, window, [this]() {
        if (!ensureCurrentRepository(QStringLiteral("Refresh"))) {
            return;
        }

        showInfo(QStringLiteral("Refreshing repository..."));
        m_appContext->mainController()->refreshAll();
    });

    QObject::connect(window, &MainWindow::fetchRequested, window, [this]() {
        if (!ensureCurrentRepository(QStringLiteral("Fetch"))) {
            return;
        }

        showInfo(QStringLiteral("Fetching..."));
        const QString remote = m_mainWindow->selectedRemote();
        m_appContext->syncController()->fetch(currentRepositoryPath(),
            remote.isEmpty() ? QStringLiteral("origin") : remote);
    });

    QObject::connect(window, &MainWindow::pullRequested, window, [this]() {
        if (!ensureCurrentRepository(QStringLiteral("Pull"))) {
            return;
        }

        showInfo(QStringLiteral("Pulling..."));
        m_appContext->syncController()->pull(currentRepositoryPath());
    });

    QObject::connect(window, &MainWindow::pushRequested, window, [this]() {
        if (!ensureCurrentRepository(QStringLiteral("Push"))) {
            return;
        }

        showInfo(QStringLiteral("Pushing..."));
        const QString branchName = currentBranchNameForPath(currentRepositoryPath());
        const QString remote = m_mainWindow->selectedRemote();
        m_appContext->syncController()->push(currentRepositoryPath(),
            remote.isEmpty() ? QStringLiteral("origin") : remote, branchName);
    });

    QObject::connect(window, &MainWindow::repositoryActivated, window, [this](const QModelIndex& index) {
        RepositoryListModel* model = m_appContext ? m_appContext->repositoryListModel() : nullptr;
        if (!model || !index.isValid()) {
            return;
        }

        const Repository repository = model->repositoryAt(index.row());
        if (!repository.localPath().isEmpty()) {
            activateRepository(repository);
        }
    });

    QObject::connect(window, &MainWindow::removeRepositoryRequested, window, [this](const QModelIndex& index) {
        RepositoryListModel* model = m_appContext ? m_appContext->repositoryListModel() : nullptr;
        if (!model || !index.isValid()) {
            return;
        }

        const Repository repository = model->repositoryAt(index.row());
        if (!repository.localPath().isEmpty()) {
            m_appContext->removeRecentRepository(repository.localPath());
            showInfo(QStringLiteral("Repository removed from recent list."));
        }
    });

    QObject::connect(window, &MainWindow::fileChangeActivated, window, [this](const QModelIndex& index, bool staged) {
        if (!ensureCurrentRepository(QStringLiteral("Load diff"))) {
            return;
        }

        FileChangeModel* model = m_appContext ? m_appContext->fileChangeModel() : nullptr;
        if (!model || !index.isValid()) {
            return;
        }

        const FileChange change = model->fileChangeAt(index.row());
        if (!change.path().isEmpty()) {
            // 冲突文件切换到 ConflictPage 处理
            if (change.isConflict()) {
                m_mainWindow->setCurrentPage(MainWindow::ConflictPageIdx);
                if (m_mainWindow->conflictPage()) {
                    m_mainWindow->conflictPage()->showConflictFile(
                        currentRepositoryPath(), change.path());
                }
            } else {
                m_mainWindow->setCurrentPage(MainWindow::ChangesPageIdx);
                m_mainWindow->changesPage()->showDiffView();
                m_appContext->changesController()->loadDiff(
                    currentRepositoryPath(), change.path(), staged);
            }
        }
    });

    QObject::connect(window, &MainWindow::commitActivated, window, [this](const QModelIndex& index) {
        if (!ensureCurrentRepository(QStringLiteral("Load commit diff"))) {
            return;
        }

        CommitHistoryModel* model = m_appContext ? m_appContext->commitHistoryModel() : nullptr;
        if (!model || !index.isValid()) {
            return;
        }

        const Commit commit = model->commitAt(index.row());
        if (!commit.hash().isEmpty()) {
            m_mainWindow->setCommitDetail(formatCommitDetail(commit));
            m_appContext->historyController()->loadCommitDiff(currentRepositoryPath(), commit.hash());
        }
    });

    QObject::connect(window, &MainWindow::stageRequested, window, [this](const QModelIndex& index) {
        if (!ensureCurrentRepository(QStringLiteral("Stage file"))) {
            return;
        }

        FileChangeModel* model = m_appContext ? m_appContext->fileChangeModel() : nullptr;
        if (!model || !index.isValid()) {
            return;
        }

        const FileChange change = model->fileChangeAt(index.row());
        if (!change.path().isEmpty()) {
            m_appContext->changesController()->stageFile(currentRepositoryPath(), change.path());
        }
    });

    QObject::connect(window, &MainWindow::unstageRequested, window, [this](const QModelIndex& index) {
        if (!ensureCurrentRepository(QStringLiteral("Unstage file"))) {
            return;
        }

        FileChangeModel* model = m_appContext ? m_appContext->fileChangeModel() : nullptr;
        if (!model || !index.isValid()) {
            return;
        }

        const FileChange change = model->fileChangeAt(index.row());
        if (!change.path().isEmpty()) {
            m_appContext->changesController()->unstageFile(currentRepositoryPath(), change.path());
        }
    });

    QObject::connect(window, &MainWindow::discardRequested, window, [this](const QModelIndex& index) {
        if (!ensureCurrentRepository(QStringLiteral("Discard changes"))) {
            return;
        }

        FileChangeModel* model = m_appContext ? m_appContext->fileChangeModel() : nullptr;
        if (!model || !index.isValid()) {
            return;
        }

        const FileChange change = model->fileChangeAt(index.row());
        if (change.path().isEmpty()) {
            return;
        }

        const auto btn = QMessageBox::warning(
            m_mainWindow.get(),
            QStringLiteral("Discard Changes"),
            QStringLiteral("This will permanently discard all changes to \"%1\".\n\nThis action cannot be undone. Continue?")
                .arg(change.path()),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (btn != QMessageBox::Yes) {
            return;
        }

        const auto result = m_appContext->changesController()->discardChanges(
            currentRepositoryPath(), change.path());
        if (result.isFailure()) {
            showError(QStringLiteral("Discard changes"), result.errorMessage());
        }
    });

    QObject::connect(window, &MainWindow::ignoreRequested, window, [this](const QModelIndex& index) {
        if (!ensureCurrentRepository(QStringLiteral("Ignore file"))) {
            return;
        }

        FileChangeModel* model = m_appContext ? m_appContext->fileChangeModel() : nullptr;
        if (!model || !index.isValid()) {
            return;
        }

        const FileChange change = model->fileChangeAt(index.row());
        if (change.path().isEmpty()) {
            return;
        }

        const auto result = m_appContext->changesController()->ignoreFile(
            currentRepositoryPath(), change.path());
        if (result.isSuccess()) {
            showInfo(QStringLiteral("Added \"%1\" to .gitignore").arg(change.path()));
        } else {
            showError(QStringLiteral("Ignore file"), result.errorMessage());
        }
    });

    QObject::connect(window, &MainWindow::renameRequested, window, [this](const QModelIndex& index) {
        if (!ensureCurrentRepository(QStringLiteral("Rename file"))) {
            return;
        }

        FileChangeModel* model = m_appContext ? m_appContext->fileChangeModel() : nullptr;
        if (!model || !index.isValid()) {
            return;
        }

        const FileChange change = model->fileChangeAt(index.row());
        if (change.path().isEmpty()) {
            return;
        }

        const QString oldPath = change.path();
        // 从仓库相对路径中提取文件名和父目录
        const int lastSlash = oldPath.lastIndexOf(QLatin1Char('/'));
        const QString oldName = (lastSlash >= 0) ? oldPath.mid(lastSlash + 1) : oldPath;
        const QString parentDir = (lastSlash >= 0) ? oldPath.left(lastSlash) : QString();

        bool ok = false;
        const QString newName = QInputDialog::getText(
            m_mainWindow.get(),
            QStringLiteral("Rename File"),
            QStringLiteral("New name for \"%1\":"),
            QLineEdit::Normal,
            oldName,
            &ok);

        if (!ok || newName.isEmpty() || newName == oldName) {
            return;
        }

        // 保持仓库相对路径
        const QString newPath = parentDir.isEmpty()
            ? newName
            : parentDir + QStringLiteral("/") + newName;
        const auto result = m_appContext->changesController()->renameFile(
            currentRepositoryPath(), oldPath, newPath);
        if (result.isFailure()) {
            showError(QStringLiteral("Rename file"), result.errorMessage());
        }
    });

    QObject::connect(window, &MainWindow::stageAllRequested, window, [this]() {
        if (ensureCurrentRepository(QStringLiteral("Stage all"))) {
            // 注意: stageAllFiles 通过 GitTaskRunner 异步执行，
            // 实际结果由 ChangesController::onAllFilesStaged 处理后
            // 通过文件变更列表的自动刷新体现，不在此处检查返回值
            m_appContext->changesController()->stageAllFiles(currentRepositoryPath());
        }
    });

    QObject::connect(window, &MainWindow::unstageAllRequested, window, [this]() {
        if (ensureCurrentRepository(QStringLiteral("Unstage all"))) {
            const auto result = m_appContext->changesController()->unstageAllFiles(currentRepositoryPath());
            if (result.isSuccess()) {
                showInfo(QStringLiteral("All files unstaged."));
            }
        }
    });

    QObject::connect(window, &MainWindow::commitRequested, window, [this](const QString& message) {
        if (!ensureCurrentRepository(QStringLiteral("Commit"))) {
            return;
        }

        FileChangeModel* model = m_appContext ? m_appContext->fileChangeModel() : nullptr;
        if (!model || model->stagedCount() <= 0) {
            showError(QStringLiteral("Commit"),
                      QStringLiteral("No staged changes. Stage files before committing."));
            return;
        }

        m_appContext->changesController()->commit(currentRepositoryPath(), message);
    });

    QObject::connect(window, &MainWindow::createBranchRequested, window, [this](const QString& name) {
        if (ensureCurrentRepository(QStringLiteral("Create branch"))) {
            m_appContext->branchController()->createBranch(currentRepositoryPath(), name);
        }
    });

    QObject::connect(window, &MainWindow::deleteBranchRequested, window, [this](const QString& name) {
        if (ensureCurrentRepository(QStringLiteral("Delete branch"))) {
            m_appContext->branchController()->deleteBranch(currentRepositoryPath(), name);
        }
    });

    QObject::connect(window, &MainWindow::checkoutRequested, window, [this](const QString& name) {
        if (ensureCurrentRepository(QStringLiteral("Checkout branch"))) {
            m_appContext->branchController()->checkoutBranch(currentRepositoryPath(), name);
        }
    });

    QObject::connect(window, &MainWindow::mergeRequested, window, [this](const QString& /*name*/) {
        if (ensureCurrentRepository(QStringLiteral("Merge branch"))) {
            // 使用增强版 Merge 流程：弹出 MergeDialog 让用户选择目标分支
            m_appContext->branchController()->showMergeDialog(
                currentRepositoryPath(),
                currentBranchNameForPath(currentRepositoryPath()));
        }
    });

    // ---- Stash 操作 ----
    QObject::connect(window, &MainWindow::saveStashRequested, window, [this](const QString& message) {
        if (!ensureCurrentRepository(QStringLiteral("Save stash"))) {
            return;
        }

        showInfo(QStringLiteral("Saving stash..."));
        const auto result = m_appContext->stashController()->saveStash(currentRepositoryPath(), message);
        if (result.isFailure()) {
            showError(QStringLiteral("Save stash"), result.errorMessage());
        }
    });

    QObject::connect(window, &MainWindow::applyStashRequested, window, [this](int index) {
        if (!ensureCurrentRepository(QStringLiteral("Apply stash"))) {
            return;
        }

        showInfo(QStringLiteral("Applying stash..."));
        const auto result = m_appContext->stashController()->applyStash(currentRepositoryPath(), index);
        if (result.isFailure()) {
            showError(QStringLiteral("Apply stash"), result.errorMessage());
        }
    });

    QObject::connect(window, &MainWindow::dropStashRequested, window, [this](int index) {
        if (!ensureCurrentRepository(QStringLiteral("Drop stash"))) {
            return;
        }

        showInfo(QStringLiteral("Dropping stash..."));
        const auto result = m_appContext->stashController()->dropStash(currentRepositoryPath(), index);
        if (result.isFailure()) {
            showError(QStringLiteral("Drop stash"), result.errorMessage());
        }
    });

    QObject::connect(window, &MainWindow::showStashDiffRequested, window, [this](int index) {
        if (!ensureCurrentRepository(QStringLiteral("Show stash diff"))) {
            return;
        }

        showInfo(QStringLiteral("Loading stash diff..."));
        m_appContext->stashController()->showStashDiff(currentRepositoryPath(), index);
    });

    // ---- Remote 操作 ----
    QObject::connect(window, &MainWindow::addRemoteRequested, window, [this](const QString& name, const QString& url) {
        if (!ensureCurrentRepository(QStringLiteral("Add remote"))) {
            return;
        }

        showInfo(QStringLiteral("Adding remote..."));
        const auto result = m_appContext->remoteController()->addRemote(currentRepositoryPath(), name, url);
        if (result.isFailure()) {
            showError(QStringLiteral("Add remote"), result.errorMessage());
        }
    });

    QObject::connect(window, &MainWindow::removeRemoteRequested, window, [this](const QString& name) {
        if (!ensureCurrentRepository(QStringLiteral("Remove remote"))) {
            return;
        }

        showInfo(QStringLiteral("Removing remote..."));
        const auto result = m_appContext->remoteController()->removeRemote(currentRepositoryPath(), name);
        if (result.isFailure()) {
            showError(QStringLiteral("Remove remote"), result.errorMessage());
        }
    });

    QObject::connect(window, &MainWindow::renameRemoteRequested, window, [this](const QString& oldName, const QString& newName) {
        if (!ensureCurrentRepository(QStringLiteral("Rename remote"))) {
            return;
        }

        showInfo(QStringLiteral("Renaming remote..."));
        const auto result = m_appContext->remoteController()->renameRemote(currentRepositoryPath(), oldName, newName);
        if (result.isFailure()) {
            showError(QStringLiteral("Rename remote"), result.errorMessage());
        }
    });

    QObject::connect(window, &MainWindow::setRemoteUrlRequested, window, [this](const QString& name, const QString& url) {
        if (!ensureCurrentRepository(QStringLiteral("Set remote URL"))) {
            return;
        }

        showInfo(QStringLiteral("Updating remote URL..."));
        const auto result = m_appContext->remoteController()->setRemoteUrl(currentRepositoryPath(), name, url);
        if (result.isFailure()) {
            showError(QStringLiteral("Set remote URL"), result.errorMessage());
        }
    });

    // ---- Rebase 操作 ----
    QObject::connect(window, &MainWindow::rebaseRequested, window, [this](const QString& /*name*/) {
        if (ensureCurrentRepository(QStringLiteral("Rebase branch"))) {
            m_appContext->branchController()->showRebaseDialog(
                currentRepositoryPath(),
                currentBranchNameForPath(currentRepositoryPath()));
        }
    });

    // ---- 树形图右键菜单操作 ----
    // HistoryPage 信号 → MainWindow 信号转发
    if (auto* hp = window->historyPage()) {
        QObject::connect(hp, &HistoryPage::checkoutCommitRequested, window, &MainWindow::checkoutCommitRequested);
        QObject::connect(hp, &HistoryPage::createBranchAtCommitRequested, window, &MainWindow::createBranchAtCommitRequested);
        QObject::connect(hp, &HistoryPage::resetToCommitRequested, window, &MainWindow::resetToCommitRequested);

        // File-in-commit click → load per-file commit diff into DiffViewer
        QObject::connect(hp, &HistoryPage::fileInCommitClicked, window,
                         [this](const QString& commitHash, const QString& filePath) {
            const QString repoPath = currentRepositoryPath();
            if (repoPath.isEmpty()) return;
            const auto raw = m_appContext->gitService()->rawCommitDiff(repoPath, commitHash, filePath);
            if (raw.isSuccess()) {
                auto* hp2 = m_mainWindow->historyPage();
                if (hp2) {
                    // Parse raw diff into DiffLineModel so it shows in the QTableView
                    Diff diff = Diff::fromUnifiedDiff(raw.value());
                    auto* dlm = m_appContext->historyController()->diffLineModel();
                    if (dlm) {
                        dlm->setDiff(diff);
                    }
                    // Switch to Diff mode
                    if (hp2->diffViewer()) {
                        hp2->diffViewer()->updateViewState();
                    }
                }
            }
        });
    }

    QObject::connect(window, &MainWindow::checkoutCommitRequested, window, [this](const QString& hash) {
        if (!ensureCurrentRepository(QStringLiteral("Checkout commit"))) return;
        auto r = m_appContext->gitService()->checkoutCommit(currentRepositoryPath(), hash);
        if (r.isFailure())
            ToastManager::showError(m_mainWindow.get(), QStringLiteral("Checkout failed: %1").arg(r.errorMessage()));
        else {
            ToastManager::showSuccess(m_mainWindow.get(), QStringLiteral("Checked out %1 (detached HEAD)").arg(hash.left(7)));
            if (auto* hc = m_appContext->historyController()) {
                hc->loadHistory(currentRepositoryPath());
                hc->loadCommitGraph(currentRepositoryPath());
            }
        }
    });

    QObject::connect(window, &MainWindow::createBranchAtCommitRequested, window, [this](const QString& hash) {
        if (!ensureCurrentRepository(QStringLiteral("Create branch"))) return;
        bool ok; QString name = QInputDialog::getText(m_mainWindow.get(), QStringLiteral("Create Branch"),
            QStringLiteral("Branch name:"), QLineEdit::Normal, QString(), &ok);
        if (!ok || name.trimmed().isEmpty()) return;
        auto r = m_appContext->gitService()->createBranchAt(currentRepositoryPath(), name.trimmed(), hash);
        if (r.isFailure())
            ToastManager::showError(m_mainWindow.get(), QStringLiteral("Create branch failed: %1").arg(r.errorMessage()));
        else {
            ToastManager::showSuccess(m_mainWindow.get(), QStringLiteral("Branch '%1' created at %2").arg(name.trimmed(), hash.left(7)));
            m_appContext->branchController()->loadBranches(currentRepositoryPath());
        }
    });

    QObject::connect(window, &MainWindow::resetToCommitRequested, window, [this](const QString& hash, const QString& mode) {
        if (!ensureCurrentRepository(QStringLiteral("Reset"))) return;
        if (mode == QStringLiteral("hard")) {
            auto btn = QMessageBox::warning(m_mainWindow.get(), QStringLiteral("Confirm Reset --hard"),
                QStringLiteral("Reset --hard will discard all uncommitted changes. Continue?"),
                QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
            if (btn != QMessageBox::Yes) return;
        }
        auto r = m_appContext->gitService()->reset(currentRepositoryPath(), hash, mode);
        if (r.isFailure())
            ToastManager::showError(m_mainWindow.get(), QStringLiteral("Reset failed: %1").arg(r.errorMessage()));
        else {
            ToastManager::showSuccess(m_mainWindow.get(), QStringLiteral("Reset --%1 to %2").arg(mode, hash.left(7)));
            if (auto* hc = m_appContext->historyController()) {
                hc->loadHistory(currentRepositoryPath());
                hc->loadCommitGraph(currentRepositoryPath());
            }
            m_appContext->branchController()->loadBranches(currentRepositoryPath());
        }
    });

    // ---- Conflict 操作 ----
    // ConflictPage: 点击冲突文件 → 加载到右侧编辑器
    if (window->conflictPage()) {
        QObject::connect(window->conflictPage(),
                         &ConflictPage::conflictFileSelected,
                         window,
                         [this](const QString& filePath) {
            if (!ensureCurrentRepository(QStringLiteral("Resolve conflict"))) return;
            if (m_mainWindow->conflictPage()) {
                m_mainWindow->conflictPage()->showConflictFile(
                    currentRepositoryPath(), filePath);
            }
        });

        QObject::connect(window->conflictPage()->refreshButton(),
                         &QPushButton::clicked,
                         window,
                         [this]() {
            if (ensureCurrentRepository(QStringLiteral("Refresh conflicts"))) {
                m_appContext->mainController()->refreshAll();
            }
        });
    }

    // ---- AI Commit Message（Phase 5）----
    if (window->changesPage() && window->changesPage()->commitPanel()) {
        CommitPanel* commitPanel = window->changesPage()->commitPanel();
        AICommitController* aiCtrl = m_appContext->aiCommitController();

        // 用户点击 "AI Generate" → 触发生成
        QObject::connect(commitPanel, &CommitPanel::generateAIRequested,
                         window, [this]() {
            if (!ensureCurrentRepository(QStringLiteral("AI Generate"))) {
                return;
            }

            AICommitController* ctrl = m_appContext->aiCommitController();
            if (ctrl) {
                ctrl->generate(currentRepositoryPath());
            }
        });

        // AI Controller → UI 状态更新
        if (aiCtrl) {
            QObject::connect(aiCtrl, &AICommitController::generationStarted,
                             window, [commitPanel]() {
                commitPanel->setGenerating(true);
                commitPanel->setAIStatusMessage(
                    QStringLiteral("Generating commit message..."), false);
            });

            QObject::connect(aiCtrl, &AICommitController::generationSucceeded,
                             window, [commitPanel](const CommitMessageSuggestion& suggestion) {
                commitPanel->setGenerating(false);
                commitPanel->setAIStatusMessage(
                    QStringLiteral("AI suggestion applied. You can edit it before committing."), false);
                commitPanel->setMessage(suggestion.fullMessage());
            });

            QObject::connect(aiCtrl, &AICommitController::generationFailed,
                             window, [commitPanel](const QString& errorMessage) {
                commitPanel->setGenerating(false);
                commitPanel->setAIStatusMessage(
                    QStringLiteral("Generation failed: %1").arg(errorMessage), true);
            });

            QObject::connect(aiCtrl, &AICommitController::generationFinished,
                             window, [commitPanel]() {
                commitPanel->setGenerating(false);
            });
        }
    }
    // ---- 登录操作 ----
    QObject::connect(window, &MainWindow::loginRequested, window, [this]() {
        LoginDialog dialog(m_mainWindow.get());
        QObject::connect(&dialog, &LoginDialog::loginRequested, &dialog,
                         [this, &dialog](CodeHostingPlatform platform, const QString& token) {
            ICodeHostingProvider* provider = m_appContext->providerFactory()
                ? m_appContext->providerFactory()->provider(platform) : nullptr;
            if (!provider) {
                showError(QStringLiteral("Login failed"),
                          QStringLiteral("Unsupported platform."));
                return;
            }
            const auto result = m_appContext->authController()->login(provider, token);
            if (result.isSuccess()) {
                showInfo(QStringLiteral("Logged in as %1.").arg(
                    result.value().platformDisplayName()));
                dialog.accept();
            } else {
                showError(QStringLiteral("Login failed"), result.errorMessage());
            }
        });
        dialog.exec();
    });

    // ---- Pull Request 操作 ----
    QObject::connect(window, &MainWindow::createPullRequestRequested, window,
                     [this](const QString& title, const QString& body,
                             const QString& head, const QString& base) {
        if (!ensureCurrentRepository(QStringLiteral("Create PR"))) {
            return;
        }

        const RemoteRepoInfo repo = resolveRemoteRepo(
            m_appContext.get(), m_appContext->currentRepositoryPath());

        if (!repo.isValid()) {
            showError(QStringLiteral("Create PR"),
                      QStringLiteral("Could not determine code hosting platform from remote URL. "
                                     "Make sure a remote is configured."));
            return;
        }

        showInfo(QStringLiteral("Creating pull request on %1...")
                     .arg(codeHostingPlatformName(repo.provider->platform())));
        const auto result = m_appContext->pullRequestController()->createPullRequest(
            repo.provider, repo.owner, repo.repoName, title, body, head, base);
        if (result.isFailure()) {
            showError(QStringLiteral("Create PR"), result.errorMessage());
            return;
        }

        showInfo(QStringLiteral("Pull request created successfully."));
    });

    QObject::connect(window, &MainWindow::mergePullRequestRequested, window,
                     [this](int prNumber) {
        if (!ensureCurrentRepository(QStringLiteral("Merge PR"))) {
            return;
        }

        const RemoteRepoInfo repo = resolveRemoteRepo(
            m_appContext.get(), m_appContext->currentRepositoryPath());

        if (!repo.isValid()) {
            showError(QStringLiteral("Merge PR"),
                      QStringLiteral("Could not determine code hosting platform from remote URL."));
            return;
        }

        showInfo(QStringLiteral("Merging pull request..."));
        const auto result = m_appContext->pullRequestController()->mergePullRequest(
            repo.provider, repo.owner, repo.repoName, prNumber);
        if (result.isFailure()) {
            showError(QStringLiteral("Merge PR"), result.errorMessage());
        }
    });

    // 切换到 Pull Requests 页面时自动加载 PR 列表
    QObject::connect(window, &MainWindow::pageChanged, window, [this](int pageIndex) {
        if (pageIndex != MainWindow::PullRequestsPageIdx) {
            return;
        }

        // 尝试恢复登录（遍历各平台已保存的 token）
        if (!m_appContext->authController()->isLoggedIn()) {
            if (m_appContext->providerFactory()) {
                const auto providers = m_appContext->providerFactory()->allProviders();
                for (auto* p : providers) {
                    const auto result = m_appContext->authController()->restoreLogin(p);
                    if (result.isSuccess()) break;
                }
            }
        }
        if (!m_appContext->authController()->isLoggedIn()) {
            showInfo(QStringLiteral("Please log in to a code hosting platform first."));
            return;
        }

        const RemoteRepoInfo repo = resolveRemoteRepo(
            m_appContext.get(), m_appContext->currentRepositoryPath());

        if (!repo.isValid()) {
            showInfo(QStringLiteral("No supported code hosting remote configured."));
            return;
        }

        m_appContext->pullRequestController()->loadPullRequests(
            repo.provider, repo.owner, repo.repoName);
    });

    // PR 页面手动刷新按钮
    if (m_mainWindow->pullRequestsPage()) {
        QObject::connect(m_mainWindow->pullRequestsPage(),
                         &PullRequestsPage::refreshRequested,
                         window, [this]() {
            const RemoteRepoInfo repo = resolveRemoteRepo(
                m_appContext.get(), m_appContext->currentRepositoryPath());
            if (!repo.isValid()) return;

            m_appContext->pullRequestController()->loadPullRequests(
                repo.provider, repo.owner, repo.repoName);
        });
    }

    // ---- AI Code Review（Phase 6）----
    // 连接 CommitPanel 的 Review 按钮 → 仅打开 ReviewPanel，用户自行选择审查类型
    if (window->changesPage() && window->changesPage()->commitPanel()) {
        CommitPanel* commitPanel = window->changesPage()->commitPanel();

        QObject::connect(commitPanel, &CommitPanel::reviewRequested,
                         window, [this]() {
            if (!ensureCurrentRepository(QStringLiteral("AI Code Review"))) {
                return;
            }

            if (m_mainWindow && m_mainWindow->changesPage()) {
                m_mainWindow->changesPage()->showReviewPanel();
            }
        });
    }

    // 连接 CodeReviewController → ReviewPanel UI 状态更新
    if (window->changesPage() && window->changesPage()->reviewPanel()) {
        ReviewPanel* reviewPanel = window->changesPage()->reviewPanel();
        CodeReviewController* reviewCtrl = m_appContext->codeReviewController();

        // 将 ReviewFindingModel 绑定到 ReviewPanel 的列表视图
        reviewPanel->setModel(m_appContext->reviewFindingModel());

        // ReviewPanel 按钮 → 触发审查
        QObject::connect(reviewPanel, &ReviewPanel::reviewStagedRequested,
                         window, [this]() {
            if (!ensureCurrentRepository(QStringLiteral("AI Code Review"))) return;
            CodeReviewController* ctrl = m_appContext->codeReviewController();
            if (ctrl) {
                if (m_mainWindow && m_mainWindow->changesPage())
                    m_mainWindow->changesPage()->showReviewPanel();
                ctrl->reviewStaged(currentRepositoryPath());
            }
        });

        QObject::connect(reviewPanel, &ReviewPanel::reviewWorkingTreeRequested,
                         window, [this]() {
            if (!ensureCurrentRepository(QStringLiteral("AI Code Review"))) return;
            CodeReviewController* ctrl = m_appContext->codeReviewController();
            if (ctrl) {
                if (m_mainWindow && m_mainWindow->changesPage())
                    m_mainWindow->changesPage()->showReviewPanel();
                ctrl->reviewWorkingTree(currentRepositoryPath());
            }
        });

        QObject::connect(reviewPanel, &ReviewPanel::cancelRequested,
                         window, [this]() {
            CodeReviewController* ctrl = m_appContext->codeReviewController();
            if (ctrl) ctrl->cancelReview();
        });

        // Controller → UI 状态
        if (reviewCtrl) {
            QObject::connect(reviewCtrl, &CodeReviewController::reviewStarted,
                             window, [reviewPanel]() {
                reviewPanel->setReviewing(true);
            });

            QObject::connect(reviewCtrl, &CodeReviewController::reviewCompleted,
                             window, [this, reviewPanel](const std::vector<ReviewFinding>& findings) {
                reviewPanel->setReviewing(false);
                const int total = static_cast<int>(findings.size());
                if (total == 0) {
                    reviewPanel->setSummaryText(
                        QStringLiteral("No issues found. Your code looks good!"));
                } else {
                    int c = 0, h = 0, m = 0, l = 0, i = 0;
                    for (const auto& f : findings) {
                        const QString& sev = f.severity();
                        if (sev == QStringLiteral("critical")) ++c;
                        else if (sev == QStringLiteral("high")) ++h;
                        else if (sev == QStringLiteral("medium")) ++m;
                        else if (sev == QStringLiteral("low")) ++l;
                        else ++i;
                    }
                    reviewPanel->setSummaryText(
                        QStringLiteral("Found %1 issue(s): %2 critical, %3 high, %4 medium, %5 low, %6 info")
                            .arg(total).arg(c).arg(h).arg(m).arg(l).arg(i));
                }
                showInfo(QStringLiteral("Code review completed."));
            });

            QObject::connect(reviewCtrl, &CodeReviewController::reviewFailed,
                             window, [this, reviewPanel](const QString& errorMessage) {
                reviewPanel->setReviewing(false);
                reviewPanel->setSummaryText(
                    QStringLiteral("Review failed: %1").arg(errorMessage));
                showError(QStringLiteral("AI Code Review"), errorMessage);
            });

            QObject::connect(reviewCtrl, &CodeReviewController::reviewFinished,
                             window, [reviewPanel]() {
                reviewPanel->setReviewing(false);
            });
        }
    }
}

void Application::connectControllerSignals()
{
    if (!m_appContext || !m_mainWindow) {
        return;
    }

    MainWindow* window = m_mainWindow.get();

    QObject::connect(m_appContext->repositoryController(), &RepositoryController::repositoryOpened,
                     window, [this](const Repository& repository) {
        activateRepository(repository);
    });

    QObject::connect(m_appContext->repositoryController(), &RepositoryController::repositoryCloned,
                     window, [this](const Repository& repository) {
        activateRepository(repository);
    });

    QObject::connect(m_appContext->repositoryController(), &RepositoryController::repositoryInitialized,
                     window, [this](const Repository& repository) {
        activateRepository(repository);
    });

    QObject::connect(m_appContext->mainController(), &MainController::globalError,
                     window, [this](const QString& title, const QString& message) {
        showError(title, message);
    });

    // PullRequest、Auth、Remote 控制器的错误信号（MainController 不管理它们）
    if (m_appContext->pullRequestController()) {
        QObject::connect(m_appContext->pullRequestController(),
                         &PullRequestController::errorOccurred,
                         window, [this](const QString& op, const QString& msg) {
            showError(op, msg);
        });
    }
    if (m_appContext->authController()) {
        QObject::connect(m_appContext->authController(),
                         &AuthController::errorOccurred,
                         window, [this](const QString& op, const QString& msg) {
            showError(op, msg);
        });
    }
    if (m_appContext->remoteController()) {
        QObject::connect(m_appContext->remoteController(),
                         &RemoteController::errorOccurred,
                         window, [this](const QString& op, const QString& msg) {
            showError(op, msg);
        });
    }

    QObject::connect(m_appContext.get(), &AppContext::currentRepositoryCleared,
                     window, [this]() {
        m_mainWindow->setCurrentRepository(QString(), QString());
        showInfo(QStringLiteral("Repository closed."));
    });

    QObject::connect(m_appContext->changesController(), &ChangesController::fileStaged,
                     window, [this](bool success, const QString&, const QString& errorMessage) {
        if (success) {
            showInfo(QStringLiteral("File staged."));
        } else {
            showError(QStringLiteral("Stage file"), errorMessage);
        }
    });

    QObject::connect(m_appContext->changesController(), &ChangesController::fileUnstaged,
                     window, [this](bool success, const QString&, const QString& errorMessage) {
        if (success) {
            showInfo(QStringLiteral("File unstaged."));
        } else {
            showError(QStringLiteral("Unstage file"), errorMessage);
        }
    });

    QObject::connect(m_appContext->changesController(), &ChangesController::commitFinished,
                     window, [this](bool success, const QString& errorMessage) {
        if (success) {
            showInfo(QStringLiteral("Commit completed."));
            if (m_mainWindow) {
                m_mainWindow->clearCommitMessage();
            }
            m_appContext->mainController()->refreshAll();
        } else {
            const QString lowerError = errorMessage.toLower();
            if (lowerError.contains(QStringLiteral("no changes added to commit"))
                || lowerError.contains(QStringLiteral("nothing to commit"))) {
                showError(QStringLiteral("Commit"),
                          QStringLiteral("No staged changes. Stage files before committing."));
            } else {
                showError(QStringLiteral("Commit"), errorMessage);
            }
        }
    });

    QObject::connect(m_appContext->branchController(), &BranchController::operationFinished,
                     window, [this](const QString& operation, bool success, const QString& errorMessage) {
        if (!success) {
            showError(operation, errorMessage);
            return;
        }

        showInfo(QStringLiteral("Branch operation completed."));
        refreshCurrentRepositoryHeader();
        m_appContext->mainController()->refreshAll();
    });

    // Merge / Rebase 操作完成回调
    QObject::connect(m_appContext->branchController(),
                     &BranchController::mergeRebaseCompleted,
                     window,
                     [this](const QString& operation, const QString& message) {
        showInfo(operation + QStringLiteral(": ") + message);
        refreshCurrentRepositoryHeader();
        m_appContext->mainController()->refreshAll();
    });

    // Merge / Rebase 操作的 busy 状态管理
    QObject::connect(m_appContext->mergeController(),
                     &MergeController::operationStarted,
                     window,
                     [this](const QString& operation) {
        m_mainWindow->setBusy(true);
        m_mainWindow->setStatusMessage(QStringLiteral("正在执行 %1...").arg(operation));
    });
    QObject::connect(m_appContext->mergeController(),
                     &MergeController::operationFinished,
                     window,
                     [this](const QString&) {
        m_mainWindow->setBusy(false);
    });
    QObject::connect(m_appContext->rebaseController(),
                     &RebaseController::operationStarted,
                     window,
                     [this](const QString& operation) {
        m_mainWindow->setBusy(true);
        m_mainWindow->setStatusMessage(QStringLiteral("正在执行 %1...").arg(operation));
    });
    QObject::connect(m_appContext->rebaseController(),
                     &RebaseController::operationFinished,
                     window,
                     [this](const QString&) {
        m_mainWindow->setBusy(false);
    });

    QObject::connect(m_appContext->syncController(), &SyncController::fetchFinished,
                     window, [this](bool success, const QString& errorMessage) {
        if (success) {
            showInfo(QStringLiteral("Fetch completed."));
            m_appContext->mainController()->refreshAll();
        } else {
            showError(QStringLiteral("Fetch"), errorMessage);
        }
    });

    QObject::connect(m_appContext->syncController(), &SyncController::pullFinished,
                     window, [this](bool success, const QString& errorMessage) {
        if (success) {
            showInfo(QStringLiteral("Pull completed."));
            m_appContext->mainController()->refreshAll();
        } else {
            showError(QStringLiteral("Pull"), errorMessage);
        }
    });

    QObject::connect(m_appContext->syncController(), &SyncController::pushFinished,
                     window, [this](bool success, const QString& errorMessage) {
        if (success) {
            showInfo(QStringLiteral("Push completed."));
            m_appContext->mainController()->refreshAll();
        } else {
            showError(QStringLiteral("Push"), errorMessage);
        }
    });

    // ---- Stash 控制器信号 ----
    QObject::connect(m_appContext->stashController(), &StashController::stashesLoaded,
                     window, [this](bool success, const QString& errorMessage) {
        if (!success) {
            showError(QStringLiteral("Load stashes"), errorMessage);
        }
    });

    QObject::connect(m_appContext->stashController(), &StashController::operationFinished,
                     window, [this](const QString& operation, bool success, const QString& errorMessage) {
        if (success) {
            showInfo(QStringLiteral("Stash operation completed."));
            refreshCurrentRepositoryHeader();
        } else {
            showError(operation, errorMessage);
        }
    });

    QObject::connect(m_appContext->stashController(), &StashController::stashDiffLoaded,
                     window, [this](bool success, int /*stashIndex*/, const QString& errorMessage) {
        if (success) {
            // 刷新工作区变更列表，确保 Changes 页面显示最新状态
            m_appContext->changesController()->refreshChanges(currentRepositoryPath());
            // 切换到 Changes 页面，以便用户能在 DiffViewer 中看到已加载的 diff
            m_mainWindow->setCurrentPage(MainWindow::ChangesPageIdx);
            showInfo(QStringLiteral("Stash diff loaded."));
        } else {
            showError(QStringLiteral("Show stash diff"), errorMessage);
        }
    });

    // ---- Remote 控制器信号 ----
    QObject::connect(m_appContext->remoteController(), &RemoteController::remotesLoaded,
                     window, [this](bool success, const QString& errorMessage) {
        if (!success) {
            showError(QStringLiteral("Load remotes"), errorMessage);
        }
    });

    QObject::connect(m_appContext->remoteController(), &RemoteController::operationFinished,
                     window, [this](const QString& operation, bool success, const QString& errorMessage) {
        if (success) {
            showInfo(QStringLiteral("Remote operation completed."));
        } else {
            showError(operation, errorMessage);
        }
    });

    // ---- Auth 控制器信号 ----
    QObject::connect(m_appContext->authController(), &AuthController::loginSucceeded,
                     window, [this](const Account& account) {
        m_mainWindow->setLoggedInUser(account.platformDisplayName());

        // 登录成功后刷新 PR 列表
        const RemoteRepoInfo repo = resolveRemoteRepo(
            m_appContext.get(), m_appContext->currentRepositoryPath());
        if (repo.isValid()) {
            m_appContext->pullRequestController()->loadPullRequests(
                repo.provider, repo.owner, repo.repoName);
        }
    });

    QObject::connect(m_appContext->authController(), &AuthController::logoutCompleted,
                     window, [this]() {
        m_mainWindow->clearLoggedInUser();
    });

    // ---- Conflict 控制器信号 ----
    if (m_mainWindow && m_mainWindow->conflictPage()
        && m_mainWindow->conflictPage()->conflictResolver()) {
        QObject::connect(m_mainWindow->conflictPage()->conflictResolver(),
                         &ConflictResolver::markResolvedRequested,
                         window,
                         [this](const QString& repoPath, const QString& filePath) {
            GitService* gitService = m_appContext ? m_appContext->gitService() : nullptr;
            if (gitService) {
                const Result<void> stageResult = gitService->stageFile(repoPath, filePath);
                if (stageResult.isSuccess()) {
                    showInfo(QStringLiteral("文件已标记为已解决: %1").arg(filePath));
                    m_appContext->mainController()->refreshAll();
                } else {
                    showError(QStringLiteral("Mark Resolved"), stageResult.errorMessage());
                }
            }
        });
    }

    // 命令行终端
    if (m_appContext->terminalController()) {
        TerminalWidget* terminal = window->findChild<TerminalWidget*>();
        if (terminal) {
            QObject::connect(terminal, &TerminalWidget::commandEntered,
                             window, [this](const QString& command) {
                m_appContext->terminalController()->executeCommand(
                    currentRepositoryPath(), command);
            });
            QObject::connect(terminal, &TerminalWidget::clearRequested,
                             window, [this]() {
                if (m_appContext->terminalController())
                    m_appContext->terminalController()->clear();
            });
        }
    }
}

QString Application::currentRepositoryPath() const
{
    return m_appContext ? m_appContext->currentRepositoryPath() : QString();
}

void Application::activateRepository(const Repository& repository)
{
    if (!m_appContext || !m_mainWindow || repository.localPath().isEmpty()) {
        return;
    }

    Repository activeRepository = repository;

    // 优化: repository.currentBranch() 已由 openRepository 填充，无需再次查询
    // 仅当为空时才回退到同步查询（兼容旧代码路径）
    if (activeRepository.currentBranch().isEmpty()) {
        activeRepository.setCurrentBranch(
            currentBranchNameForPath(repository.localPath()));
    }

    // 优化: 检测 merge/rebase 状态 — 纯文件系统检查，不调用 git 子进程
    QString stateHint;
    const QString gitDir = repository.localPath() + QStringLiteral("/.git");
    if (QFileInfo::exists(gitDir + QStringLiteral("/MERGE_HEAD"))) {
        stateHint = QStringLiteral("merging");
    } else if (QFileInfo::exists(gitDir + QStringLiteral("/rebase-merge"))
               || QFileInfo::exists(gitDir + QStringLiteral("/rebase-apply"))) {
        stateHint = QStringLiteral("rebasing");
    }

    m_appContext->setCurrentRepository(activeRepository);
    m_mainWindow->setCurrentRepository(activeRepository.displayName(),
                                       activeRepository.localPath(),
                                       activeRepository.currentBranch(),
                                       stateHint);

    SettingsService* settings = m_appContext->settingsService();
    if (settings) {
        settings->setLastOpenedRepositoryPath(repository.localPath());
    }

    m_appContext->mainController()->switchToRepository(activeRepository.localPath());
    showInfo(QStringLiteral("Repository opened."));
}

QString Application::currentBranchNameForPath(const QString& repoPath, const QString& fallback) const
{
    if (!m_appContext || repoPath.trimmed().isEmpty()) {
        return fallback.trimmed();
    }

    BranchController* branchController = m_appContext->branchController();
    if (!branchController) {
        return fallback.trimmed();
    }

    const Result<QString> result = branchController->currentBranchName(repoPath);
    if (result.isSuccess()) {
        return result.value().trimmed();
    }

    return fallback.trimmed();
}

void Application::refreshCurrentRepositoryHeader()
{
    if (!m_appContext || !m_mainWindow) {
        return;
    }

    Repository repository = m_appContext->currentRepository();
    if (repository.localPath().isEmpty()) {
        m_mainWindow->setCurrentRepository(QString(), QString(), QString());
        return;
    }

    // 每次刷新时都从 git 读取最新的分支名
    repository.setCurrentBranch(
        currentBranchNameForPath(repository.localPath(), repository.currentBranch()));

    // 优化: 纯文件系统检查，不调用 git 子进程
    QString stateHint;
    const QString gitDir = repository.localPath() + QStringLiteral("/.git");
    if (QFileInfo::exists(gitDir + QStringLiteral("/MERGE_HEAD"))) {
        stateHint = QStringLiteral("merging");
    } else if (QFileInfo::exists(gitDir + QStringLiteral("/rebase-merge"))
               || QFileInfo::exists(gitDir + QStringLiteral("/rebase-apply"))) {
        stateHint = QStringLiteral("rebasing");
    }

    m_appContext->setCurrentRepository(repository);
    m_mainWindow->setCurrentRepository(repository.displayName(),
                                       repository.localPath(),
                                       repository.currentBranch(),
                                       stateHint);
}

bool Application::ensureCurrentRepository(const QString& operation) const
{
    if (currentRepositoryPath().isEmpty()) {
        showError(operation, QStringLiteral("No repository is currently selected."));
        return false;
    }

    return true;
}

void Application::showInfo(const QString& message) const
{
    if (!m_mainWindow) {
        return;
    }

    m_mainWindow->setStatusMessage(message, 3000);
    ToastManager::showMessage(m_mainWindow.get(), message, 2500);
}

void Application::showError(const QString& title, const QString& message) const
{
    if (!m_mainWindow) {
        return;
    }

    const QString text = title.trimmed().isEmpty()
                             ? message
                             : QStringLiteral("%1: %2").arg(title, message);
    m_mainWindow->setStatusMessage(text, 5000);
    ToastManager::showError(m_mainWindow.get(), text, 5000);
    Logger::instance().error(text, QStringLiteral("Application"));
}

void Application::restoreMainWindowState()
{
    if (!m_appContext || !m_mainWindow) {
        return;
    }

    SettingsService* settings = m_appContext->settingsService();
    if (!settings) {
        return;
    }

    const QByteArray geometry = settings->mainWindowGeometry();
    if (!geometry.isEmpty()) {
        m_mainWindow->restoreGeometry(geometry);
    }

    const QByteArray state = settings->mainWindowState();
    if (!state.isEmpty()) {
        m_mainWindow->restoreState(state);
    }
}

void Application::saveMainWindowState()
{
    if (!m_appContext || !m_mainWindow) {
        return;
    }

    SettingsService* settings = m_appContext->settingsService();
    if (!settings) {
        return;
    }

    settings->setMainWindowGeometry(m_mainWindow->saveGeometry());
    settings->setMainWindowState(m_mainWindow->saveState());
}

void Application::connectApplicationSignals()
{
    if (m_aboutToQuitConnection) {
        return;
    }

    m_aboutToQuitConnection = QObject::connect(&m_qtApplication,
                                               &QCoreApplication::aboutToQuit,
                                               &m_qtApplication,
                                               [this]() {
                                                   saveMainWindowState();
                                               });
}

void Application::setLastError(const QString& errorMessage)
{
    m_lastError = errorMessage.trimmed();
}
