#include "DomainUtils.h"
#include "repository.h"

#include <QDir>
#include <QFileInfo>

Repository::Repository()
    : m_provider(GitTypes::RemoteProvider::Unknown),
    m_state(GitTypes::RepositoryState::Unknown)
{
}

Repository::Repository(const QString& localPath)
    : Repository()
{
    setLocalPath(localPath);
    setName(nameFromPath(localPath));
}

Repository::Repository(const QString& localPath, const QString& name)
    : Repository()
{
    setLocalPath(localPath);
    setName(name);
}

QString Repository::id() const
{
    return m_id;
}

void Repository::setId(const QString& id)
{
    assignTrimmed(m_id, id);
}

QString Repository::name() const
{
    return m_name;
}

void Repository::setName(const QString& name)
{
    assignTrimmed(m_name, name);
}

QString Repository::localPath() const
{
    return m_localPath;
}

void Repository::setLocalPath(const QString& localPath)
{
    m_localPath = resolveAbsolutePath(localPath);

    if (m_id.isEmpty()) {
        m_id = m_localPath;
    }

    if (m_name.isEmpty()) {
        m_name = nameFromPath(m_localPath);
    }
}

QString Repository::currentBranch() const
{
    return m_currentBranch;
}

void Repository::setCurrentBranch(const QString& currentBranch)
{
    assignTrimmed(m_currentBranch, currentBranch);
}

QString Repository::defaultBranch() const
{
    return m_defaultBranch;
}

void Repository::setDefaultBranch(const QString& defaultBranch)
{
    assignTrimmed(m_defaultBranch, defaultBranch);
}

QString Repository::remoteName() const
{
    return m_remoteName;
}

void Repository::setRemoteName(const QString& remoteName)
{
    assignTrimmed(m_remoteName, remoteName);
}

QString Repository::remoteUrl() const
{
    return m_remoteUrl;
}

void Repository::setRemoteUrl(const QString& remoteUrl)
{
    assignTrimmed(m_remoteUrl, remoteUrl);
    m_provider = GitTypes::remoteProviderFromUrl(m_remoteUrl);
}

GitTypes::RemoteProvider Repository::provider() const
{
    return m_provider;
}

void Repository::setProvider(GitTypes::RemoteProvider provider)
{
    m_provider = provider;
}

GitTypes::RepositoryState Repository::state() const
{
    return m_state;
}

void Repository::setState(GitTypes::RepositoryState state)
{
    m_state = state;
}

QDateTime Repository::lastOpenedAt() const
{
    return m_lastOpenedAt;
}

void Repository::setLastOpenedAt(const QDateTime& lastOpenedAt)
{
    m_lastOpenedAt = lastOpenedAt;
}

bool Repository::isValid() const
{
    return !m_localPath.isEmpty() && m_state == GitTypes::RepositoryState::Ready;
}

bool Repository::hasRemote() const
{
    return !m_remoteUrl.isEmpty();
}

bool Repository::isMissing() const
{
    return m_state == GitTypes::RepositoryState::Missing;
}

bool Repository::isGitHubRepository() const
{
    return m_provider == GitTypes::RemoteProvider::GitHub;
}

QString Repository::displayName() const
{
    if (!m_name.isEmpty()) {
        return m_name;
    }

    return nameFromPath(m_localPath);
}

QString Repository::resolveAbsolutePath(const QString& path)
{
    if (path.trimmed().isEmpty()) {
        return QString();
    }

    QDir dir(path);
    return QDir::cleanPath(dir.absolutePath());
}

QString Repository::nameFromPath(const QString& path)
{
    if (path.trimmed().isEmpty()) {
        return QString();
    }

    QFileInfo info(resolveAbsolutePath(path));
    return info.fileName();
}

bool Repository::operator==(const Repository& other) const
{
    return m_id == other.m_id;
}
