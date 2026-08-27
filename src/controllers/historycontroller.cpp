#include "historycontroller.h"

#include "domain/Tag.h"
#include "domain/diff.h"
#include "models/CommitGraphModel.h"
#include "models/commithistorymodel.h"
#include "models/difflinemodel.h"
#include "services/gitservice.h"
#include "services/gittaskrunner.h"

#include <QList>
#include <QtConcurrent/QtConcurrentRun>
#include <tuple>
#include <vector>

HistoryController::HistoryController(GitService* gitService,
                                     CommitHistoryModel* commitHistoryModel,
                                     CommitGraphModel* commitGraphModel,
                                     DiffLineModel* diffLineModel,
                                     QObject* parent)
    : BaseController(parent),
    m_gitService(gitService),
    m_commitHistoryModel(commitHistoryModel),
    m_commitGraphModel(commitGraphModel),
    m_diffLineModel(diffLineModel)
{
}

void HistoryController::setGitTaskRunner(GitTaskRunner* taskRunner)
{
    if (m_taskRunner) {
        QObject::disconnect(m_taskRunner, &GitTaskRunner::commitDiffLoaded,
                            this, &HistoryController::onCommitDiffLoaded);
    }

    m_taskRunner = taskRunner;

    if (m_taskRunner) {
        QObject::connect(m_taskRunner, &GitTaskRunner::commitDiffLoaded,
                         this, &HistoryController::onCommitDiffLoaded);
    }
}

void HistoryController::loadHistory(const QString& repoPath, int maxCount)
{
    if (!m_gitService) {
        emit historyLoaded(false, QStringLiteral("Git service is not available."));
        return;
    }

    const Result<QList<Commit>> result = m_gitService->commitHistory(repoPath, maxCount);

    if (result.isFailure()) {
        emit historyLoaded(false, result.errorMessage());
        emit errorOccurred(QStringLiteral("loadHistory"), result.errorMessage());
        return;
    }

    if (m_commitHistoryModel) {
        const QList<Commit>& commits = result.value();
        const std::vector<Commit> commitVector(commits.begin(), commits.end());
        m_commitHistoryModel->setCommits(commitVector);
    }

    emit historyLoaded(true, QString());
}

void HistoryController::loadCommitGraph(const QString& repoPath)
{
    if (!m_gitService) {
        emit commitGraphLoaded(false, QStringLiteral("Git service is not available."));
        return;
    }
    if (!m_commitGraphModel) {
        emit commitGraphLoaded(false, QStringLiteral("Commit graph model is not available."));
        return;
    }

    // 在后台线程执行所有 git 查询，避免阻塞 UI
    GitService* const svc = m_gitService;
    CommitGraphModel* const model = m_commitGraphModel;
    const QString repo = repoPath;

    QtConcurrent::run([svc, repo]() -> std::tuple<
        Result<QList<Commit>>,
        Result<QList<Branch>>,
        Result<QList<Tag>>,
        Result<QString>>
    {
        // ── 后台线程：所有 git 调用在此并行/串行执行 ──
        auto commitsResult  = svc->commitGraph(repo, 500);
        auto branchesResult = svc->branches(repo);
        auto tagsResult     = svc->tags(repo);
        auto headResult     = svc->currentHEAD(repo);
        return {commitsResult, branchesResult, tagsResult, headResult};
    }).then([this, model, repo](std::tuple<
        Result<QList<Commit>>,
        Result<QList<Branch>>,
        Result<QList<Tag>>,
        Result<QString>> results)
    {
        // ── 主线程回调 ──
        auto& [commitsResult, branchesResult, tagsResult, headResult] = results;

        if (commitsResult.isFailure()) {
            emit commitGraphLoaded(false, commitsResult.errorMessage());
            emit errorOccurred(QStringLiteral("loadCommitGraph"), commitsResult.errorMessage());
            return;
        }

        std::vector<Branch> branches;
        if (branchesResult.isSuccess()) {
            const QList<Branch>& blist = branchesResult.value();
            branches.assign(blist.begin(), blist.end());
        }

        std::vector<Tag> tags;
        if (tagsResult.isSuccess()) {
            const QList<Tag>& tlist = tagsResult.value();
            tags.assign(tlist.begin(), tlist.end());
        }

        QString headHash;
        if (headResult.isSuccess()) {
            headHash = headResult.value();
        }

        const QList<Commit>& qCommits = commitsResult.value();
        std::vector<Commit> commitVector(qCommits.begin(), qCommits.end());

        model->buildLayout(commitVector, branches, tags, headHash);

        emit commitGraphLoaded(true, QString());
    });
}

Result<Commit> HistoryController::getCommitDetail(const QString& repoPath, const QString& commitHash)
{
    if (!m_gitService) {
        return Result<Commit>::failure(QStringLiteral("Git service is not available."));
    }

    Q_UNUSED(repoPath);

    if (m_commitHistoryModel) {
        const Commit cached = m_commitHistoryModel->commitForHash(commitHash);
        if (cached.isValid()) {
            return Result<Commit>::success(cached);
        }
    }

    return Result<Commit>::failure(
        QStringLiteral("Commit %1 not found in loaded history.").arg(commitHash));
}

void HistoryController::loadCommitDiff(const QString& repoPath, const QString& commitHash)
{
    if (m_taskRunner) {
        m_taskRunner->loadCommitDiff(repoPath, commitHash);
        return;
    }

    // 回退：同步执行
    if (!m_gitService) {
        emit commitDiffLoaded(false, commitHash, QStringLiteral("Git service is not available."));
        return;
    }

    const Result<QString> rawDiffResult = m_gitService->rawCommitDiff(repoPath, commitHash);

    if (rawDiffResult.isFailure()) {
        emit commitDiffLoaded(false, commitHash, rawDiffResult.errorMessage());
        emit errorOccurred(QStringLiteral("loadCommitDiff"), rawDiffResult.errorMessage());
        return;
    }

    const Diff diff = Diff::fromUnifiedDiff(rawDiffResult.value());

    if (m_diffLineModel) {
        m_diffLineModel->setDiff(diff);
    }

    emit commitDiffLoaded(true, commitHash, QString());
}

void HistoryController::onCommitDiffLoaded(bool success, const QString& repoPath, const QString& commitHash, const QString& filePath, const QString& rawText, const QString& errorMessage)
{
    Q_UNUSED(repoPath);
    Q_UNUSED(filePath);

    if (success) {
        const Diff diff = Diff::fromUnifiedDiff(rawText);
        if (m_diffLineModel) {
            m_diffLineModel->setDiff(diff);
        }
    } else {
        emit errorOccurred(QStringLiteral("loadCommitDiff"), errorMessage);
    }

    emit commitDiffLoaded(success, commitHash, errorMessage);
}

QList<Commit> HistoryController::filterByAuthor(const QString& author) const
{
    if (!m_commitHistoryModel) return QList<Commit>();

    const std::vector<Commit>& commits = m_commitHistoryModel->commits();
    QList<Commit> filtered;
    for (const Commit& commit : commits) {
        if (commit.authorName().contains(author, Qt::CaseInsensitive))
            filtered.append(commit);
    }
    return filtered;
}

QList<Commit> HistoryController::searchByMessage(const QString& keyword) const
{
    if (!m_commitHistoryModel) return QList<Commit>();

    const std::vector<Commit>& commits = m_commitHistoryModel->commits();
    QList<Commit> matched;
    for (const Commit& commit : commits) {
        if (commit.message().contains(keyword, Qt::CaseInsensitive))
            matched.append(commit);
    }
    return matched;
}

CommitHistoryModel* HistoryController::commitHistoryModel() const
{
    return m_commitHistoryModel;
}

CommitGraphModel* HistoryController::commitGraphModel() const
{
    return m_commitGraphModel;
}

DiffLineModel* HistoryController::diffLineModel() const
{
    return m_diffLineModel;
}

void HistoryController::clear()
{
    if (m_commitHistoryModel) m_commitHistoryModel->clear();
    if (m_commitGraphModel)   m_commitGraphModel->clear();
    if (m_diffLineModel)      m_diffLineModel->clear();
}
