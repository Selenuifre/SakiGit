#ifndef REMOTELISTMODEL_H
#define REMOTELISTMODEL_H

#include "domain/Remote.h"

#include <QAbstractListModel>
#include <QHash>
#include <QModelIndex>
#include <QVariant>
#include <vector>

class RemoteListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        NameRole = Qt::UserRole + 1,
        UrlRole,
        PushUrlRole
    };

    explicit RemoteListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setRemotes(const std::vector<Remote>& remotes);
    Remote remoteAt(int row) const;
    QString remoteNameAt(int row) const;
    void clear();

private:
    std::vector<Remote> remotes_;
};

#endif // REMOTELISTMODEL_H
