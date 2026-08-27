#include "PullRequestModel.h"

PullRequestModel::PullRequestModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int PullRequestModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return static_cast<int>(m_prs.size());
}

QVariant PullRequestModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(m_prs.size())) {
        return QVariant();
    }

    const PullRequest& pr = m_prs.at(row);

    switch (role) {
    case Qt::DisplayRole:
    case TitleRole:
        return pr.displayTitle();
    case NumberRole:
        return pr.number();
    case AuthorRole:
        return pr.author();
    case StateRole:
        return pr.displayState();
    case HeadRole:
        return pr.head();
    case BaseRole:
        return pr.base();
    case Qt::ToolTipRole:
        return QStringLiteral("#%1 by %2 (%3)").arg(pr.number()).arg(pr.author(), pr.displayState());
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> PullRequestModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles.insert(NumberRole, "number");
    roles.insert(TitleRole, "title");
    roles.insert(AuthorRole, "author");
    roles.insert(StateRole, "state");
    roles.insert(HeadRole, "head");
    roles.insert(BaseRole, "base");
    return roles;
}

void PullRequestModel::setPullRequests(const std::vector<PullRequest>& prs)
{
    beginResetModel();
    m_prs = prs;
    endResetModel();
}

PullRequest PullRequestModel::pullRequestAt(int row) const
{
    if (row < 0 || row >= static_cast<int>(m_prs.size())) {
        return PullRequest();
    }

    return m_prs.at(row);
}

int PullRequestModel::pullRequestNumberAt(int row) const
{
    return pullRequestAt(row).number();
}

void PullRequestModel::clear()
{
    if (m_prs.empty()) {
        return;
    }

    beginResetModel();
    m_prs.clear();
    endResetModel();
}
