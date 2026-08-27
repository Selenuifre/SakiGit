#ifndef HISTORYCONTROLLER_H
#define HISTORYCONTROLLER_H

#include "domain/commit.h"
#include "infrastructure/result.h"

#include <QList>
#include "BaseController.h"
#include <QString>

class CommitGraphModel;
class CommitHistoryModel;
class DiffLineModel;
class GitService;
class GitTaskRunner;

class HistoryController : public BaseController
{
    Q_OBJECT

public:
    // 使用 Git 服务和提交历史/diff/图形模型构造控制器
    explicit HistoryController(GitService* gitService,
                               CommitHistoryModel* commitHistoryModel,
                               CommitGraphModel* commitGraphModel,
                               DiffLineModel* diffLineModel,
                               QObject* parent = nullptr);

    // 注入异步任务执行器
    void setGitTaskRunner(GitTaskRunner* taskRunner);

    // 加载提交历史
    void loadHistory(const QString& repoPath, int maxCount = 100);

    // 加载提交树形图（全部分支拓扑）
    void loadCommitGraph(const QString& repoPath);

    // 获取指定提交的详细信息
    Result<Commit> getCommitDetail(const QString& repoPath, const QString& commitHash);

    // 加载指定提交的 diff
    void loadCommitDiff(const QString& repoPath, const QString& commitHash);

    // 按作者过滤提交历史
    QList<Commit> filterByAuthor(const QString& author) const;

    // 按提交消息关键字搜索
    QList<Commit> searchByMessage(const QString& keyword) const;

    // 返回提交历史模型
    CommitHistoryModel* commitHistoryModel() const;

    // 返回提交图形模型
    CommitGraphModel* commitGraphModel() const;

    // 返回 diff 行模型
    DiffLineModel* diffLineModel() const;

    // 清空所有模型数据
    void clear();

signals:
    // 提交历史加载完成
    void historyLoaded(bool success, const QString& errorMessage);

    // 提交树形图加载完成
    void commitGraphLoaded(bool success, const QString& errorMessage);

    // 提交 diff 加载完成
    void commitDiffLoaded(bool success, const QString& commitHash, const QString& errorMessage);

private slots:
    void onCommitDiffLoaded(bool success, const QString& repoPath, const QString& commitHash, const QString& filePath, const QString& rawText, const QString& errorMessage);

private:
    // Git 核心服务
    GitService* m_gitService;

    // 提交历史模型
    CommitHistoryModel* m_commitHistoryModel;

    // 提交图形模型
    CommitGraphModel* m_commitGraphModel;

    // diff 行显示模型
    DiffLineModel* m_diffLineModel;

    // 异步任务执行器
    GitTaskRunner* m_taskRunner = nullptr;
};

#endif // HISTORYCONTROLLER_H
