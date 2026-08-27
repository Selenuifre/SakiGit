#include "RemoteListModel.h"

RemoteListModel::RemoteListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int RemoteListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return static_cast<int>(remotes_.size());
}

QVariant RemoteListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(remotes_.size())) {
        return QVariant();
    }

    const Remote& remote = remotes_.at(row);

    switch (role) {
    case Qt::DisplayRole:
    case NameRole:
        return remote.displayName();
    case Qt::ToolTipRole:
        return remote.displayUrl();
    case UrlRole:
        return remote.url();
    case PushUrlRole:
        return remote.pushUrl().isEmpty() ? remote.url() : remote.pushUrl();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> RemoteListModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles.insert(NameRole, "name");
    roles.insert(UrlRole, "url");
    roles.insert(PushUrlRole, "pushUrl");
    return roles;
}

void RemoteListModel::setRemotes(const std::vector<Remote>& remotes)
{
    beginResetModel();
    remotes_ = remotes;
    endResetModel();
}

Remote RemoteListModel::remoteAt(int row) const
{
    if (row < 0 || row >= static_cast<int>(remotes_.size())) {
        return Remote();
    }

    return remotes_.at(row);
}

QString RemoteListModel::remoteNameAt(int row) const
{
    return remoteAt(row).name();
}

void RemoteListModel::clear()
{
    if (remotes_.empty()) {
        return;
    }

    beginResetModel();
    remotes_.clear();
    endResetModel();
}
