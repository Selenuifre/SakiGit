#include "commithistorymodel.h"

#include <Qt>

// ---- 静态辅助函数 ----
QString CommitHistoryModel::normalizedHash(const QString& hash)
{
    return hash.trimmed().toLower();
}

bool CommitHistoryModel::hashMatches(const QString& commitHash, const QString& queryHash)
{
    const QString cleanCommitHash = normalizedHash(commitHash);
    const QString cleanQueryHash = normalizedHash(queryHash);

    if (cleanCommitHash.isEmpty() || cleanQueryHash.isEmpty()) {
        return false;
    }

    return cleanCommitHash == cleanQueryHash
           || cleanCommitHash.startsWith(cleanQueryHash)
           || cleanQueryHash.startsWith(cleanCommitHash);
}

QString CommitHistoryModel::normalizedFilePath(const QString& filePath)
{
    QString cleanPath = filePath.trimmed();
    cleanPath.replace(QLatin1Char('\\'), QLatin1Char('/'));
    return cleanPath;
}

// ---- 构造 ----
CommitHistoryModel::CommitHistoryModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

// ---- QAbstractTableModel 接口 ----
int CommitHistoryModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return static_cast<int>(m_commits.size());
}

int CommitHistoryModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return ColumnCount;
}

QVariant CommitHistoryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !isValidRow(index.row())) {
        return QVariant();
    }

    const Commit& commit = m_commits.at(index.row());

    // ---- 按列处理的 DisplayRole ----
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case HashColumn:
            return commit.shortHash();
        case MessageColumn:
            return commit.summary();
        case AuthorColumn:
            return commit.displayAuthor();
        case DateColumn:
            return commit.displayDate();
        default:
            return QVariant();
        }
    }

    // ---- 自定义角色 ----
    switch (role) {
    case Qt::EditRole:
    case SummaryRole:
        return commit.summary();
    case Qt::ToolTipRole:
        return commit.message();
    case CommitRole:
        return QVariant::fromValue(commit);
    case HashRole:
        return commit.hash();
    case ShortHashRole:
        return commit.shortHash();
    case MessageRole:
        return commit.message();
    case DisplayTitleRole:
        return commit.displayTitle();
    case BodyRole:
        return commit.body();
    case AuthorRole:
    case AuthorDisplayRole:
        return commit.displayAuthor();
    case AuthorNameRole:
        return commit.authorName();
    case AuthorEmailRole:
        return commit.authorEmail();
    case AuthorDateRole:
        return commit.authorDate();
    case AuthorDateTextRole:
        return commit.displayDate();
    case DateRole:
        return commit.displayDate();
    case CommitterNameRole:
        return commit.committerName();
    case CommitterEmailRole:
        return commit.committerEmail();
    case CommitterDateRole:
        return commit.committerDate();
    case ParentHashesRole:
        return commit.parentHashes();
    case ParentCountRole:
        return commit.parentHashes().size();
    case ChangedFilesRole:
        return commit.changedFiles();
    case ChangedFileCountRole:
        return commit.changedFiles().size();
    case MergeCommitRole:
        return commit.isMergeCommit();
    case HasBodyRole:
        return commit.hasBody();
    case ValidRole:
        return commit.isValid();
    default:
        return QVariant();
    }
}

bool CommitHistoryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || !isValidRow(index.row())) {
        return false;
    }

    Commit& commit = m_commits[index.row()];
    bool changed = false;

    switch (role) {
    case HashRole: {
        const QString hash = value.toString().trimmed();
        if (commit.hash() != hash) {
            commit.setHash(hash);
            changed = true;
        }
        break;
    }
    case Qt::EditRole:
    case SummaryRole: {
        const QString summary = value.toString().trimmed();
        if (commit.summary() != summary) {
            commit.setSummary(summary);
            changed = true;
        }
        break;
    }
    case BodyRole: {
        const QString body = value.toString().trimmed();
        if (commit.body() != body) {
            commit.setBody(body);
            changed = true;
        }
        break;
    }
    case MessageRole: {
        const QString message = value.toString();
        if (commit.message() != message.trimmed()) {
            commit.setMessage(message);
            changed = true;
        }
        break;
    }
    case AuthorNameRole: {
        const QString authorName = value.toString().trimmed();
        if (commit.authorName() != authorName) {
            commit.setAuthorName(authorName);
            changed = true;
        }
        break;
    }
    case AuthorEmailRole: {
        const QString authorEmail = value.toString().trimmed();
        if (commit.authorEmail() != authorEmail) {
            commit.setAuthorEmail(authorEmail);
            changed = true;
        }
        break;
    }
    case AuthorDateRole: {
        const QDateTime authorDate = value.toDateTime();
        if (commit.authorDate() != authorDate) {
            commit.setAuthorDate(authorDate);
            changed = true;
        }
        break;
    }
    case CommitterNameRole: {
        const QString committerName = value.toString().trimmed();
        if (commit.committerName() != committerName) {
            commit.setCommitterName(committerName);
            changed = true;
        }
        break;
    }
    case CommitterEmailRole: {
        const QString committerEmail = value.toString().trimmed();
        if (commit.committerEmail() != committerEmail) {
            commit.setCommitterEmail(committerEmail);
            changed = true;
        }
        break;
    }
    case CommitterDateRole: {
        const QDateTime committerDate = value.toDateTime();
        if (commit.committerDate() != committerDate) {
            commit.setCommitterDate(committerDate);
            changed = true;
        }
        break;
    }
    case ParentHashesRole: {
        const QStringList parentHashes = value.toStringList();
        if (commit.parentHashes() != parentHashes) {
            commit.setParentHashes(parentHashes);
            changed = true;
        }
        break;
    }
    case ChangedFilesRole: {
        const QStringList changedFiles = value.toStringList();
        if (commit.changedFiles() != changedFiles) {
            commit.setChangedFiles(changedFiles);
            changed = true;
        }
        break;
    }
    case CommitRole: {
        if (!value.canConvert<Commit>()) {
            return false;
        }
        commit = value.value<Commit>();
        changed = true;
        break;
    }
    default:
        return false;
    }

    if (!changed) {
        return false;
    }

    emitCommitChanged(index.row());
    return true;
}

QVariant CommitHistoryModel::headerData(int section, Qt::Orientation orientation,
                                         int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
    case HashColumn:
        return QStringLiteral("Hash");
    case MessageColumn:
        return QStringLiteral("Message");
    case AuthorColumn:
        return QStringLiteral("Author");
    case DateColumn:
        return QStringLiteral("Date");
    default:
        return QVariant();
    }
}

Qt::ItemFlags CommitHistoryModel::flags(const QModelIndex& index) const
{
    if (!index.isValid() || !isValidRow(index.row())) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEnabled
           | Qt::ItemIsSelectable
           | Qt::ItemIsEditable;
}

QHash<int, QByteArray> CommitHistoryModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractTableModel::roleNames();
    roles.insert(HashRole, "hash");
    roles.insert(ShortHashRole, "shortHash");
    roles.insert(MessageRole, "message");
    roles.insert(AuthorRole, "author");
    roles.insert(DateRole, "date");
    roles.insert(CommitRole, "commit");
    roles.insert(SummaryRole, "summary");
    roles.insert(BodyRole, "body");
    roles.insert(DisplayTitleRole, "displayTitle");
    roles.insert(AuthorNameRole, "authorName");
    roles.insert(AuthorEmailRole, "authorEmail");
    roles.insert(AuthorDateRole, "authorDate");
    roles.insert(AuthorDisplayRole, "authorDisplay");
    roles.insert(AuthorDateTextRole, "authorDateText");
    roles.insert(CommitterNameRole, "committerName");
    roles.insert(CommitterEmailRole, "committerEmail");
    roles.insert(CommitterDateRole, "committerDate");
    roles.insert(ParentHashesRole, "parentHashes");
    roles.insert(ParentCountRole, "parentCount");
    roles.insert(ChangedFilesRole, "changedFiles");
    roles.insert(ChangedFileCountRole, "changedFileCount");
    roles.insert(MergeCommitRole, "mergeCommit");
    roles.insert(HasBodyRole, "hasBody");
    roles.insert(ValidRole, "valid");
    return roles;
}

// ---- 规范要求的方法 ----
void CommitHistoryModel::setCommits(const std::vector<Commit>& commits)
{
    beginResetModel();

    m_commits.clear();

    for (const Commit& commit : commits) {
        if (!commit.isValid()) {
            continue;
        }

        const int existingRow = indexOfHash(commit.hash());
        if (existingRow >= 0) {
            m_commits[existingRow] = commit;
        } else {
            m_commits.push_back(commit);
        }
    }

    endResetModel();
    emit commitsChanged();
}

void CommitHistoryModel::appendCommits(const std::vector<Commit>& commits)
{
    if (commits.empty()) {
        return;
    }

    const int firstRow = static_cast<int>(m_commits.size());
    const int lastRow = firstRow + static_cast<int>(commits.size()) - 1;

    beginInsertRows(QModelIndex(), firstRow, lastRow);

    for (const Commit& commit : commits) {
        if (commit.isValid()) {
            m_commits.push_back(commit);
        }
    }

    endInsertRows();
    emit commitsChanged();
}

Commit CommitHistoryModel::commitAt(int row) const
{
    if (!isValidRow(row)) {
        return Commit();
    }

    return m_commits.at(row);
}

void CommitHistoryModel::clear()
{
    if (m_commits.empty()) {
        return;
    }

    beginResetModel();
    m_commits.clear();
    endResetModel();

    emit commitsChanged();
}

// ---- 扩展方法 ----
std::vector<Commit> CommitHistoryModel::commits() const
{
    return m_commits;
}

bool CommitHistoryModel::isEmpty() const
{
    return m_commits.empty();
}

int CommitHistoryModel::count() const
{
    return static_cast<int>(m_commits.size());
}

Commit CommitHistoryModel::commitForHash(const QString& hash) const
{
    const int row = indexOfHash(hash);
    if (row < 0) {
        return Commit();
    }

    return m_commits.at(row);
}

QModelIndex CommitHistoryModel::indexForHash(const QString& hash) const
{
    const int row = indexOfHash(hash);
    if (row < 0) {
        return QModelIndex();
    }

    return index(row, 0);
}

int CommitHistoryModel::indexOfHash(const QString& hash) const
{
    const QString targetHash = normalizedHash(hash);
    if (targetHash.isEmpty()) {
        return -1;
    }

    for (int row = 0; row < static_cast<int>(m_commits.size()); ++row) {
        if (hashMatches(m_commits.at(row).hash(), targetHash)) {
            return row;
        }
    }

    return -1;
}

bool CommitHistoryModel::containsHash(const QString& hash) const
{
    return indexOfHash(hash) >= 0;
}

void CommitHistoryModel::addCommit(const Commit& commit)
{
    if (!commit.isValid()) {
        return;
    }

    const int existingRow = indexOfHash(commit.hash());
    if (existingRow >= 0) {
        m_commits[existingRow] = commit;
        emitCommitChanged(existingRow);
        return;
    }

    const int row = static_cast<int>(m_commits.size());
    beginInsertRows(QModelIndex(), row, row);
    m_commits.push_back(commit);
    endInsertRows();

    emit commitAdded(commit);
    emit commitsChanged();
}

bool CommitHistoryModel::updateCommit(const Commit& commit)
{
    const int row = indexOfHash(commit.hash());
    if (row < 0) {
        return false;
    }

    m_commits[row] = commit;
    emitCommitChanged(row);
    return true;
}

bool CommitHistoryModel::upsertCommit(const Commit& commit)
{
    const int row = indexOfHash(commit.hash());
    if (row >= 0) {
        m_commits[row] = commit;
        emitCommitChanged(row);
        return true;
    }

    addCommit(commit);
    return false;
}

bool CommitHistoryModel::removeCommitAt(int row)
{
    if (!isValidRow(row)) {
        return false;
    }

    const QString removedHash = m_commits.at(row).hash();

    beginRemoveRows(QModelIndex(), row, row);
    m_commits.erase(m_commits.begin() + row);
    endRemoveRows();

    emit commitRemoved(removedHash);
    emit commitsChanged();
    return true;
}

bool CommitHistoryModel::removeCommit(const QString& hash)
{
    const int row = indexOfHash(hash);
    if (row < 0) {
        return false;
    }

    return removeCommitAt(row);
}

std::vector<Commit> CommitHistoryModel::mergeCommits() const
{
    std::vector<Commit> result;

    for (const Commit& commit : m_commits) {
        if (commit.isMergeCommit()) {
            result.push_back(commit);
        }
    }

    return result;
}

std::vector<Commit> CommitHistoryModel::commitsTouchingFile(const QString& filePath) const
{
    std::vector<Commit> result;
    const QString targetPath = normalizedFilePath(filePath);

    if (targetPath.isEmpty()) {
        return result;
    }

    for (const Commit& commit : m_commits) {
        const QStringList changedFiles = commit.changedFiles();
        for (const QString& changedFile : changedFiles) {
            if (normalizedFilePath(changedFile) == targetPath) {
                result.push_back(commit);
                break;
            }
        }
    }

    return result;
}

int CommitHistoryModel::mergeCommitCount() const
{
    int count = 0;
    for (const Commit& commit : m_commits) {
        if (commit.isMergeCommit()) {
            ++count;
        }
    }
    return count;
}

Commit CommitHistoryModel::latestCommit() const
{
    if (m_commits.empty()) {
        return Commit();
    }

    return m_commits.front();
}

// ---- 内部辅助 ----
bool CommitHistoryModel::isValidRow(int row) const
{
    return row >= 0 && row < static_cast<int>(m_commits.size());
}

void CommitHistoryModel::emitCommitChanged(int row)
{
    if (!isValidRow(row)) {
        return;
    }

    const QModelIndex topLeft = index(row, 0);
    const QModelIndex bottomRight = index(row, ColumnCount - 1);
    emit dataChanged(topLeft, bottomRight);
    emit commitUpdated(m_commits.at(row));
    emit commitsChanged();
}
