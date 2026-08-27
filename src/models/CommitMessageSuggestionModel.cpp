#include "CommitMessageSuggestionModel.h"

CommitMessageSuggestionModel::CommitMessageSuggestionModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int CommitMessageSuggestionModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_suggestions.size());
}

QVariant CommitMessageSuggestionModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_suggestions.size()) {
        return {};
    }

    const CommitMessageSuggestion& suggestion = m_suggestions.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
    case FullMessageRole:
        return suggestion.fullMessage();
    case TypeRole:
        return suggestion.type();
    case ScopeRole:
        return suggestion.scope();
    case SubjectRole:
        return suggestion.subject();
    case BodyRole:
        return suggestion.body();
    case RawResponseRole:
        return suggestion.rawResponse();
    case IsValidRole:
        return suggestion.isValid();
    default:
        return {};
    }
}

QHash<int, QByteArray> CommitMessageSuggestionModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[FullMessageRole]  = "fullMessage";
    names[TypeRole]         = "type";
    names[ScopeRole]        = "scope";
    names[SubjectRole]      = "subject";
    names[BodyRole]         = "body";
    names[RawResponseRole]  = "rawResponse";
    names[IsValidRole]      = "isValid";
    return names;
}

void CommitMessageSuggestionModel::setSuggestions(
    const QVector<CommitMessageSuggestion>& suggestions)
{
    beginResetModel();
    m_suggestions = suggestions;
    endResetModel();
}

void CommitMessageSuggestionModel::addSuggestion(
    const CommitMessageSuggestion& suggestion)
{
    const int row = static_cast<int>(m_suggestions.size());
    beginInsertRows(QModelIndex(), row, row);
    m_suggestions.append(suggestion);
    endInsertRows();
}

CommitMessageSuggestion CommitMessageSuggestionModel::suggestionAt(int row) const
{
    if (row < 0 || row >= m_suggestions.size()) {
        return CommitMessageSuggestion();
    }
    return m_suggestions.at(row);
}

CommitMessageSuggestion CommitMessageSuggestionModel::bestSuggestion() const
{
    for (const auto& suggestion : m_suggestions) {
        if (suggestion.isValid()) {
            return suggestion;
        }
    }

    if (!m_suggestions.isEmpty()) {
        return m_suggestions.first();
    }

    return CommitMessageSuggestion();
}

int CommitMessageSuggestionModel::count() const
{
    return static_cast<int>(m_suggestions.size());
}

bool CommitMessageSuggestionModel::isEmpty() const
{
    return m_suggestions.isEmpty();
}

void CommitMessageSuggestionModel::clear()
{
    if (m_suggestions.isEmpty()) {
        return;
    }
    beginResetModel();
    m_suggestions.clear();
    endResetModel();
}
