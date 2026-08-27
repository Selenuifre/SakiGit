cmake_minimum_required(VERSION 3.19)
project(SakiGit LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Concurrent Network Svg)

qt_standard_project_setup()

set(CMAKE_AUTORCC ON)

qt_add_executable(SakiGit
    MACOSX_BUNDLE
    src/main.cpp
    src/theme/ThemeManager.h src/theme/ThemeManager.cpp
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/mainwindow.ui
    src/resources/repomind.qrc
    src/domain/gittypes.h src/domain/gittypes.cpp
    src/domain/Stash.h src/domain/Stash.cpp
    src/domain/Remote.h src/domain/Remote.cpp
    src/domain/CommitMessageSuggestion.h src/domain/CommitMessageSuggestion.cpp
    src/ui/branchespage.h src/ui/branchespage.cpp
    src/ui/changespage.h src/ui/changespage.cpp
    src/ui/clonedialog.h src/ui/clonedialog.cpp
    src/ui/commitpanel.h src/ui/commitpanel.cpp
    src/ui/diffviewer.h src/ui/diffviewer.cpp
    src/ui/historypage.h src/ui/historypage.cpp
    src/ui/preferencesdialog.h src/ui/preferencesdialog.cpp
    src/ui/toastmanager.h src/ui/toastmanager.cpp
    src/ui/StashPage.h src/ui/StashPage.cpp
    src/ui/RemoteSettingsPage.h src/ui/RemoteSettingsPage.cpp
    src/ui/TerminalWidget.h src/ui/TerminalWidget.cpp
    src/ui/MergeDialog.h src/ui/MergeDialog.cpp
    src/ui/RebaseDialog.h src/ui/RebaseDialog.cpp
    src/ui/ConflictResolver.h src/ui/ConflictResolver.cpp
    src/ui/ConflictPage.h src/ui/ConflictPage.cpp
    src/ui/ConflictResolver.h src/ui/ConflictResolver.cpp
    src/ui/MergeDialog.h src/ui/MergeDialog.cpp
    src/ui/MergeEditor.h src/ui/MergeEditor.cpp
    src/ui/RebaseDialog.h src/ui/RebaseDialog.cpp
    src/ui/ThreeWayMergeViewer.h src/ui/ThreeWayMergeViewer.cpp
    src/ui/PageHeaderWidget.h src/ui/PageHeaderWidget.cpp
    src/ui/uiutils.h
    src/domain/repository.h src/domain/repository.cpp
    src/domain/commit.h src/domain/commit.cpp
    src/domain/branch.h src/domain/branch.cpp
    src/domain/filechange.h src/domain/filechange.cpp
    src/domain/diff.h src/domain/diff.cpp
    src/domain/Stash.h src/domain/Stash.cpp
    src/domain/ConflictFile.h src/domain/ConflictFile.cpp
    src/domain/GitOperationResult.h src/domain/GitOperationResult.cpp
    src/domain/MergeFileContent.h src/domain/MergeFileContent.cpp
    src/domain/DomainUtils.h
    src/domain/Operators.h
    src/domain/CommandLogEntry.h
    src/infrastructure/result.h
    src/infrastructure/AIClient.h src/infrastructure/AIClient.cpp
    src/infrastructure/gitcommandexecutor.h src/infrastructure/gitcommandexecutor.cpp
    src/infrastructure/logger.h src/infrastructure/logger.cpp
    src/infrastructure/PathUtils.h
    src/infrastructure/ResultUtils.h
    src/services/settingsservice.h src/services/settingsservice.cpp
    src/services/AIService.h src/services/AIService.cpp
    src/services/TerminalService.h src/services/TerminalService.cpp
    src/services/CommitMessageService.h src/services/CommitMessageService.cpp
    src/services/gitservice.h src/services/gitservice.cpp
    src/services/gittaskrunner.h src/services/gittaskrunner.cpp
    src/services/syncservice.h src/services/syncservice.cpp
    src/services/branchservice.h src/services/branchservice.cpp
    src/services/StashService.h src/services/StashService.cpp
    src/services/RemoteService.h src/services/RemoteService.cpp
    src/services/repositoryservice.h src/services/repositoryservice.cpp
    src/services/StashService.h src/services/StashService.cpp
    src/services/MergeService.h src/services/MergeService.cpp
    src/services/RebaseService.h src/services/RebaseService.cpp
    src/services/ConflictService.h src/services/ConflictService.cpp
    src/services/MergeToolService.h src/services/MergeToolService.cpp
    src/services/BaseService.h
    src/models/repositorylistmodel.h src/models/repositorylistmodel.cpp
    src/models/filechangemodel.h src/models/filechangemodel.cpp
    src/models/commithistorymodel.h src/models/commithistorymodel.cpp
    src/models/CommitMessageSuggestionModel.h src/models/CommitMessageSuggestionModel.cpp
    src/models/difflinemodel.h src/models/difflinemodel.cpp
    src/models/branchlistmodel.h src/models/branchlistmodel.cpp
    src/models/StashListModel.h src/models/StashListModel.cpp
    src/models/RemoteListModel.h src/models/RemoteListModel.cpp
    src/models/ConflictFileModel.h src/models/ConflictFileModel.cpp
    src/models/TerminalOutputModel.h src/models/TerminalOutputModel.cpp
    src/models/MergeLineModel.h src/models/MergeLineModel.cpp
    src/models/BaseTableModel.h
    src/models/BaseKeyedTableModel.h
    src/models/BaseListModel.h
    src/app/application.h src/app/application.cpp
    src/app/appcontext.h src/app/appcontext.cpp
    src/ui/repositorysidebar.h src/ui/repositorysidebar.cpp
    src/ui/PageHeaderWidget.h
    src/ui/PageHeaderWidget.cpp
    src/ui/uiutils.h
    src/infrastructure/error.h src/infrastructure/error.cpp
    src/infrastructure/result.cpp
    src/infrastructure/ResultUtils.h
    src/infrastructure/PathUtils.h
    src/controllers/AICommitController.h src/controllers/AICommitController.cpp
    src/controllers/maincontroller.h src/controllers/maincontroller.cpp
    src/controllers/repositorycontroller.h src/controllers/repositorycontroller.cpp
    src/controllers/changescontroller.h src/controllers/changescontroller.cpp
    src/controllers/historycontroller.h src/controllers/historycontroller.cpp
    src/controllers/branchcontroller.h src/controllers/branchcontroller.cpp
    src/controllers/StashController.h src/controllers/StashController.cpp
    src/controllers/RemoteController.h src/controllers/RemoteController.cpp
    src/controllers/TerminalController.h src/controllers/TerminalController.cpp
    src/controllers/synccontroller.h src/controllers/synccontroller.cpp
    src/controllers/StashController.h src/controllers/StashController.cpp
    src/controllers/MergeController.h src/controllers/MergeController.cpp
    src/controllers/RebaseController.h src/controllers/RebaseController.cpp
    src/controllers/ConflictController.h src/controllers/ConflictController.cpp
    src/controllers/MergeToolController.h src/controllers/MergeToolController.cpp
    src/controllers/BaseController.h
    src/controllers/GitOperationController.h
    src/domain/Account.h
    src/domain/Account.cpp
    src/domain/PullRequest.h
    src/domain/PullRequest.cpp
    src/infrastructure/HttpClient.h
    src/infrastructure/HttpClient.cpp
    src/services/AuthService.h
    src/services/AuthService.cpp
    src/services/CredentialStore.h
    src/services/CredentialStore.cpp
    src/services/GitHubApiClient.h
    src/services/GitHubApiClient.cpp
    src/services/PullRequestService.h
    src/services/PullRequestService.cpp
    src/controllers/AuthController.h
    src/controllers/AuthController.cpp
    src/controllers/PullRequestController.h
    src/controllers/PullRequestController.cpp
    src/models/PullRequestModel.h
    src/models/PullRequestModel.cpp
    src/ui/LoginDialog.h
    src/ui/LoginDialog.cpp
    src/ui/PullRequestsPage.h
    src/ui/PullRequestsPage.cpp
    src/domain/ReviewFinding.h
    src/domain/ReviewFinding.cpp
    src/services/CodeReviewService.h
    src/services/CodeReviewService.cpp
    src/controllers/CodeReviewController.h
    src/controllers/CodeReviewController.cpp
    src/models/ReviewFindingModel.h
    src/models/ReviewFindingModel.cpp
    src/ui/ReviewPanel.h
    src/ui/ReviewPanel.cpp
    src/domain/Tag.h
    src/models/CommitGraphModel.cpp src/models/CommitGraphModel.h
    src/domain/CodeHostingPlatform.h
    src/infrastructure/ICodeHostingProvider.h
    src/services/ProviderFactory.cpp
    src/services/ProviderFactory.h
    src/services/providers/GitHubProvider.cpp
    src/services/providers/GitHubProvider.h
    src/services/providers/GitLabProvider.cpp
    src/services/providers/GitLabProvider.h
    src/services/providers/GiteeProvider.cpp
    src/services/providers/GiteeProvider.h
    src/ui/CommitGraphView.h src/ui/CommitGraphView.cpp
    src/cli/CliMain.cpp
    src/cli/CliApplication.h src/cli/CliApplication.cpp
    src/cli/CliCommand.h
    src/cli/CliCommandParser.h src/cli/CliCommandParser.cpp
    src/cli/CliResult.h
    src/cli/GenerateCommitCommand.h src/cli/GenerateCommitCommand.cpp
    src/cli/ReviewCommand.h src/cli/ReviewCommand.cpp
    src/app.rc
)

target_include_directories(SakiGit PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(SakiGit
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt6::Concurrent
        Qt6::Network
        Qt6::Svg
)

include(GNUInstallDirs)

install(TARGETS SakiGit
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET SakiGit
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
