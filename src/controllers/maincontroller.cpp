#include "maincontroller.h"

#include "BaseController.h"
#include "branchcontroller.h"
#include "changescontroller.h"
#include "ConflictController.h"
#include "historycontroller.h"
#include "repositorycontroller.h"
#include "PullRequestController.h"
#include "RemoteController.h"
#include "StashController.h"
#include "synccontroller.h"
#include "domain/branch.h"
#include "domain/ConflictFile.h"
#include "domain/filechange.h"
#include "domain/Stash.h"
#include "domain/Remote.h"
#include "models/branchlistmodel.h"
#include "models/commithistorymodel.h"
#include "models/ConflictFileModel.h"
#include "models/filechangemodel.h"
#include "models/RemoteListModel.h"
#include "models/StashListModel.h"
#include "services/gitservice.h"
#include "services/gittaskrunner.h"
#include "services/settingsservice.h"

#include <QFileInfo>
#include <QPointer>
#include <QtConcurrent/QtConcurrentRun>
#include <tuple>

MainController::MainController(QObject* parent)
    : QObject(parent),
    m_repositoryController(nullptr),
    m_changesController(nullptr),
    m_historyController(nullptr),
    m_branchController(nullptr),
    m_stashController(nullptr),
    m_remoteController(nullptr),
    m_syncController(nullptr),
    m_conflictController(nullptr),
    m_pullRequestController(nullptr),
    m_gitService(nullptr),
    m_settingsService(nullptr)
{
}

// -- 依赖注入 --

void MainController::setRepositoryController(RepositoryController* controller)
{
    if (m_repositoryController) {
        QObject::disconnect(m_repositoryController, &BaseController::errorOccurred,
                            this, &MainController::onSubControllerError);
    }

    m_repositoryController = controller;
    connectErrorSignals();
}

void MainController::setChangesController(ChangesController* controller)
{
    if (m_changesController) {
        QObject::disconnect(m_changesController, &BaseController::errorOccurred,
                            this, &MainController::onSubControllerError);
    }

    m_changesController = controller;
    connectErrorSignals();
}

void MainController::setHistoryController(HistoryController* controller)
{
    if (m_historyController) {
        QObject::disconnect(m_historyController, &BaseController::errorOccurred,
                            this, &MainController::onSubControllerError);
    }

    m_historyController = controller;
    connectErrorSignals();
}

void MainController::setBranchController(BranchController* controller)
{
    if (m_branchController) {
        QObject::disconnect(m_branchController, &BaseController::errorOccurred,
                            this, &MainController::onSubControllerError);
    }

    m_branchController = controller;
    connectErrorSignals();
}

void MainController::setStashController(StashController* controller)
{
    if (m_stashController) {
        QObject::disconnect(m_stashController, &BaseController::errorOccurred,
                            this, &MainController::onSubControllerError);
    }

    m_stashController = controller;
    connectErrorSignals();
}

void MainController::setRemoteController(RemoteController* controller)
{
    if (m_remoteController) {
        QObject::disconnect(m_remoteController, &RemoteController::errorOccurred,
                            this, &MainController::onSubControllerError);
    }

    m_remoteController = controller;
    connectErrorSignals();
}

void MainController::setSyncController(SyncController* controller)
{
    if (m_syncController) {
        QObject::disconnect(m_syncController, &BaseController::errorOccurred,
                            this, &MainController::onSubControllerError);
    }

    m_syncController = controller;
    connectErrorSignals();
}

void MainController::setConflictController(ConflictController* controller)
{
    if (m_conflictController) {
        QObject::disconnect(m_conflictController, &BaseController::errorOccurred,
                            this, &MainController::onSubControllerError);
    }
    m_conflictController = controller;
    connectErrorSignals();
}

void MainController::setGitService(GitService* gitService)
{
    m_gitService = gitService;
}

void MainController::setSettingsService(SettingsService* settingsService)
{
    m_settingsService = settingsService;
}

void MainController::setGitTaskRunner(GitTaskRunner* taskRunner)
{
    m_gitTaskRunner = taskRunner;
}

void MainController::setPullRequestController(PullRequestController* controller)
{
    m_pullRequestController = controller;
}

// -- 操作 --

void MainController::openRepository(const QString& path)
{
    if (!m_repositoryController) {
        emit globalError(QStringLiteral("打开仓库失败"),
                         QStringLiteral("Repository controller is not available."));
        return;
    }

    m_repositoryController->openRepository(path);
}

void MainController::switchToRepository(const QString& repoPath)
{
    m_currentRepoPath = repoPath;

    // 系统自动刷新不显示在终端
    if (m_gitService)
        m_gitService->executor().setSilentMode(true);

    // ── 值拷贝 GitService，每个后台线程拥有独立的 executor 状态，
    //     避免与主线程的 m_silentMode 等字段产生数据竞争 ──
    const GitService svc = m_gitService ? *m_gitService : GitService();
    const QString repo = repoPath;

    FileChangeModel* const fcm = m_changesController
        ? m_changesController->fileChangeModel() : nullptr;
    CommitHistoryModel* const chm = m_historyController
        ? m_historyController->commitHistoryModel() : nullptr;
    BranchListModel* const blm = m_branchController
        ? m_branchController->branchListModel() : nullptr;
    StashListModel* const slm = m_stashController
        ? m_stashController->stashListModel() : nullptr;
    RemoteListModel* const rlm = m_remoteController
        ? m_remoteController->remoteListModel() : nullptr;
    ConflictFileModel* const cfm = m_conflictController
        ? m_conflictController->conflictFileModel() : nullptr;

    QPointer<MainController> self(this);

    // ── 并行执行所有 git 查询（每个 QtConcurrent::run 在后台线程运行） ──

    // 1. 工作区变更
    QtConcurrent::run([svc, repo]() {
        return svc.status(repo);
    }).then([self, this, fcm, cfm, repo](Result<QList<FileChange>> result) {
        if (!self) return;
        if (result.isSuccess() && fcm) {
            const QList<FileChange>& changes = result.value();
            fcm->setFileChanges(std::vector<FileChange>(changes.begin(), changes.end()));
            // 优化5: 冲突检测复用同一份 git status 结果
            if (cfm) {
                std::vector<ConflictFile> conflicts;
                for (const FileChange& fc : changes) {
                    if (fc.isConflict()) {
                        ConflictFile cf;
                        cf.setPath(fc.path());
                        // 根据仓库状态推断冲突类型
                        const QString gitDir = repo + QStringLiteral("/.git");
                        if (QFileInfo::exists(gitDir + QStringLiteral("/rebase-merge"))
                            || QFileInfo::exists(gitDir + QStringLiteral("/rebase-apply"))) {
                            cf.setConflictType(QStringLiteral("rebase"));
                        } else {
                            cf.setConflictType(QStringLiteral("merge"));
                        }
                        if (cf.isValid())
                            conflicts.push_back(cf);
                    }
                }
                cfm->setConflictFiles(conflicts);
            }
        }
        if (self->m_changesController)
            emit self->m_changesController->changesRefreshed(result.isSuccess(),
                result.isFailure() ? result.errorMessage() : QString());
    });

    // 2. 提交历史
    QtConcurrent::run([svc, repo]() {
        return svc.commitHistory(repo, 100);
    }).then([self, this, chm](Result<QList<Commit>> result) {
        if (!self) return;
        if (result.isSuccess() && chm) {
            const QList<Commit>& commits = result.value();
            chm->setCommits(std::vector<Commit>(commits.begin(), commits.end()));
        }
        if (self->m_historyController)
            emit self->m_historyController->historyLoaded(result.isSuccess(),
                result.isFailure() ? result.errorMessage() : QString());
    });

    // 3. 提交树形图（已有异步实现，直接调用）
    if (m_historyController) {
        m_historyController->loadCommitGraph(repo);
    }

    // 4. 分支列表
    QtConcurrent::run([svc, repo]() {
        return svc.branches(repo);
    }).then([self, this, blm](Result<QList<Branch>> result) {
        if (!self) return;
        if (result.isSuccess() && blm) {
            const QList<Branch>& branches = result.value();
            blm->setBranches(std::vector<Branch>(branches.begin(), branches.end()));
        }
        if (self->m_branchController)
            emit self->m_branchController->branchesLoaded(result.isSuccess(),
                result.isFailure() ? result.errorMessage() : QString());
    });

    // 5. Stash 列表
    QtConcurrent::run([svc, repo]() {
        return svc.listStashes(repo);
    }).then([self, this, slm](Result<std::vector<Stash>> result) {
        if (!self) return;
        if (result.isSuccess() && slm) {
            slm->setStashes(result.value());
        }
        if (self->m_stashController)
            emit self->m_stashController->stashesLoaded(result.isSuccess(),
                result.isFailure() ? result.errorMessage() : QString());
    });

    // 6. Remote 列表
    QtConcurrent::run([svc, repo]() {
        return svc.remoteDetails(repo);
    }).then([self, this, rlm](Result<std::vector<Remote>> result) {
        if (!self) return;
        if (result.isSuccess() && rlm) {
            rlm->setRemotes(result.value());
        }
        if (self->m_remoteController)
            emit self->m_remoteController->remotesLoaded(result.isSuccess(),
                result.isFailure() ? result.errorMessage() : QString());
    });

    // 恢复主线程的 silent 模式（各后台线程使用独立拷贝，互不影响）
    if (m_gitService)
        m_gitService->executor().setSilentMode(false);

    emit repositorySwitched(repo);
}

void MainController::closeCurrentRepository()
{
    m_currentRepoPath.clear();

    if (m_changesController) {
        m_changesController->clear();
    }

    if (m_historyController) {
        m_historyController->clear();
    }

    if (m_branchController) {
        m_branchController->clear();
    }

    if (m_stashController) {
        m_stashController->clear();
    }

    if (m_remoteController) {
        m_remoteController->clear();
    }

    if (m_pullRequestController) {
        m_pullRequestController->clear();
    }

    emit repositoryClosed();
}

void MainController::refreshAll()
{
    if (m_currentRepoPath.isEmpty()) {
        return;
    }

    if (m_gitService)
        m_gitService->executor().setSilentMode(true);

    if (m_changesController) {
        m_changesController->refreshChanges(m_currentRepoPath);
    }

    if (m_historyController) {
        m_historyController->loadHistory(m_currentRepoPath);
        m_historyController->loadCommitGraph(m_currentRepoPath);
    }

    if (m_branchController) {
        m_branchController->loadBranches(m_currentRepoPath);
    }

    if (m_stashController) {
        m_stashController->loadStashes(m_currentRepoPath);
    }
    if (m_conflictController) {
        m_conflictController->loadConflicts(m_currentRepoPath);
    }

    if (m_gitService)
        m_gitService->executor().setSilentMode(false);

    if (m_remoteController) {
        m_remoteController->loadRemotes(m_currentRepoPath);
    }

    emit refreshCompleted();
}

// -- 查询 --

QString MainController::currentRepositoryPath() const
{
    return m_currentRepoPath;
}

RepositoryController* MainController::repositoryController() const
{
    return m_repositoryController;
}

ChangesController* MainController::changesController() const
{
    return m_changesController;
}

HistoryController* MainController::historyController() const
{
    return m_historyController;
}

BranchController* MainController::branchController() const
{
    return m_branchController;
}

StashController* MainController::stashController() const
{
    return m_stashController;
}

RemoteController* MainController::remoteController() const
{
    return m_remoteController;
}

SyncController* MainController::syncController() const
{
    return m_syncController;
}

ConflictController* MainController::conflictController() const
{
    return m_conflictController;
}

PullRequestController* MainController::pullRequestController() const
{
    return m_pullRequestController;
}

// -- 私有方法 --

void MainController::onSubControllerError(const QString& operation, const QString& errorMessage)
{
    emit globalError(operation, errorMessage);
}

void MainController::connectErrorSignals()
{
    // errorOccurred 信号已统一移至 BaseController，
    // 所有子控制器通过基类指针统一连接即可。
    BaseController* controllers[] = {
        m_repositoryController,
        m_changesController,
        m_historyController,
        m_branchController,
        m_stashController,
        m_syncController,
        m_conflictController,
    };

    for (BaseController* ctrl : controllers) {
        if (!ctrl)
            continue;
        QObject::disconnect(ctrl, &BaseController::errorOccurred,
                            this, &MainController::onSubControllerError);
        QObject::connect(ctrl, &BaseController::errorOccurred,
                         this, &MainController::onSubControllerError);
    }

    // RemoteController 暂未继承 BaseController，独立连接错误信号
    if (m_remoteController) {
        QObject::disconnect(m_remoteController, &RemoteController::errorOccurred,
                            this, &MainController::onSubControllerError);
        QObject::connect(m_remoteController, &RemoteController::errorOccurred,
                         this, &MainController::onSubControllerError);
    }
}
