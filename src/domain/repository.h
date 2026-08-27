#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <QDateTime>
#include <QMetaType>
#include <QString>

#include "gittypes.h"
#include "Operators.h"

class Repository : public EqualityOperators<Repository>
{
public:
    Repository();
    explicit Repository(const QString& localPath);
    Repository(const QString& localPath, const QString& name);

    QString id() const;
    void setId(const QString& id);

    QString name() const;
    void setName(const QString& name);

    QString localPath() const;
    void setLocalPath(const QString& localPath);

    QString currentBranch() const;
    void setCurrentBranch(const QString& currentBranch);

    QString defaultBranch() const;
    void setDefaultBranch(const QString& defaultBranch);

    QString remoteName() const;
    void setRemoteName(const QString& remoteName);

    QString remoteUrl() const;
    void setRemoteUrl(const QString& remoteUrl);

    GitTypes::RemoteProvider provider() const;
    void setProvider(GitTypes::RemoteProvider provider);

    GitTypes::RepositoryState state() const;
    void setState(GitTypes::RepositoryState state);

    QDateTime lastOpenedAt() const;
    void setLastOpenedAt(const QDateTime& lastOpenedAt);

    bool isValid() const;
    bool hasRemote() const;
    bool isMissing() const;
    bool isGitHubRepository() const;

    QString displayName() const;

    static QString resolveAbsolutePath(const QString& path);
    static QString nameFromPath(const QString& path);

    bool operator==(const Repository& other) const;

private:
    QString m_id;
    QString m_name;
    QString m_localPath;

    QString m_currentBranch;
    QString m_defaultBranch;

    QString m_remoteName;
    QString m_remoteUrl;

    GitTypes::RemoteProvider m_provider;
    GitTypes::RepositoryState m_state;

    QDateTime m_lastOpenedAt;
};

Q_DECLARE_METATYPE(Repository)

#endif // REPOSITORY_H
