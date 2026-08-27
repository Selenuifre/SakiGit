#ifndef STASHCONTROLLER_H
#define STASHCONTROLLER_H

#include "infrastructure/result.h"

#include "BaseController.h"
#include <QString>

#include "domain/diff.h"

class DiffLineModel;
class GitTaskRunner;
class StashListModel;
class StashService;

class StashController : public BaseController
{
    Q_OBJECT

public:
    explicit StashController(StashService* stashService,
                             StashListModel* stashListModel,
                             DiffLineModel* diffLineModel,
                             QObject* parent = nullptr);

    void setGitTaskRunner(GitTaskRunner* taskRunner);

    void loadStashes(const QString& repoPath);
    Result<void> saveStash(const QString& repoPath, const QString& message);
    Result<void> applyStash(const QString& repoPath, int index);
    Result<void> dropStash(const QString& repoPath, int index);
    void showStashDiff(const QString& repoPath, int index);

    StashListModel* stashListModel() const;
    void clear();

signals:
    void stashesLoaded(bool success, const QString& errorMessage);
    void stashDiffLoaded(bool success, int stashIndex, const QString& errorMessage);

private slots:
    void onStashCreated(bool success, const QString& repoPath, const QString& message, const QString& errorMessage);
    void onStashApplied(bool success, const QString& repoPath, int index, const QString& errorMessage);
    void onStashDropped(bool success, const QString& repoPath, int index, const QString& errorMessage);
    void onStashDiffLoaded(bool success, const QString& repoPath, int index, const Diff& diff, const QString& errorMessage);

private:
    StashService* m_stashService;
    StashListModel* m_stashListModel;
    DiffLineModel* m_diffLineModel;
    GitTaskRunner* m_taskRunner = nullptr;
};

#endif // STASHCONTROLLER_H
