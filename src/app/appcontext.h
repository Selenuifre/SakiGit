#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include "domain/CodeHostingPlatform.h"
#include "domain/repository.h"

#include <QObject>
#include <QString>

#include <memory>

class AIClient;
class AICommitController;
class AIService;
class BranchController;
class BranchListModel;
class BranchService;
class ChangesController;
class CodeReviewController;
class CodeReviewService;
class RemoteController;
class RemoteListModel;
class RemoteService;
class AuthController;
class AuthService;
class CredentialStore;
class HttpClient;
class ICodeHostingProvider;
class ProviderFactory;
class PullRequestController;
class PullRequestModel;
class PullRequestService;
class ReviewFindingModel;
class StashController;
class StashListModel;
class StashService;
class TerminalController;
class TerminalOutputModel;
class TerminalService;
class CommitGraphModel;
class CommitHistoryModel;
class CommitMessageService;
class CommitMessageSuggestionModel;
class ConflictController;
class ConflictFileModel;
class ConflictService;
class DiffLineModel;
class FileChangeModel;
class GitService;
class GitTaskRunner;
class HistoryController;
class MainController;
class MainWindow;
class MergeController;
class MergeLineModel;
class MergeService;
class MergeToolController;
class MergeToolService;
class RebaseController;
class RebaseService;
class RepositoryController;
class RepositoryListModel;
class RepositoryService;
class SettingsService;
class SyncController;
class SyncService;

class AppContext : public QObject
{
    Q_OBJECT

public:
    explicit AppContext(QObject* parent = nullptr);
    ~AppContext() override;

    AppContext(const AppContext&) = delete;
    AppContext& operator=(const AppContext&) = delete;

    void initialize();
    bool isInitialized() const;
    void shutdown();

    // ---- 服务访问器 ----
    SettingsService* settingsService() const;
    GitService* gitService() const;
    GitTaskRunner* gitTaskRunner() const;
    RepositoryService* repositoryService() const;
    BranchService* branchService() const;
    StashService* stashService() const;
    RemoteService* remoteService() const;
    SyncService* syncService() const;
    AuthService* authService() const;
    PullRequestService* pullRequestService() const;
    CredentialStore* credentialStore() const;
    HttpClient* httpClient() const;

    // ---- 代码托管平台 Provider ----
    // 返回 Provider 工厂，用于按平台/remote URL 获取对应的 ICodeHostingProvider
    ProviderFactory* providerFactory() const;

    // 便捷方法：根据 git remote URL 自动识别平台并返回 Provider
    ICodeHostingProvider* providerForRemote(const QString& remoteUrl);

    // 便捷方法：获取默认 Provider（GitHub）
    ICodeHostingProvider* defaultProvider();

    // ---- 模型访问器 ----
    RepositoryListModel* repositoryListModel() const;
    FileChangeModel* fileChangeModel() const;
    DiffLineModel* diffLineModel() const;
    CommitHistoryModel* commitHistoryModel() const;
    CommitGraphModel* commitGraphModel() const;
    BranchListModel* branchListModel() const;
    StashListModel* stashListModel() const;
    RemoteListModel* remoteListModel() const;
    PullRequestModel* pullRequestModel() const;

    // ---- 控制器访问器 ----
    MainController* mainController() const;
    RepositoryController* repositoryController() const;
    ChangesController* changesController() const;
    HistoryController* historyController() const;
    BranchController* branchController() const;
    StashController* stashController() const;
    RemoteController* remoteController() const;
    AuthController* authController() const;
    PullRequestController* pullRequestController() const;
    SyncController* syncController() const;
    MergeController* mergeController() const;
    RebaseController* rebaseController() const;
    MergeService* mergeService() const;
    RebaseService* rebaseService() const;
    ConflictService* conflictService() const;
    MergeToolService* mergeToolService() const;
    ConflictController* conflictController() const;
    MergeToolController* mergeToolController() const;
    ConflictFileModel* conflictFileModel() const;
    MergeLineModel* mergeLineModel() const;

    // AI Commit Message（Phase 5）
    AIClient* aiClient() const;
    AIService* aiService() const;
    CommitMessageService* commitMessageService() const;
    AICommitController* aiCommitController() const;
    CommitMessageSuggestionModel* commitMessageSuggestionModel() const;

    // 命令行终端
    TerminalService* terminalService() const;
    TerminalController* terminalController() const;
    TerminalOutputModel* terminalOutputModel() const;

    // AI Code Review（Phase 6）
    CodeReviewService* codeReviewService() const;
    CodeReviewController* codeReviewController() const;
    ReviewFindingModel* reviewFindingModel() const;

    // ---- 主窗口 / 仓库管理 ----
    MainWindow* mainWindow() const;
    void setMainWindow(MainWindow* mainWindow);

    Repository currentRepository() const;
    void setCurrentRepository(const Repository& repository);
    void clearCurrentRepository();
    bool hasCurrentRepository() const;
    QString currentRepositoryPath() const;

    void loadRecentRepositories();
    void saveRecentRepositories();
    void addRecentRepository(const Repository& repository);
    void removeRecentRepository(const QString& localPath);
    void clearRepositoryData();
    void updateGitExecutablePath(const QString& gitExecutablePath);

signals:
    void initialized();
    void shutdownRequested();
    void mainWindowChanged(MainWindow* mainWindow);
    void currentRepositoryChanged(const Repository& repository);
    void currentRepositoryCleared();

private:
    void createDefaultObjects();
    void applyGitExecutablePath();

private:
    std::unique_ptr<SettingsService> m_settingsService;
    std::unique_ptr<GitService> m_gitService;
    std::unique_ptr<GitTaskRunner> m_gitTaskRunner;
    std::unique_ptr<RepositoryService> m_repositoryService;
    std::unique_ptr<BranchService> m_branchService;
    std::unique_ptr<StashService> m_stashService;
    std::unique_ptr<RemoteService> m_remoteService;
    std::unique_ptr<SyncService> m_syncService;
    std::unique_ptr<HttpClient> m_httpClient;
    std::unique_ptr<ProviderFactory> m_providerFactory;
    std::unique_ptr<CredentialStore> m_credentialStore;
    std::unique_ptr<AuthService> m_authService;
    std::unique_ptr<PullRequestService> m_pullRequestService;

    std::unique_ptr<RepositoryListModel> m_repositoryListModel;
    std::unique_ptr<FileChangeModel> m_fileChangeModel;
    std::unique_ptr<DiffLineModel> m_diffLineModel;
    std::unique_ptr<CommitHistoryModel> m_commitHistoryModel;
    std::unique_ptr<CommitGraphModel> m_commitGraphModel;
    std::unique_ptr<BranchListModel> m_branchListModel;
    std::unique_ptr<StashListModel> m_stashListModel;
    std::unique_ptr<RemoteListModel> m_remoteListModel;
    std::unique_ptr<PullRequestModel> m_pullRequestModel;

    std::unique_ptr<MainController> m_mainController;
    std::unique_ptr<RepositoryController> m_repositoryController;
    std::unique_ptr<ChangesController> m_changesController;
    std::unique_ptr<HistoryController> m_historyController;
    std::unique_ptr<BranchController> m_branchController;
    std::unique_ptr<StashController> m_stashController;
    std::unique_ptr<RemoteController> m_remoteController;
    std::unique_ptr<AuthController> m_authController;
    std::unique_ptr<PullRequestController> m_pullRequestController;
    std::unique_ptr<SyncController> m_syncController;
    std::unique_ptr<MergeService> m_mergeService;
    std::unique_ptr<RebaseService> m_rebaseService;
    std::unique_ptr<MergeController> m_mergeController;
    std::unique_ptr<RebaseController> m_rebaseController;
    std::unique_ptr<ConflictService> m_conflictService;
    std::unique_ptr<MergeToolService> m_mergeToolService;
    std::unique_ptr<ConflictFileModel> m_conflictFileModel;
    std::unique_ptr<MergeLineModel> m_mergeLineModel;
    std::unique_ptr<ConflictController> m_conflictCtrl;
    std::unique_ptr<MergeToolController> m_mergeToolController;

    std::unique_ptr<AIClient> m_aiClient;
    std::unique_ptr<AIService> m_aiService;
    std::unique_ptr<CommitMessageService> m_commitMessageService;
    std::unique_ptr<CommitMessageSuggestionModel> m_commitMessageSuggestionModel;
    std::unique_ptr<AICommitController> m_aiCommitController;
    std::unique_ptr<TerminalService> m_terminalService;
    std::unique_ptr<TerminalOutputModel> m_terminalOutputModel;
    std::unique_ptr<TerminalController> m_terminalController;
    std::unique_ptr<CodeReviewService> m_codeReviewService;
    std::unique_ptr<ReviewFindingModel> m_reviewFindingModel;
    std::unique_ptr<CodeReviewController> m_codeReviewController;

    MainWindow* m_mainWindow;
    Repository m_currentRepository;
    bool m_initialized;
};

#endif // APPCONTEXT_H
