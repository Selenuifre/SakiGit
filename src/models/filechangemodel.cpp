#include "filechangemodel.h"

#include <Qt>

// ---- 静态路径规范化工具 ----
namespace {
QString normalizedPath(const QString& path)
{
    QString clean = path.trimmed();
    clean.replace(QLatin1Char('\\'), QLatin1Char('/'));
    return clean;
}
} // anonymous namespace

// ---- 构造 ----
FileChangeModel::FileChangeModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

// ---- QAbstractTableModel 接口 ----
int FileChangeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return static_cast<int>(m_fileChanges.size());
}

int FileChangeModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return ColumnCount;
}

QVariant FileChangeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !isValidRow(index.row())) {
        return QVariant();
    }

    const FileChange& fileChange = m_fileChanges.at(index.row());

    // ---- 按列处理的 DisplayRole ----
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case FilePathColumn:
            return fileChange.displayPath();
        case StatusColumn:
            return fileChange.statusText();
        case StagedColumn:
            return fileChange.stageStateText();
        default:
            return QVariant();
        }
    }

    // ---- 自定义角色 ----
    switch (role) {
    case Qt::EditRole:
    case FilePathRole:
        return fileChange.path();
    case Qt::ToolTipRole:
        return fileChange.displayPath();
    case FileChangeRole:
        return QVariant::fromValue(fileChange);
    case OldPathRole:
        return fileChange.oldPath();
    case DisplayPathRole:
        return fileChange.displayPath();
    case StatusRole:
        return static_cast<int>(fileChange.status());
    case StatusTextRole:
        return fileChange.statusText();
    case StageStateRole:
        return static_cast<int>(fileChange.stageState());
    case StageStateTextRole:
        return fileChange.stageStateText();
    case IndexStatusRole:
        return fileChange.indexStatus();
    case WorktreeStatusRole:
        return fileChange.worktreeStatus();
    case PorcelainCodeRole:
        return fileChange.porcelainCode();
    case StagedRole:
        return fileChange.isStaged();
    case UnstagedRole:
        return fileChange.isUnstaged();
    case PartiallyStagedRole:
        return fileChange.isPartiallyStaged();
    case ConflictRole:
        return fileChange.isConflict();
    case DeletedRole:
        return fileChange.isDeleted();
    case RenamedRole:
        return fileChange.isRenamed();
    case UntrackedRole:
        return fileChange.isUntracked();
    case HasOldPathRole:
        return fileChange.hasOldPath();
    case ValidRole:
        return fileChange.isValid();
    default:
        return QVariant();
    }
}

bool FileChangeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || !isValidRow(index.row())) {
        return false;
    }

    FileChange& fileChange = m_fileChanges[index.row()];
    bool changed = false;

    switch (role) {
    case Qt::EditRole:
    case FilePathRole: {
        const QString path = GitTypes::cleanPathSeparators(value.toString());
        if (fileChange.path() != path) {
            fileChange.setPath(path);
            changed = true;
        }
        break;
    }
    case OldPathRole: {
        const QString oldPath = GitTypes::cleanPathSeparators(value.toString());
        if (fileChange.oldPath() != oldPath) {
            fileChange.setOldPath(oldPath);
            changed = true;
        }
        break;
    }
    case StatusRole: {
        const auto status = static_cast<GitTypes::FileStatus>(value.toInt());
        if (fileChange.status() != status) {
            fileChange.setStatus(status);
            changed = true;
        }
        break;
    }
    case StageStateRole: {
        const auto stageState = static_cast<GitTypes::StageState>(value.toInt());
        if (fileChange.stageState() != stageState) {
            fileChange.setStageState(stageState);
            changed = true;
        }
        break;
    }
    case IndexStatusRole: {
        const QString indexStatus = value.toString();
        if (fileChange.indexStatus() != indexStatus.trimmed().left(1)) {
            fileChange.setIndexStatus(indexStatus);
            changed = true;
        }
        break;
    }
    case WorktreeStatusRole: {
        const QString worktreeStatus = value.toString();
        if (fileChange.worktreeStatus() != worktreeStatus.trimmed().left(1)) {
            fileChange.setWorktreeStatus(worktreeStatus);
            changed = true;
        }
        break;
    }
    case FileChangeRole: {
        if (!value.canConvert<FileChange>()) {
            return false;
        }
        fileChange = value.value<FileChange>();
        changed = true;
        break;
    }
    default:
        return false;
    }

    if (!changed) {
        return false;
    }

    emitFileChangeChanged(index.row());
    return true;
}

QVariant FileChangeModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
    case FilePathColumn:
        return QStringLiteral("File");
    case StatusColumn:
        return QStringLiteral("Status");
    case StagedColumn:
        return QStringLiteral("Staged");
    default:
        return QVariant();
    }
}

Qt::ItemFlags FileChangeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid() || !isValidRow(index.row())) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEnabled
           | Qt::ItemIsSelectable
           | Qt::ItemIsEditable;
}

QHash<int, QByteArray> FileChangeModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractTableModel::roleNames();
    roles.insert(FilePathRole, "filePath");
    roles.insert(StatusRole, "status");
    roles.insert(StagedRole, "staged");
    roles.insert(FileChangeRole, "fileChange");
    roles.insert(OldPathRole, "oldPath");
    roles.insert(DisplayPathRole, "displayPath");
    roles.insert(StatusTextRole, "statusText");
    roles.insert(StageStateRole, "stageState");
    roles.insert(StageStateTextRole, "stageStateText");
    roles.insert(IndexStatusRole, "indexStatus");
    roles.insert(WorktreeStatusRole, "worktreeStatus");
    roles.insert(PorcelainCodeRole, "porcelainCode");
    roles.insert(UnstagedRole, "unstaged");
    roles.insert(PartiallyStagedRole, "partiallyStaged");
    roles.insert(ConflictRole, "conflict");
    roles.insert(DeletedRole, "deleted");
    roles.insert(RenamedRole, "renamed");
    roles.insert(UntrackedRole, "untracked");
    roles.insert(HasOldPathRole, "hasOldPath");
    roles.insert(ValidRole, "valid");
    return roles;
}

// ---- 规范要求的方法 ----
void FileChangeModel::setChanges(const std::vector<FileChange>& changes)
{
    setFileChanges(changes);
}

FileChange FileChangeModel::changeAt(int row) const
{
    return fileChangeAt(row);
}

QString FileChangeModel::filePathAt(int row) const
{
    if (!isValidRow(row)) {
        return QString();
    }

    return m_fileChanges.at(row).path();
}

void FileChangeModel::clear()
{
    if (m_fileChanges.empty()) {
        return;
    }

    beginResetModel();
    m_fileChanges.clear();
    endResetModel();

    emit fileChangesChanged();
}

// ---- 扩展方法 ----
std::vector<FileChange> FileChangeModel::fileChanges() const
{
    return m_fileChanges;
}

void FileChangeModel::setFileChanges(const std::vector<FileChange>& fileChanges)
{
    beginResetModel();

    m_fileChanges.clear();

    for (const FileChange& fileChange : fileChanges) {
        if (!fileChange.isValid()) {
            continue;
        }

        const int existingRow = indexOfPath(fileChange.path());
        if (existingRow >= 0) {
            m_fileChanges[existingRow] = fileChange;
        } else {
            m_fileChanges.push_back(fileChange);
        }
    }

    endResetModel();
    emit fileChangesChanged();
}

bool FileChangeModel::isEmpty() const
{
    return m_fileChanges.empty();
}

int FileChangeModel::count() const
{
    return static_cast<int>(m_fileChanges.size());
}

FileChange FileChangeModel::fileChangeAt(int row) const
{
    if (!isValidRow(row)) {
        return FileChange();
    }

    return m_fileChanges.at(row);
}

FileChange FileChangeModel::fileChangeForPath(const QString& path) const
{
    const int row = indexOfPath(path);
    if (row < 0) {
        return FileChange();
    }

    return m_fileChanges.at(row);
}

QModelIndex FileChangeModel::indexForPath(const QString& path) const
{
    const int row = indexOfPath(path);
    if (row < 0) {
        return QModelIndex();
    }

    return index(row, 0);
}

int FileChangeModel::indexOfPath(const QString& path) const
{
    const QString targetPath = normalizedPath(path);
    if (targetPath.isEmpty()) {
        return -1;
    }

    for (int row = 0; row < static_cast<int>(m_fileChanges.size()); ++row) {
        if (normalizedPath(m_fileChanges.at(row).path()) == targetPath) {
            return row;
        }
    }

    return -1;
}

bool FileChangeModel::containsPath(const QString& path) const
{
    return indexOfPath(path) >= 0;
}

void FileChangeModel::addFileChange(const FileChange& fileChange)
{
    if (!fileChange.isValid()) {
        return;
    }

    const int existingRow = indexOfPath(fileChange.path());
    if (existingRow >= 0) {
        m_fileChanges[existingRow] = fileChange;
        emitFileChangeChanged(existingRow);
        return;
    }

    const int row = static_cast<int>(m_fileChanges.size());
    beginInsertRows(QModelIndex(), row, row);
    m_fileChanges.push_back(fileChange);
    endInsertRows();

    emit fileChangeAdded(fileChange);
    emit fileChangesChanged();
}

bool FileChangeModel::updateFileChange(const FileChange& fileChange)
{
    const int row = indexOfPath(fileChange.path());
    if (row < 0) {
        return false;
    }

    m_fileChanges[row] = fileChange;
    emitFileChangeChanged(row);
    return true;
}

bool FileChangeModel::upsertFileChange(const FileChange& fileChange)
{
    const int row = indexOfPath(fileChange.path());
    if (row >= 0) {
        m_fileChanges[row] = fileChange;
        emitFileChangeChanged(row);
        return true;
    }

    addFileChange(fileChange);
    return false;
}

bool FileChangeModel::removeFileChangeAt(int row)
{
    if (!isValidRow(row)) {
        return false;
    }

    const QString removedPath = m_fileChanges.at(row).path();

    beginRemoveRows(QModelIndex(), row, row);
    m_fileChanges.erase(m_fileChanges.begin() + row);
    endRemoveRows();

    emit fileChangeRemoved(removedPath);
    emit fileChangesChanged();
    return true;
}

bool FileChangeModel::removeFileChange(const QString& path)
{
    const int row = indexOfPath(path);
    if (row < 0) {
        return false;
    }

    return removeFileChangeAt(row);
}

std::vector<FileChange> FileChangeModel::stagedChanges() const
{
    std::vector<FileChange> changes;

    for (const FileChange& fileChange : m_fileChanges) {
        if (fileChange.isStaged() || fileChange.isPartiallyStaged()) {
            changes.push_back(fileChange);
        }
    }

    return changes;
}

std::vector<FileChange> FileChangeModel::unstagedChanges() const
{
    std::vector<FileChange> changes;

    for (const FileChange& fileChange : m_fileChanges) {
        if (fileChange.isUnstaged() || fileChange.isPartiallyStaged()) {
            changes.push_back(fileChange);
        }
    }

    return changes;
}

std::vector<FileChange> FileChangeModel::conflictedChanges() const
{
    std::vector<FileChange> changes;

    for (const FileChange& fileChange : m_fileChanges) {
        if (fileChange.isConflict()) {
            changes.push_back(fileChange);
        }
    }

    return changes;
}

int FileChangeModel::stagedCount() const
{
    int count = 0;
    for (const FileChange& fileChange : m_fileChanges) {
        if (fileChange.isStaged() || fileChange.isPartiallyStaged()) {
            ++count;
        }
    }
    return count;
}

int FileChangeModel::unstagedCount() const
{
    int count = 0;
    for (const FileChange& fileChange : m_fileChanges) {
        if (fileChange.isUnstaged() || fileChange.isPartiallyStaged()) {
            ++count;
        }
    }
    return count;
}

int FileChangeModel::conflictedCount() const
{
    int count = 0;
    for (const FileChange& fileChange : m_fileChanges) {
        if (fileChange.isConflict()) {
            ++count;
        }
    }
    return count;
}

// ---- 内部辅助 ----
bool FileChangeModel::isValidRow(int row) const
{
    return row >= 0 && row < static_cast<int>(m_fileChanges.size());
}

void FileChangeModel::emitFileChangeChanged(int row)
{
    if (!isValidRow(row)) {
        return;
    }

    const QModelIndex topLeft = index(row, 0);
    const QModelIndex bottomRight = index(row, ColumnCount - 1);
    emit dataChanged(topLeft, bottomRight);
    emit fileChangeUpdated(m_fileChanges.at(row));
    emit fileChangesChanged();
}
