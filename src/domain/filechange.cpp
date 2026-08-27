#include "DomainUtils.h"
#include "fileChange.h"

FileChange::FileChange()
    : m_status(Status::Unknown),
    m_stageState(StageState::Unknown)
{
}

FileChange::FileChange(const QString& path)
    : FileChange()
{
    setPath(path);
}

FileChange::FileChange(const QString& path, Status status)
    : FileChange(path)
{
    setStatus(status);
}

FileChange::FileChange(const QString& path, Status status, StageState stageState)
    : FileChange(path, status)
{
    setStageState(stageState);
}

QString FileChange::path() const
{
    return m_path;
}

void FileChange::setPath(const QString& path)
{
    m_path = GitTypes::cleanPathSeparators(path);
}

QString FileChange::oldPath() const
{
    return m_oldPath;
}

void FileChange::setOldPath(const QString& oldPath)
{
    m_oldPath = GitTypes::cleanPathSeparators(oldPath);
}

FileChange::Status FileChange::status() const
{
    return m_status;
}

void FileChange::setStatus(Status status)
{
    m_status = status;
}

FileChange::StageState FileChange::stageState() const
{
    return m_stageState;
}

void FileChange::setStageState(StageState stageState)
{
    m_stageState = stageState;
}

QString FileChange::indexStatus() const
{
    return m_indexStatus;
}

void FileChange::setIndexStatus(const QString& indexStatus)
{
    m_indexStatus = indexStatus.trimmed().left(1);
}

QString FileChange::worktreeStatus() const
{
    return m_worktreeStatus;
}

void FileChange::setWorktreeStatus(const QString& worktreeStatus)
{
    m_worktreeStatus = worktreeStatus.trimmed().left(1);
}

bool FileChange::isValid() const
{
    return !m_path.isEmpty();
}

bool FileChange::isStaged() const
{
    return m_stageState == StageState::Staged;
}

bool FileChange::isUnstaged() const
{
    return m_stageState == StageState::Unstaged;
}

bool FileChange::isPartiallyStaged() const
{
    return m_stageState == StageState::PartiallyStaged;
}

bool FileChange::isConflict() const
{
    return m_status == Status::Conflicted || m_stageState == StageState::Conflict;
}

bool FileChange::isDeleted() const
{
    return m_status == Status::Deleted;
}

bool FileChange::isRenamed() const
{
    return m_status == Status::Renamed;
}

bool FileChange::isUntracked() const
{
    return m_status == Status::Untracked;
}

bool FileChange::hasOldPath() const
{
    return !m_oldPath.isEmpty();
}

QString FileChange::displayPath() const
{
    if (isRenamed() && hasOldPath()) {
        return m_oldPath + QStringLiteral(" -> ") + m_path;
    }

    return m_path;
}

QString FileChange::statusText() const
{
    return GitTypes::toString(m_status);
}

QString FileChange::stageStateText() const
{
    return GitTypes::toString(m_stageState);
}

QString FileChange::porcelainCode() const
{
    const QString index = m_indexStatus.isEmpty() ? QStringLiteral(" ") : m_indexStatus.left(1);
    const QString worktree = m_worktreeStatus.isEmpty() ? QStringLiteral(" ") : m_worktreeStatus.left(1);

    return index + worktree;
}

bool FileChange::operator==(const FileChange& other) const
{
    return m_path == other.m_path
           && m_oldPath == other.m_oldPath
           && m_status == other.m_status
           && m_stageState == other.m_stageState;
}
