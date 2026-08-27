#include "gittaskrunner.h"

#include "domain/filechange.h"

#include <QMetaObject>
#include <QMetaType>
#include <QPointer>
#include <QtConcurrent/QtConcurrent>

GitTaskRunner::GitTaskRunner(QObject* parent)
    : QObject(parent),
    m_activeTaskCount(0)
{
    qRegisterMetaType<Repository>("Repository");
    qRegisterMetaType<Diff>("Diff");
    qRegisterMetaType<QList<FileChange>>("QList<FileChange>");
    qRegisterMetaType<QList<Commit>>("QList<Commit>");
    qRegisterMetaType<QList<Branch>>("QList<Branch>");
}

QString GitTaskRunner::gitExecutablePath() const
{
    return m_service.gitExecutablePath();
}

void GitTaskRunner::setGitExecutablePath(const QString& gitExecutablePath)
{
    m_service.setGitExecutablePath(gitExecutablePath);
}

int GitTaskRunner::timeoutMs() const
{
    return m_service.timeoutMs();
}

void GitTaskRunner::setTimeoutMs(int timeoutMs)
{
    m_service.setTimeoutMs(timeoutMs);
}

bool GitTaskRunner::isBusy() const
{
    return m_activeTaskCount > 0;
}

int GitTaskRunner::activeTaskCount() const
{
    return m_activeTaskCount;
}

void GitTaskRunner::checkGitAvailable()
{
    const QString taskName = QStringLiteral("checkGitAvailable");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, taskName]() {
        const Result<void> result = service.checkGitAvailable();

        if (!self) {
            return;
        }

        QMetaObject::invokeMethod(self.data(), [self, result, taskName]() {
            if (!self) {
                return;
            }

            emit self->gitAvailableChecked(result.isSuccess(), result.errorMessage());
            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::openRepository(const QString& repositoryPath)
{
    const QString taskName = QStringLiteral("openRepository");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repositoryPath, taskName]() {
        const Result<Repository> result = service.openRepository(repositoryPath);

        if (!self) {
            return;
        }

        QMetaObject::invokeMethod(self.data(), [self, result, repositoryPath, taskName]() {
            if (!self) {
                return;
            }

            if (result.isSuccess()) {
                emit self->repositoryOpened(true, repositoryPath, result.value(), QString());
            } else {
                emit self->repositoryOpened(false, repositoryPath, Repository(), result.errorMessage());
            }

            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::loadStatus(const QString& repositoryPath)
{
    const QString taskName = QStringLiteral("loadStatus");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repositoryPath, taskName]() {
        const Result<QList<FileChange>> result = service.status(repositoryPath);

        if (!self) {
            return;
        }

        QMetaObject::invokeMethod(self.data(), [self, result, repositoryPath, taskName]() {
            if (!self) {
                return;
            }

            if (result.isSuccess()) {
                emit self->statusLoaded(true, repositoryPath, result.value(), QString());
            } else {
                emit self->statusLoaded(false, repositoryPath, QList<FileChange>(), result.errorMessage());
            }

            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::loadRawDiff(const QString& repositoryPath, const QString& filePath)
{
    const QString taskName = QStringLiteral("loadRawDiff");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repositoryPath, filePath, taskName]() {
        const Result<QString> result = service.rawDiff(repositoryPath, filePath);

        if (!self) {
            return;
        }

        QMetaObject::invokeMethod(self.data(), [self, result, repositoryPath, filePath, taskName]() {
            if (!self) {
                return;
            }

            if (result.isSuccess()) {
                emit self->rawDiffLoaded(true, repositoryPath, filePath, result.value(), QString());
            } else {
                emit self->rawDiffLoaded(false, repositoryPath, filePath, QString(), result.errorMessage());
            }

            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::loadDiff(const QString& repositoryPath, const QString& filePath, bool staged)
{
    const QString taskName = QStringLiteral("loadDiff");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repositoryPath, filePath, staged, taskName]() {
        const Result<Diff> result = service.diff(repositoryPath, filePath, staged);

        if (!self) {
            return;
        }

        QMetaObject::invokeMethod(self.data(), [self, result, repositoryPath, filePath, taskName]() {
            if (!self) {
                return;
            }

            if (result.isSuccess()) {
                emit self->diffLoaded(true, repositoryPath, filePath, result.value(), QString());
            } else {
                emit self->diffLoaded(false, repositoryPath, filePath, Diff(), result.errorMessage());
            }

            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::loadCommitHistory(const QString& repositoryPath, int maxCount)
{
    const QString taskName = QStringLiteral("loadCommitHistory");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repositoryPath, maxCount, taskName]() {
        const Result<QList<Commit>> result = service.commitHistory(repositoryPath, maxCount);

        if (!self) {
            return;
        }

        QMetaObject::invokeMethod(self.data(), [self, result, repositoryPath, taskName]() {
            if (!self) {
                return;
            }

            if (result.isSuccess()) {
                emit self->commitHistoryLoaded(true, repositoryPath, result.value(), QString());
            } else {
                emit self->commitHistoryLoaded(false, repositoryPath, QList<Commit>(), result.errorMessage());
            }

            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::loadBranches(const QString& repositoryPath)
{
    const QString taskName = QStringLiteral("loadBranches");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repositoryPath, taskName]() {
        const Result<QList<Branch>> result = service.branches(repositoryPath);

        if (!self) {
            return;
        }

        QMetaObject::invokeMethod(self.data(), [self, result, repositoryPath, taskName]() {
            if (!self) {
                return;
            }

            if (result.isSuccess()) {
                emit self->branchesLoaded(true, repositoryPath, result.value(), QString());
            } else {
                emit self->branchesLoaded(false, repositoryPath, QList<Branch>(), result.errorMessage());
            }

            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::loadCurrentBranch(const QString& repositoryPath)
{
    const QString taskName = QStringLiteral("loadCurrentBranch");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repositoryPath, taskName]() {
        const Result<QString> result = service.currentBranch(repositoryPath);

        if (!self) {
            return;
        }

        QMetaObject::invokeMethod(self.data(), [self, result, repositoryPath, taskName]() {
            if (!self) {
                return;
            }

            if (result.isSuccess()) {
                emit self->currentBranchLoaded(true, repositoryPath, result.value(), QString());
            } else {
                emit self->currentBranchLoaded(false, repositoryPath, QString(), result.errorMessage());
            }

            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::stageFile(const QString& repositoryPath, const QString& filePath)
{
    const QString taskName = QStringLiteral("stageFile");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repositoryPath, filePath, taskName]() {
        const Result<void> result = service.stageFile(repositoryPath, filePath);

        if (!self) {
            return;
        }

        QMetaObject::invokeMethod(self.data(), [self, result, repositoryPath, filePath, taskName]() {
            if (!self) {
                return;
            }

            emit self->fileStaged(result.isSuccess(), repositoryPath, filePath, result.errorMessage());
            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::unstageFile(const QString& repositoryPath, const QString& filePath)
{
    const QString taskName = QStringLiteral("unstageFile");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repositoryPath, filePath, taskName]() {
        const Result<void> result = service.unstageFile(repositoryPath, filePath);

        if (!self) {
            return;
        }

        QMetaObject::invokeMethod(self.data(), [self, result, repositoryPath, filePath, taskName]() {
            if (!self) {
                return;
            }

            emit self->fileUnstaged(result.isSuccess(), repositoryPath, filePath, result.errorMessage());
            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::commit(const QString& repositoryPath, const QString& message)
{
    const QString taskName = QStringLiteral("commit");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repositoryPath, message, taskName]() {
        const Result<void> result = service.commit(repositoryPath, message);

        if (!self) {
            return;
        }

        QMetaObject::invokeMethod(self.data(), [self, result, repositoryPath, taskName]() {
            if (!self) {
                return;
            }

            emit self->commitFinished(result.isSuccess(), repositoryPath, result.errorMessage());
            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::fetch(const QString& repositoryPath, const QString& remoteName)
{
    const QString taskName = QStringLiteral("fetch");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repositoryPath, remoteName, taskName]() {
        const Result<void> result = service.fetch(repositoryPath, remoteName);

        if (!self) {
            return;
        }

        QMetaObject::invokeMethod(self.data(), [self, result, repositoryPath, remoteName, taskName]() {
            if (!self) {
                return;
            }

            emit self->fetchFinished(result.isSuccess(), repositoryPath, remoteName, result.errorMessage());
            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::pull(const QString& repositoryPath,
                           const QString& remoteName,
                           bool useRebase)
{
    const QString taskName = QStringLiteral("pull");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repositoryPath, remoteName, useRebase, taskName]() {
        const Result<void> result = service.pull(repositoryPath, remoteName, useRebase);

        if (!self) {
            return;
        }

        QMetaObject::invokeMethod(self.data(), [self, result, repositoryPath, taskName]() {
            if (!self) {
                return;
            }

            emit self->pullFinished(result.isSuccess(), repositoryPath, result.errorMessage());
            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::push(const QString& repositoryPath,
                         const QString& remoteName,
                         const QString& branchName)
{
    const QString taskName = QStringLiteral("push");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repositoryPath, remoteName, branchName, taskName]() {
        const Result<void> result = service.push(repositoryPath, remoteName, branchName);

        if (!self) {
            return;
        }

        QMetaObject::invokeMethod(self.data(), [self, result, repositoryPath, remoteName, branchName, taskName]() {
            if (!self) {
                return;
            }

            emit self->pushFinished(result.isSuccess(),
                                    repositoryPath,
                                    remoteName,
                                    branchName,
                                    result.errorMessage());

            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::loadCommitDiff(const QString& repositoryPath,
                                    const QString& commitHash,
                                    const QString& filePath)
{
    const QString taskName = QStringLiteral("loadCommitDiff");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repositoryPath, commitHash, filePath, taskName]() {
        const Result<QString> result = service.rawCommitDiff(repositoryPath, commitHash, filePath);

        if (!self) return;

        QMetaObject::invokeMethod(self.data(), [self, result, repositoryPath, commitHash, filePath, taskName]() {
            if (!self) return;

            if (result.isSuccess()) {
                emit self->commitDiffLoaded(true, repositoryPath, commitHash, filePath, result.value(), QString());
            } else {
                emit self->commitDiffLoaded(false, repositoryPath, commitHash, filePath, QString(), result.errorMessage());
            }

            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::stageAllFiles(const QString& repositoryPath)
{
    const QString taskName = QStringLiteral("stageAllFiles");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repositoryPath, taskName]() {
        // 获取所有未暂存文件并逐一暂存
        const Result<QList<FileChange>> statusResult = service.status(repositoryPath);
        if (statusResult.isFailure()) {
            if (!self) return;
            QMetaObject::invokeMethod(self.data(), [self, repositoryPath, taskName, msg = statusResult.errorMessage()]() {
                if (!self) return;
                emit self->allFilesStaged(false, repositoryPath, msg);
                self->finishTask(taskName);
            }, Qt::QueuedConnection);
            return;
        }

        QString errorMessage;
        for (const FileChange& change : statusResult.value()) {
            if (!change.isStaged() && !change.isConflict()) {
                const Result<void> stageResult = service.stageFile(repositoryPath, change.path());
                if (stageResult.isFailure()) {
                    errorMessage = stageResult.errorMessage();
                    break;
                }
            }
        }

        if (!self) return;
        QMetaObject::invokeMethod(self.data(), [self, repositoryPath, taskName, errorMessage]() {
            if (!self) return;
            emit self->allFilesStaged(errorMessage.isEmpty(), repositoryPath,
                errorMessage.isEmpty() ? QString() : errorMessage);
            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::initRepository(const QString& path)
{
    const QString taskName = QStringLiteral("initRepository");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, path, taskName]() {
        const Result<void> initResult = service.init(path);

        if (!self) return;

        Repository repository;
        QString errorMessage;

        if (initResult.isSuccess()) {
            const Result<Repository> openResult = service.openRepository(path);
            if (openResult.isSuccess()) {
                repository = openResult.value();
            } else {
                errorMessage = openResult.errorMessage();
            }
        } else {
            errorMessage = initResult.errorMessage();
        }

        QMetaObject::invokeMethod(self.data(), [self, path, repository, errorMessage, taskName]() {
            if (!self) return;

            if (errorMessage.isEmpty()) {
                emit self->repositoryInitialized(true, path, repository, QString());
            } else {
                emit self->repositoryInitialized(false, path, Repository(), errorMessage);
            }

            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::cloneRepository(const QString& url, const QString& targetPath)
{
    const QString taskName = QStringLiteral("cloneRepository");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, url, targetPath, taskName]() {
        const Result<void> cloneResult = service.clone(url, targetPath);

        if (!self) return;

        Repository repository;
        QString errorMessage;

        if (cloneResult.isSuccess()) {
            const Result<Repository> openResult = service.openRepository(targetPath);
            if (openResult.isSuccess()) {
                repository = openResult.value();
            } else {
                errorMessage = openResult.errorMessage();
            }
        } else {
            errorMessage = cloneResult.errorMessage();
        }

        QMetaObject::invokeMethod(self.data(), [self, url, repository, errorMessage, taskName]() {
            if (!self) return;

            if (errorMessage.isEmpty()) {
                emit self->repositoryCloned(true, url, repository, QString());
            } else {
                emit self->repositoryCloned(false, url, Repository(), errorMessage);
            }

            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::createBranch(const QString& repoPath, const QString& branchName)
{
    const QString taskName = QStringLiteral("createBranch");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repoPath, branchName, taskName]() {
        const Result<void> result = service.createBranch(repoPath, branchName);

        if (!self) return;

        QMetaObject::invokeMethod(self.data(), [self, result, repoPath, branchName, taskName]() {
            if (!self) return;
            emit self->branchCreated(result.isSuccess(), repoPath, branchName, result.errorMessage());
            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::checkoutBranch(const QString& repoPath, const QString& branchName)
{
    const QString taskName = QStringLiteral("checkoutBranch");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repoPath, branchName, taskName]() {
        const Result<void> result = service.checkoutBranch(repoPath, branchName);

        if (!self) return;

        QMetaObject::invokeMethod(self.data(), [self, result, repoPath, branchName, taskName]() {
            if (!self) return;
            emit self->branchCheckedOut(result.isSuccess(), repoPath, branchName, result.errorMessage());
            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::deleteBranch(const QString& repoPath, const QString& branchName, bool force)
{
    const QString taskName = QStringLiteral("deleteBranch");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repoPath, branchName, force, taskName]() {
        const Result<void> result = service.deleteBranch(repoPath, branchName, force);

        if (!self) return;

        QMetaObject::invokeMethod(self.data(), [self, result, repoPath, branchName, taskName]() {
            if (!self) return;
            emit self->branchDeleted(result.isSuccess(), repoPath, branchName, result.errorMessage());
            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::createStash(const QString& repoPath, const QString& message)
{
    const QString taskName = QStringLiteral("createStash");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repoPath, message, taskName]() {
        const Result<void> result = service.createStash(repoPath, message);

        if (!self) return;

        QMetaObject::invokeMethod(self.data(), [self, result, repoPath, message, taskName]() {
            if (!self) return;
            emit self->stashCreated(result.isSuccess(), repoPath, message, result.errorMessage());
            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::applyStash(const QString& repoPath, int index)
{
    const QString taskName = QStringLiteral("applyStash");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repoPath, index, taskName]() {
        const Result<void> result = service.applyStash(repoPath, index);

        if (!self) return;

        QMetaObject::invokeMethod(self.data(), [self, result, repoPath, index, taskName]() {
            if (!self) return;
            emit self->stashApplied(result.isSuccess(), repoPath, index, result.errorMessage());
            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::dropStash(const QString& repoPath, int index)
{
    const QString taskName = QStringLiteral("dropStash");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repoPath, index, taskName]() {
        const Result<void> result = service.dropStash(repoPath, index);

        if (!self) return;

        QMetaObject::invokeMethod(self.data(), [self, result, repoPath, index, taskName]() {
            if (!self) return;
            emit self->stashDropped(result.isSuccess(), repoPath, index, result.errorMessage());
            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::showStashDiff(const QString& repoPath, int index)
{
    const QString taskName = QStringLiteral("showStashDiff");
    startTask(taskName);

    const GitService service = m_service;
    QPointer<GitTaskRunner> self(this);

    QtConcurrent::run([self, service, repoPath, index, taskName]() {
        const Result<Diff> result = service.showStashDiff(repoPath, index);

        if (!self) return;

        QMetaObject::invokeMethod(self.data(), [self, result, repoPath, index, taskName]() {
            if (!self) return;

            if (result.isSuccess()) {
                emit self->stashDiffLoaded(true, repoPath, index, result.value(), QString());
            } else {
                emit self->stashDiffLoaded(false, repoPath, index, Diff(), result.errorMessage());
            }

            self->finishTask(taskName);
        }, Qt::QueuedConnection);
    });
}

void GitTaskRunner::startTask(const QString& taskName)
{
    const bool wasBusy = isBusy();

    ++m_activeTaskCount;

    emit taskStarted(taskName);

    if (!wasBusy && isBusy()) {
        emit busyChanged(true);
    }
}

void GitTaskRunner::finishTask(const QString& taskName)
{
    if (m_activeTaskCount > 0) {
        --m_activeTaskCount;
    }

    emit taskFinished(taskName);

    if (!isBusy()) {
        emit busyChanged(false);
    }
}
