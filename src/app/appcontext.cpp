#include "appcontext.h"

#include "controllers/AICommitController.h"
#include "controllers/branchcontroller.h"
#include "controllers/changescontroller.h"
#include "controllers/CodeReviewController.h"
#include "controllers/ConflictController.h"
#include "controllers/historycontroller.h"
#include "controllers/maincontroller.h"
#include "controllers/MergeController.h"
#include "controllers/MergeToolController.h"
#include "controllers/RebaseController.h"
#include "controllers/repositorycontroller.h"
#include "controllers/AuthController.h"
#include "controllers/PullRequestController.h"
#include "controllers/RemoteController.h"
#include "controllers/StashController.h"
#include "controllers/synccontroller.h"
#include "infrastructure/AIClient.h"
#include "infrastructure/ICodeHostingProvider.h"
#include "models/branchlistmodel.h"
#include "models/CommitGraphModel.h"
#include "models/commithistorymodel.h"
#include "models/CommitMessageSuggestionModel.h"
#include "models/ConflictFileModel.h"
#include "models/difflinemodel.h"
#include "models/filechangemodel.h"
#include "models/MergeLineModel.h"
#include "models/PullRequestModel.h"
#include "models/RemoteListModel.h"
#include "models/repositorylistmodel.h"
#include "models/ReviewFindingModel.h"
#include "models/StashListModel.h"
#include "services/AIService.h"
#include "services/AuthService.h"
#include "services/branchservice.h"
#include "services/CommitMessageService.h"
#include "services/CodeReviewService.h"
#include "services/ConflictService.h"
#include "services/CredentialStore.h"
#include "services/gitservice.h"
#include "services/gittaskrunner.h"
#include "services/MergeService.h"
#include "services/MergeToolService.h"
#include "services/ProviderFactory.h"
#include "services/PullRequestService.h"
#include "services/RebaseService.h"
#include "services/RemoteService.h"
#include "services/repositoryservice.h"
#include "services/settingsservice.h"
#include "services/StashService.h"
#include "services/syncservice.h"
#include "services/TerminalService.h"
#include "controllers/TerminalController.h"
#include "models/TerminalOutputModel.h"
#include "infrastructure/HttpClient.h"

#include <QCoreApplication>

AppContext::AppContext(QObject* parent)
    : QObject(parent),
    m_mainWindow(nullptr),
    m_initialized(false)
{
}

AppContext::~AppContext()
{
    shutdown();
}

void AppContext::initialize()
{
    if (m_initialized) {
        return;
    }

    createDefaultObjects();
    applyGitExecutablePath();
    loadRecentRepositories();

    m_initialized = true;
    emit initialized();
}

bool AppContext::isInitialized() const
{
    return m_initialized;
}

void AppContext::shutdown()
{
    if (!m_initialized
        && !m_settingsService
        && !m_gitService
        && !m_gitTaskRunner
        && !m_repositoryService
        && !m_branchService
        && !m_remoteService
        && !m_syncService
        && !m_mergeService
        && !m_rebaseService
        && !m_conflictService
        && !m_mergeToolService
        && !m_repositoryListModel
        && !m_fileChangeModel
        && !m_diffLineModel
        && !m_commitHistoryModel
        && !m_commitGraphModel
        && !m_branchListModel
        && !m_remoteListModel
        && !m_pullRequestModel
        && !m_mainController
        && !m_repositoryController
        && !m_changesController
        && !m_historyController
        && !m_branchController
        && !m_remoteController
        && !m_syncController
        && !m_mergeController
        && !m_rebaseController
        && !m_aiClient
        && !m_aiService
        && !m_commitMessageService
        && !m_commitMessageSuggestionModel
        && !m_aiCommitController
        && !m_codeReviewService
        && !m_reviewFindingModel
        && !m_codeReviewController) {
        return;
    }

    emit shutdownRequested();

    saveRecentRepositories();
    clearRepositoryData();

    m_mainController.reset();
    m_syncController.reset();
    m_remoteController.reset();
    m_authController.reset();
    m_pullRequestController.reset();
    m_stashController.reset();
    m_branchController.reset();
    m_historyController.reset();
    m_changesController.reset();
    m_repositoryController.reset();
    m_mergeController.reset();
    m_rebaseController.reset();
    m_conflictCtrl.reset();
    m_mergeToolController.reset();
    m_aiCommitController.reset();
    m_codeReviewController.reset();

    m_remoteListModel.reset();
    m_pullRequestModel.reset();
    m_stashListModel.reset();
    m_branchListModel.reset();
    m_commitHistoryModel.reset();
    m_commitGraphModel.reset();
    m_conflictFileModel.reset();
    m_mergeLineModel.reset();
    m_commitMessageSuggestionModel.reset();
    m_reviewFindingModel.reset();
    m_diffLineModel.reset();
    m_fileChangeModel.reset();
    m_repositoryListModel.reset();

    m_syncService.reset();
    m_remoteService.reset();
    m_stashService.reset();
    m_pullRequestService.reset();
    m_authService.reset();
    m_credentialStore.reset();
    m_providerFactory.reset();
    m_httpClient.reset();
    m_branchService.reset();
    m_mergeService.reset();
    m_rebaseService.reset();
    m_conflictService.reset();
    m_mergeToolService.reset();
    m_commitMessageService.reset();
    m_codeReviewService.reset();
    m_aiService.reset();
    m_aiClient.reset();
    m_terminalController.reset();
    m_terminalService.reset();
    m_terminalOutputModel.reset();
    m_repositoryService.reset();
    m_gitTaskRunner.reset();
    m_gitService.reset();

    if (m_settingsService) {
        m_settingsService->sync();
        m_settingsService.reset();
    }

    m_currentRepository = Repository();
    m_mainWindow = nullptr;
    m_initialized = false;
}

// ---- 服务访问器 ----

SettingsService* AppContext::settingsService() const { return m_settingsService.get(); }
GitService* AppContext::gitService() const { return m_gitService.get(); }
GitTaskRunner* AppContext::gitTaskRunner() const { return m_gitTaskRunner.get(); }
RepositoryService* AppContext::repositoryService() const { return m_repositoryService.get(); }
BranchService* AppContext::branchService() const { return m_branchService.get(); }
StashService* AppContext::stashService() const { return m_stashService.get(); }
RemoteService* AppContext::remoteService() const { return m_remoteService.get(); }
SyncService* AppContext::syncService() const { return m_syncService.get(); }
AuthService* AppContext::authService() const { return m_authService.get(); }
PullRequestService* AppContext::pullRequestService() const { return m_pullRequestService.get(); }
CredentialStore* AppContext::credentialStore() const { return m_credentialStore.get(); }
HttpClient* AppContext::httpClient() const { return m_httpClient.get(); }

// ---- Provider ----

ProviderFactory* AppContext::providerFactory() const
{
    return m_providerFactory.get();
}

ICodeHostingProvider* AppContext::providerForRemote(const QString& remoteUrl)
{
    return m_providerFactory ? m_providerFactory->providerForRemote(remoteUrl) : nullptr;
}

ICodeHostingProvider* AppContext::defaultProvider()
{
    return m_providerFactory ? m_providerFactory->defaultProvider() : nullptr;
}

// ---- 模型访问器 ----

RepositoryListModel* AppContext::repositoryListModel() const { return m_repositoryListModel.get(); }
FileChangeModel* AppContext::fileChangeModel() const { return m_fileChangeModel.get(); }
DiffLineModel* AppContext::diffLineModel() const { return m_diffLineModel.get(); }
CommitHistoryModel* AppContext::commitHistoryModel() const { return m_commitHistoryModel.get(); }
CommitGraphModel* AppContext::commitGraphModel() const { return m_commitGraphModel.get(); }
BranchListModel* AppContext::branchListModel() const { return m_branchListModel.get(); }
StashListModel* AppContext::stashListModel() const { return m_stashListModel.get(); }
RemoteListModel* AppContext::remoteListModel() const { return m_remoteListModel.get(); }
PullRequestModel* AppContext::pullRequestModel() const { return m_pullRequestModel.get(); }

// ---- 控制器访问器 ----

MainController* AppContext::mainController() const { return m_mainController.get(); }
RepositoryController* AppContext::repositoryController() const { return m_repositoryController.get(); }
ChangesController* AppContext::changesController() const { return m_changesController.get(); }
HistoryController* AppContext::historyController() const { return m_historyController.get(); }
BranchController* AppContext::branchController() const { return m_branchController.get(); }
StashController* AppContext::stashController() const { return m_stashController.get(); }
RemoteController* AppContext::remoteController() const { return m_remoteController.get(); }
AuthController* AppContext::authController() const { return m_authController.get(); }
PullRequestController* AppContext::pullRequestController() const { return m_pullRequestController.get(); }
SyncController* AppContext::syncController() const { return m_syncController.get(); }
MergeController* AppContext::mergeController() const { return m_mergeController.get(); }
RebaseController* AppContext::rebaseController() const { return m_rebaseController.get(); }
MergeService* AppContext::mergeService() const { return m_mergeService.get(); }
RebaseService* AppContext::rebaseService() const { return m_rebaseService.get(); }
ConflictService* AppContext::conflictService() const { return m_conflictService.get(); }
MergeToolService* AppContext::mergeToolService() const { return m_mergeToolService.get(); }
ConflictController* AppContext::conflictController() const { return m_conflictCtrl.get(); }
MergeToolController* AppContext::mergeToolController() const { return m_mergeToolController.get(); }
ConflictFileModel* AppContext::conflictFileModel() const { return m_conflictFileModel.get(); }
MergeLineModel* AppContext::mergeLineModel() const { return m_mergeLineModel.get(); }

AIClient* AppContext::aiClient() const { return m_aiClient.get(); }
AIService* AppContext::aiService() const { return m_aiService.get(); }
CommitMessageService* AppContext::commitMessageService() const { return m_commitMessageService.get(); }
AICommitController* AppContext::aiCommitController() const { return m_aiCommitController.get(); }
CommitMessageSuggestionModel* AppContext::commitMessageSuggestionModel() const { return m_commitMessageSuggestionModel.get(); }

TerminalService* AppContext::terminalService() const { return m_terminalService.get(); }
TerminalController* AppContext::terminalController() const { return m_terminalController.get(); }
TerminalOutputModel* AppContext::terminalOutputModel() const { return m_terminalOutputModel.get(); }

CodeReviewService* AppContext::codeReviewService() const { return m_codeReviewService.get(); }
CodeReviewController* AppContext::codeReviewController() const { return m_codeReviewController.get(); }
ReviewFindingModel* AppContext::reviewFindingModel() const { return m_reviewFindingModel.get(); }

MainWindow* AppContext::mainWindow() const { return m_mainWindow; }

void AppContext::setMainWindow(MainWindow* mainWindow)
{
    if (m_mainWindow == mainWindow) return;
    m_mainWindow = mainWindow;
    emit mainWindowChanged(m_mainWindow);
}

Repository AppContext::currentRepository() const { return m_currentRepository; }

void AppContext::setCurrentRepository(const Repository& repository)
{
    m_currentRepository = repository;
    if (m_currentRepository.isValid() || !m_currentRepository.localPath().isEmpty()) {
        addRecentRepository(m_currentRepository);
    }
    emit currentRepositoryChanged(m_currentRepository);
}

void AppContext::clearCurrentRepository()
{
    if (!hasCurrentRepository() && m_currentRepository.localPath().isEmpty()) {
        clearRepositoryData();
        return;
    }
    m_currentRepository = Repository();
    clearRepositoryData();
    emit currentRepositoryCleared();
    emit currentRepositoryChanged(m_currentRepository);
}

bool AppContext::hasCurrentRepository() const { return m_currentRepository.isValid(); }
QString AppContext::currentRepositoryPath() const { return m_currentRepository.localPath(); }

void AppContext::loadRecentRepositories()
{
    if (!m_settingsService || !m_repositoryListModel) return;
    m_repositoryListModel->setRepositories(m_settingsService->recentRepositories());
}

void AppContext::saveRecentRepositories()
{
    if (!m_settingsService || !m_repositoryListModel) return;
    m_settingsService->setRecentRepositories(m_repositoryListModel->repositories());
}

void AppContext::addRecentRepository(const Repository& repository)
{
    if (repository.localPath().isEmpty()) return;
    if (m_repositoryListModel) m_repositoryListModel->upsertRepository(repository);
    if (m_settingsService) m_settingsService->addRecentRepository(repository);
}

void AppContext::removeRecentRepository(const QString& localPath)
{
    if (m_repositoryListModel) m_repositoryListModel->removeRepository(localPath);
    if (m_settingsService) m_settingsService->removeRecentRepository(localPath);
    if (Repository::resolveAbsolutePath(m_currentRepository.localPath())
        == Repository::resolveAbsolutePath(localPath)) {
        clearCurrentRepository();
    }
}

void AppContext::clearRepositoryData()
{
    if (m_fileChangeModel) m_fileChangeModel->clear();
    if (m_diffLineModel) m_diffLineModel->clear();
    if (m_commitHistoryModel) m_commitHistoryModel->clear();
    if (m_commitGraphModel) m_commitGraphModel->clear();
    if (m_branchListModel) m_branchListModel->clear();
    if (m_stashListModel) m_stashListModel->clear();
    if (m_remoteListModel) m_remoteListModel->clear();
}

void AppContext::updateGitExecutablePath(const QString& gitExecutablePath)
{
    const QString cleanPath = gitExecutablePath.trimmed();
    if (m_settingsService) m_settingsService->setGitExecutablePath(cleanPath);
    if (m_gitService) m_gitService->setGitExecutablePath(cleanPath);
    if (m_gitTaskRunner) m_gitTaskRunner->setGitExecutablePath(cleanPath);
}

void AppContext::createDefaultObjects()
{
    // ---- 基础服务 ----
    if (!m_settingsService) {
        m_settingsService = std::make_unique<SettingsService>(
            QCoreApplication::organizationName(),
            QCoreApplication::applicationName());
    }

    if (!m_gitService)  { m_gitService = std::make_unique<GitService>(); }
    if (!m_gitTaskRunner) { m_gitTaskRunner = std::make_unique<GitTaskRunner>(this); }
    if (!m_repositoryService) {
        m_repositoryService = std::make_unique<RepositoryService>(
            m_gitService.get(), m_settingsService.get());
    }
    if (!m_branchService) { m_branchService = std::make_unique<BranchService>(m_gitService.get()); }
    if (!m_stashService) { m_stashService = std::make_unique<StashService>(m_gitService.get()); }
    if (!m_remoteService) { m_remoteService = std::make_unique<RemoteService>(m_gitService.get()); }

    // ---- HTTP / Provider / 凭据 ----
    if (!m_httpClient) {
        m_httpClient = std::make_unique<HttpClient>();
    }

    if (!m_providerFactory) {
        m_providerFactory = std::make_unique<ProviderFactory>(m_httpClient.get());
    }

    if (!m_credentialStore) {
        m_credentialStore = std::make_unique<CredentialStore>(m_settingsService.get());
    }

    if (!m_authService) {
        m_authService = std::make_unique<AuthService>(m_credentialStore.get());
    }

    if (!m_pullRequestService) {
        m_pullRequestService = std::make_unique<PullRequestService>();
    }

    if (!m_syncService) { m_syncService = std::make_unique<SyncService>(m_gitService.get()); }

    // ---- 模型 ----
    if (!m_repositoryListModel) { m_repositoryListModel = std::make_unique<RepositoryListModel>(this); }
    if (!m_fileChangeModel) { m_fileChangeModel = std::make_unique<FileChangeModel>(this); }
    if (!m_diffLineModel) { m_diffLineModel = std::make_unique<DiffLineModel>(this); }
    if (!m_commitHistoryModel) { m_commitHistoryModel = std::make_unique<CommitHistoryModel>(this); }
    if (!m_commitGraphModel) { m_commitGraphModel = std::make_unique<CommitGraphModel>(this); }
    if (!m_branchListModel) { m_branchListModel = std::make_unique<BranchListModel>(this); }
    if (!m_stashListModel) { m_stashListModel = std::make_unique<StashListModel>(this); }
    if (!m_remoteListModel) { m_remoteListModel = std::make_unique<RemoteListModel>(this); }
    if (!m_pullRequestModel) { m_pullRequestModel = std::make_unique<PullRequestModel>(this); }

    // ---- 控制器 ----
    if (!m_repositoryController) {
        m_repositoryController = std::make_unique<RepositoryController>(
            m_repositoryService.get(), m_repositoryListModel.get(), this);
        m_repositoryController->setGitTaskRunner(m_gitTaskRunner.get());
    }
    if (!m_changesController) {
        m_changesController = std::make_unique<ChangesController>(
            m_gitService.get(), m_fileChangeModel.get(), m_diffLineModel.get(), this);
    }
    if (!m_historyController) {
        m_historyController = std::make_unique<HistoryController>(
            m_gitService.get(), m_commitHistoryModel.get(),
            m_commitGraphModel.get(), m_diffLineModel.get(), this);
    }
    if (!m_branchController) {
        m_branchController = std::make_unique<BranchController>(
            m_branchService.get(), m_branchListModel.get(), this);
    }
    if (!m_stashController) {
        m_stashController = std::make_unique<StashController>(
            m_stashService.get(), m_stashListModel.get(), m_diffLineModel.get());
    }

    if (!m_mergeService) { m_mergeService = std::make_unique<MergeService>(m_gitService.get()); }
    if (!m_rebaseService) { m_rebaseService = std::make_unique<RebaseService>(m_gitService.get()); }
    if (!m_mergeController) {
        m_mergeController = std::make_unique<MergeController>(
            m_mergeService.get(), m_gitTaskRunner.get(), this);
    }
    if (!m_rebaseController) {
        m_rebaseController = std::make_unique<RebaseController>(
            m_rebaseService.get(), m_gitTaskRunner.get(), this);
    }
    if (m_branchController) {
        m_branchController->setMergeController(m_mergeController.get());
        m_branchController->setRebaseController(m_rebaseController.get());
        m_branchController->setGitTaskRunner(m_gitTaskRunner.get());
    }

    if (!m_conflictService) { m_conflictService = std::make_unique<ConflictService>(m_gitService.get()); }
    if (!m_mergeToolService) { m_mergeToolService = std::make_unique<MergeToolService>(m_gitService.get()); }
    if (!m_conflictFileModel) { m_conflictFileModel = std::make_unique<ConflictFileModel>(this); }
    if (!m_mergeLineModel) { m_mergeLineModel = std::make_unique<MergeLineModel>(this); }
    if (!m_conflictCtrl) {
        m_conflictCtrl = std::make_unique<ConflictController>(
            m_conflictService.get(), m_conflictFileModel.get(), m_gitTaskRunner.get(), this);
    }
    if (!m_mergeToolController) {
        m_mergeToolController = std::make_unique<MergeToolController>(
            m_mergeToolService.get(), m_mergeLineModel.get(), this);
    }
    if (!m_remoteController) {
        m_remoteController = std::make_unique<RemoteController>(
            m_remoteService.get(), m_remoteListModel.get(), this);
    }

    // AI Commit Message（Phase 5）
    if (!m_aiClient) { m_aiClient = std::make_unique<AIClient>(); }
    if (!m_aiService) {
        m_aiService = std::make_unique<AIService>(m_aiClient.get(), m_settingsService.get());
    }
    if (!m_commitMessageService) {
        m_commitMessageService = std::make_unique<CommitMessageService>(
            m_gitService.get(), m_aiService.get());
    }
    if (!m_commitMessageSuggestionModel) {
        m_commitMessageSuggestionModel = std::make_unique<CommitMessageSuggestionModel>(this);
    }
    if (!m_aiCommitController) {
        m_aiCommitController = std::make_unique<AICommitController>(
            m_commitMessageService.get(), m_commitMessageSuggestionModel.get(), this);
    }

    // 命令行终端
    if (!m_terminalOutputModel) { m_terminalOutputModel = std::make_unique<TerminalOutputModel>(this); }
    if (!m_terminalService) {
        m_terminalService = std::make_unique<TerminalService>(m_gitService.get());
        m_terminalService->setOutputModel(m_terminalOutputModel.get());
    }
    if (!m_terminalController) {
        m_terminalController = std::make_unique<TerminalController>(
            m_terminalService.get(), m_terminalOutputModel.get(), this);
    }
    if (m_gitService) {
        m_gitService->setCommandLogCallback(
            [this](const QString& cmd, const QString& wd, int ec, const QString& out) {
                if (m_terminalService)
                    m_terminalService->onCommandExecuted(cmd, wd, ec, out);
            });
    }

    // Auth / PR 控制器
    if (!m_authController) {
        m_authController = std::make_unique<AuthController>(m_authService.get(), this);
    }
    if (!m_pullRequestController) {
        m_pullRequestController = std::make_unique<PullRequestController>(
            m_pullRequestService.get(), m_pullRequestModel.get());
    }

    // AI Code Review（Phase 6）
    if (!m_codeReviewService) {
        m_codeReviewService = std::make_unique<CodeReviewService>(
            m_gitService.get(), m_aiService.get());
    }
    if (!m_reviewFindingModel) { m_reviewFindingModel = std::make_unique<ReviewFindingModel>(this); }
    if (!m_codeReviewController) {
        m_codeReviewController = std::make_unique<CodeReviewController>(
            m_codeReviewService.get(), m_reviewFindingModel.get(), this);
    }

    if (!m_syncController) {
        m_syncController = std::make_unique<SyncController>(m_syncService.get(), this);
        m_syncController->setGitTaskRunner(m_gitTaskRunner.get());
    }

    // 将 GitTaskRunner 注入到需要使用异步操作的控制器
    if (m_changesController) {
        m_changesController->setGitTaskRunner(m_gitTaskRunner.get());
    }
    if (m_historyController) {
        m_historyController->setGitTaskRunner(m_gitTaskRunner.get());
    }
    if (m_stashController) {
        m_stashController->setGitTaskRunner(m_gitTaskRunner.get());
    }

    if (!m_mainController) {
        m_mainController = std::make_unique<MainController>(this);
    }

    m_mainController->setRepositoryController(m_repositoryController.get());
    m_mainController->setChangesController(m_changesController.get());
    m_mainController->setHistoryController(m_historyController.get());
    m_mainController->setBranchController(m_branchController.get());
    m_mainController->setStashController(m_stashController.get());
    m_mainController->setRemoteController(m_remoteController.get());
    m_mainController->setSyncController(m_syncController.get());
    m_mainController->setConflictController(m_conflictCtrl.get());
    m_mainController->setPullRequestController(m_pullRequestController.get());
    m_mainController->setGitService(m_gitService.get());
    m_mainController->setSettingsService(m_settingsService.get());
    m_mainController->setGitTaskRunner(m_gitTaskRunner.get());
}

void AppContext::applyGitExecutablePath()
{
    if (!m_settingsService) return;
    const QString gitExecutablePath = m_settingsService->gitExecutablePath();
    if (m_gitService) m_gitService->setGitExecutablePath(gitExecutablePath);
    if (m_gitTaskRunner) m_gitTaskRunner->setGitExecutablePath(gitExecutablePath);
}
