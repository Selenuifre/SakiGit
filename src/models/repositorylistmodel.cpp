#include "repositorylistmodel.h"

#include "domain/gittypes.h"

#include <QDateTime>
#include <Qt>

// ---- 静态路径规范化 ----
QString RepositoryListModel::normalizedPath(const QString& localPath)
{
    return Repository::resolveAbsolutePath(localPath);
}

// ---- 构造 ----
RepositoryListModel::RepositoryListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

// ---- QAbstractListModel 接口 ----
int RepositoryListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return static_cast<int>(m_repositories.size());
}

QVariant RepositoryListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !isValidRow(index.row())) {
        return QVariant();
    }

    const Repository& repository = m_repositories.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
    case NameRole:
    case DisplayNameRole:
        return repository.displayName();
    case Qt::EditRole:
        return repository.name();
    case Qt::ToolTipRole:
        return repository.localPath();
    case RepositoryRole:
        return QVariant::fromValue(repository);
    case IdRole:
        return repository.id();
    case PathRole:
        return repository.localPath();
    case CurrentBranchRole:
        return repository.currentBranch();
    case DefaultBranchRole:
        return repository.defaultBranch();
    case RemoteNameRole:
        return repository.remoteName();
    case RemoteUrlRole:
        return repository.remoteUrl();
    case ProviderRole:
        return static_cast<int>(repository.provider());
    case ProviderTextRole:
        return GitTypes::toString(repository.provider());
    case StateRole:
        return static_cast<int>(repository.state());
    case StateTextRole:
        return GitTypes::toString(repository.state());
    case LastOpenedAtRole:
        return repository.lastOpenedAt();
    case ValidRole:
        return repository.isValid();
    case MissingRole:
        return repository.isMissing();
    case HasRemoteRole:
        return repository.hasRemote();
    default:
        return QVariant();
    }
}

bool RepositoryListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || !isValidRow(index.row())) {
        return false;
    }

    Repository& repository = m_repositories[index.row()];
    bool changed = false;

    switch (role) {
    case Qt::EditRole:
    case NameRole: {
        const QString name = value.toString().trimmed();
        if (repository.name() != name) {
            repository.setName(name);
            changed = true;
        }
        break;
    }
    case CurrentBranchRole: {
        const QString currentBranch = value.toString().trimmed();
        if (repository.currentBranch() != currentBranch) {
            repository.setCurrentBranch(currentBranch);
            changed = true;
        }
        break;
    }
    case DefaultBranchRole: {
        const QString defaultBranch = value.toString().trimmed();
        if (repository.defaultBranch() != defaultBranch) {
            repository.setDefaultBranch(defaultBranch);
            changed = true;
        }
        break;
    }
    case RemoteNameRole: {
        const QString remoteName = value.toString().trimmed();
        if (repository.remoteName() != remoteName) {
            repository.setRemoteName(remoteName);
            changed = true;
        }
        break;
    }
    case RemoteUrlRole: {
        const QString remoteUrl = value.toString().trimmed();
        if (repository.remoteUrl() != remoteUrl) {
            repository.setRemoteUrl(remoteUrl);
            changed = true;
        }
        break;
    }
    case ProviderRole: {
        const auto provider = static_cast<GitTypes::RemoteProvider>(value.toInt());
        if (repository.provider() != provider) {
            repository.setProvider(provider);
            changed = true;
        }
        break;
    }
    case StateRole: {
        const auto state = static_cast<GitTypes::RepositoryState>(value.toInt());
        if (repository.state() != state) {
            repository.setState(state);
            changed = true;
        }
        break;
    }
    case LastOpenedAtRole: {
        const QDateTime lastOpenedAt = value.toDateTime();
        if (repository.lastOpenedAt() != lastOpenedAt) {
            repository.setLastOpenedAt(lastOpenedAt);
            changed = true;
        }
        break;
    }
    case RepositoryRole: {
        if (!value.canConvert<Repository>()) {
            return false;
        }
        repository = value.value<Repository>();
        changed = true;
        break;
    }
    default:
        return false;
    }

    if (!changed) {
        return false;
    }

    emitRepositoryChanged(index.row());
    return true;
}

Qt::ItemFlags RepositoryListModel::flags(const QModelIndex& index) const
{
    if (!index.isValid() || !isValidRow(index.row())) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEnabled
           | Qt::ItemIsSelectable;
}

QHash<int, QByteArray> RepositoryListModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles.insert(NameRole, "name");
    roles.insert(PathRole, "path");
    roles.insert(CurrentBranchRole, "currentBranch");
    roles.insert(RepositoryRole, "repository");
    roles.insert(IdRole, "id");
    roles.insert(DisplayNameRole, "displayName");
    roles.insert(DefaultBranchRole, "defaultBranch");
    roles.insert(RemoteNameRole, "remoteName");
    roles.insert(RemoteUrlRole, "remoteUrl");
    roles.insert(ProviderRole, "provider");
    roles.insert(ProviderTextRole, "providerText");
    roles.insert(StateRole, "state");
    roles.insert(StateTextRole, "stateText");
    roles.insert(LastOpenedAtRole, "lastOpenedAt");
    roles.insert(ValidRole, "valid");
    roles.insert(MissingRole, "missing");
    roles.insert(HasRemoteRole, "hasRemote");
    return roles;
}

// ---- 规范要求的方法 ----
void RepositoryListModel::setRepositories(const QList<Repository>& repositories)
{
    beginResetModel();

    m_repositories.clear();

    for (const Repository& repository : repositories) {
        if (repository.localPath().isEmpty()) {
            continue;
        }

        const int existingRow = indexOfPath(repository.localPath());
        if (existingRow >= 0) {
            m_repositories[existingRow] = repository;
        } else {
            m_repositories.append(repository);
        }
    }

    endResetModel();
    emit repositoriesChanged();
}

void RepositoryListModel::addRepository(const Repository& repository)
{
    if (repository.localPath().isEmpty()) {
        return;
    }

    const int existingRow = indexOfPath(repository.localPath());
    if (existingRow >= 0) {
        m_repositories[existingRow] = repository;
        emitRepositoryChanged(existingRow);
        return;
    }

    const int row = static_cast<int>(m_repositories.size());
    beginInsertRows(QModelIndex(), row, row);
    m_repositories.append(repository);
    endInsertRows();

    emit repositoryAdded(repository);
    emit repositoriesChanged();
}

void RepositoryListModel::removeRepository(const QString& path)
{
    const int row = indexOfPath(path);
    if (row < 0) {
        return;
    }

    removeRepositoryAt(row);
}

Repository RepositoryListModel::repositoryAt(int row) const
{
    if (!isValidRow(row)) {
        return Repository();
    }

    return m_repositories.at(row);
}

void RepositoryListModel::clear()
{
    if (m_repositories.isEmpty()) {
        return;
    }

    beginResetModel();
    m_repositories.clear();
    endResetModel();

    emit repositoriesChanged();
}

// ---- 扩展方法 ----
QList<Repository> RepositoryListModel::repositories() const
{
    return m_repositories;
}

bool RepositoryListModel::isEmpty() const
{
    return m_repositories.isEmpty();
}

int RepositoryListModel::count() const
{
    return static_cast<int>(m_repositories.size());
}

Repository RepositoryListModel::repositoryForPath(const QString& localPath) const
{
    const int row = indexOfPath(localPath);
    if (row < 0) {
        return Repository();
    }

    return m_repositories.at(row);
}

QModelIndex RepositoryListModel::indexForPath(const QString& localPath) const
{
    const int row = indexOfPath(localPath);
    if (row < 0) {
        return QModelIndex();
    }

    return index(row, 0);
}

int RepositoryListModel::indexOfPath(const QString& localPath) const
{
    const QString targetPath = normalizedPath(localPath);
    if (targetPath.isEmpty()) {
        return -1;
    }

    for (int row = 0; row < static_cast<int>(m_repositories.size()); ++row) {
        if (normalizedPath(m_repositories.at(row).localPath()) == targetPath) {
            return row;
        }
    }

    return -1;
}

bool RepositoryListModel::containsPath(const QString& localPath) const
{
    return indexOfPath(localPath) >= 0;
}

bool RepositoryListModel::updateRepository(const Repository& repository)
{
    const int row = indexOfPath(repository.localPath());
    if (row < 0) {
        return false;
    }

    m_repositories[row] = repository;
    emitRepositoryChanged(row);
    return true;
}

bool RepositoryListModel::upsertRepository(const Repository& repository)
{
    const int row = indexOfPath(repository.localPath());
    if (row >= 0) {
        m_repositories[row] = repository;
        emitRepositoryChanged(row);
        return true;
    }

    addRepository(repository);
    return false;
}

bool RepositoryListModel::removeRepositoryAt(int row)
{
    if (!isValidRow(row)) {
        return false;
    }

    const QString removedPath = m_repositories.at(row).localPath();

    beginRemoveRows(QModelIndex(), row, row);
    m_repositories.removeAt(row);
    endRemoveRows();

    emit repositoryRemoved(removedPath);
    emit repositoriesChanged();
    return true;
}

// ---- 内部辅助 ----
bool RepositoryListModel::isValidRow(int row) const
{
    return row >= 0 && row < static_cast<int>(m_repositories.size());
}

void RepositoryListModel::emitRepositoryChanged(int row)
{
    if (!isValidRow(row)) {
        return;
    }

    const QModelIndex changedIndex = index(row, 0);
    emit dataChanged(changedIndex, changedIndex);
    emit repositoryUpdated(m_repositories.at(row));
    emit repositoriesChanged();
}
