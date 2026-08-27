#include "TerminalOutputModel.h"

TerminalOutputModel::TerminalOutputModel(QObject* parent)
    : BaseListModel(parent)
{
}

QHash<int, QByteArray> TerminalOutputModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles.insert(DisplayLineRole, "displayLine");
    roles.insert(OutputRole, "output");
    roles.insert(ExitCodeRole, "exitCode");
    roles.insert(IsUserInputRole, "isUserInput");
    roles.insert(TextColorRole, "textColor");
    return roles;
}

QVariant TerminalOutputModel::dataForRole(const CommandLogEntry& entry, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
    case DisplayLineRole:
        return entry.displayLine();
    case OutputRole:
        return entry.output;
    case ExitCodeRole:
        return entry.exitCode;
    case IsUserInputRole:
        return entry.isUserInput;
    case TextColorRole:
        if (entry.exitCode != 0)
            return QColor(0xE9, 0x45, 0x60); // red for errors
        if (entry.isUserInput)
            return QColor(0x60, 0xA5, 0xFA); // blue for user input
        return QColor(0x9A, 0x99, 0xAA);     // gray for GUI-triggered
    default:
        return {};
    }
}

void TerminalOutputModel::appendEntry(const CommandLogEntry& entry)
{
    const int row = m_items.size();
    beginInsertRows(QModelIndex(), row, row);
    m_items.push_back(entry);
    endInsertRows();
    emit entryAppended();
}

CommandLogEntry TerminalOutputModel::entryAt(int row) const
{
    return itemAt(row);
}
