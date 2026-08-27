#include "ReviewFindingModel.h"

ReviewFindingModel::ReviewFindingModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ReviewFindingModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_findings.size());
}

QVariant ReviewFindingModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_findings.size()) {
        return {};
    }

    const ReviewFinding& finding = m_findings.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
    case TitleRole:
        return finding.title();
    case FilePathRole:
        return finding.filePath();
    case LineNumberRole:
        return finding.lineNumber();
    case SeverityRole:
        return finding.severity();
    case CategoryRole:
        return finding.category();
    case MessageRole:
        return finding.message();
    case SuggestionRole:
        return finding.suggestion();
    case CodeSnippetRole:
        return finding.codeSnippet();
    case IsAiGeneratedRole:
        return finding.isAiGenerated();
    case SeverityWeightRole:
        return ReviewFinding::severityWeight(finding.severity());
    default:
        return {};
    }
}

QHash<int, QByteArray> ReviewFindingModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names[FilePathRole]      = "filePath";
    names[LineNumberRole]    = "lineNumber";
    names[SeverityRole]      = "severity";
    names[CategoryRole]      = "category";
    names[TitleRole]         = "title";
    names[MessageRole]       = "message";
    names[SuggestionRole]    = "suggestion";
    names[CodeSnippetRole]   = "codeSnippet";
    names[IsAiGeneratedRole] = "isAiGenerated";
    names[SeverityWeightRole] = "severityWeight";
    return names;
}

void ReviewFindingModel::setFindings(const std::vector<ReviewFinding>& findings)
{
    beginResetModel();
    m_findings.clear();
    m_findings.reserve(static_cast<int>(findings.size()));
    for (const auto& f : findings) {
        m_findings.append(f);
    }
    endResetModel();
}

ReviewFinding ReviewFindingModel::findingAt(int row) const
{
    if (row < 0 || row >= m_findings.size()) {
        return ReviewFinding();
    }
    return m_findings.at(row);
}

int ReviewFindingModel::totalCount() const
{
    return static_cast<int>(m_findings.size());
}

int ReviewFindingModel::countBySeverity(const QString& severity) const
{
    int count = 0;
    for (const auto& f : m_findings) {
        if (f.severity() == severity) {
            ++count;
        }
    }
    return count;
}

int ReviewFindingModel::criticalCount() const { return countBySeverity(QStringLiteral("critical")); }
int ReviewFindingModel::highCount() const     { return countBySeverity(QStringLiteral("high")); }
int ReviewFindingModel::mediumCount() const   { return countBySeverity(QStringLiteral("medium")); }
int ReviewFindingModel::lowCount() const      { return countBySeverity(QStringLiteral("low")); }
int ReviewFindingModel::infoCount() const     { return countBySeverity(QStringLiteral("info")); }

bool ReviewFindingModel::isEmpty() const
{
    return m_findings.isEmpty();
}

void ReviewFindingModel::clear()
{
    if (m_findings.isEmpty()) {
        return;
    }
    beginResetModel();
    m_findings.clear();
    endResetModel();
}
