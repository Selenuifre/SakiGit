#ifndef PULLREQUESTMODEL_H
#define PULLREQUESTMODEL_H

#include "domain/PullRequest.h"

#include <QAbstractListModel>
#include <QHash>
#include <QModelIndex>
#include <QVariant>
#include <vector>

class PullRequestModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        NumberRole = Qt::UserRole + 1,
        TitleRole,
        AuthorRole,
        StateRole,
        HeadRole,
        BaseRole
    };

    explicit PullRequestModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setPullRequests(const std::vector<PullRequest>& prs);
    PullRequest pullRequestAt(int row) const;
    int pullRequestNumberAt(int row) const;
    void clear();

private:
    std::vector<PullRequest> m_prs;
};

#endif // PULLREQUESTMODEL_H
