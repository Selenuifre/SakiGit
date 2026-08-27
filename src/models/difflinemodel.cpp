#include "difflinemodel.h"

DiffLineModel::DiffLineModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int DiffLineModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return static_cast<int>(m_lines.size());
}

int DiffLineModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return ColumnCount;
}

QVariant DiffLineModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const int row = index.row();
    if (row < 0 || row >= static_cast<int>(m_lines.size())) {
        return QVariant();
    }

    const DiffLine& line = m_lines.at(row);

    if (role == LineTypeRole) {
        return static_cast<int>(line.type());
    }

    if (role == OldLineNumberRole) {
        if (line.oldLineNumber() >= 0) {
            return line.oldLineNumber();
        }
        return QVariant();
    }

    if (role == NewLineNumberRole) {
        if (line.newLineNumber() >= 0) {
            return line.newLineNumber();
        }
        return QVariant();
    }

    if (role == ContentRole) {
        return line.text();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case OldLineNumberColumn:
            if (line.oldLineNumber() >= 0) {
                return line.oldLineNumber();
            }
            return QVariant();
        case NewLineNumberColumn:
            if (line.newLineNumber() >= 0) {
                return line.newLineNumber();
            }
            return QVariant();
        case ContentColumn:
            return line.text();
        default:
            break;
        }
    }

    // 前景色角色：新增行绿色，删除行红色
    if (role == Qt::ForegroundRole) {
        if (line.isAdded()) {
            return QColor(0x1A, 0x7F, 0x37); // GitHub green
        }
        if (line.isRemoved()) {
            return QColor(0xCF, 0x22, 0x2E); // GitHub red
        }
    }

    // 背景色角色
    if (role == Qt::BackgroundRole) {
        if (line.isAdded()) {
            return QColor(0xDA, 0xFB, 0xE1); // light green
        }
        if (line.isRemoved()) {
            return QColor(0xFF, 0xDB, 0xDE); // light red
        }
    }

    return QVariant();
}

QVariant DiffLineModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
    case OldLineNumberColumn:
        return QStringLiteral("Old");
    case NewLineNumberColumn:
        return QStringLiteral("New");
    case ContentColumn:
        return QStringLiteral("Content");
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> DiffLineModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractTableModel::roleNames();
    roles.insert(OldLineNumberRole, "oldLineNumber");
    roles.insert(NewLineNumberRole, "newLineNumber");
    roles.insert(ContentRole, "content");
    roles.insert(LineTypeRole, "lineType");
    return roles;
}

void DiffLineModel::setDiff(const Diff& diff)
{
    std::vector<DiffLine> allLines;

    for (const FileDiff& fileDiff : diff.files()) {
        for (const DiffHunk& hunk : fileDiff.hunks()) {
            // 为每个 hunk 先加入一条 hunk 头部行
            DiffLine headerLine(GitTypes::DiffLineType::HunkHeader, hunk.header());
            allLines.push_back(headerLine);

            for (const DiffLine& line : hunk.lines()) {
                allLines.push_back(line);
            }
        }
    }

    setLines(allLines);
}

void DiffLineModel::setLines(const std::vector<DiffLine>& lines)
{
    beginResetModel();
    m_lines = lines;
    endResetModel();
}

DiffLine DiffLineModel::lineAt(int row) const
{
    if (row < 0 || row >= static_cast<int>(m_lines.size())) {
        return DiffLine();
    }

    return m_lines.at(row);
}

void DiffLineModel::clear()
{
    if (m_lines.empty()) {
        return;
    }

    beginResetModel();
    m_lines.clear();
    endResetModel();
}
