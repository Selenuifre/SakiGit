#ifndef COMMITGRAPHMODEL_H
#define COMMITGRAPHMODEL_H

#include "domain/branch.h"
#include "domain/commit.h"
#include "domain/Tag.h"

#include <QColor>
#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>
#include <vector>

struct CommitGraphNode {
    int row = 0;
    int column = 0;
    QString hash;
    QString shortHash;
    QString summary;
    QDateTime authorDate;
    QStringList branchLabels;
    QStringList tagLabels;
    bool isHEAD = false;
    QStringList parentHashes;
    bool hasBranchLabels() const { return !branchLabels.isEmpty(); }
    bool hasTagLabels() const { return !tagLabels.isEmpty(); }
    bool isMerge() const { return parentHashes.size() > 1; }
};

struct CommitGraphLine {
    int fromRow, fromColumn;
    int toRow,   toColumn;
    QColor color;
};

class CommitGraphModel : public QObject {
    Q_OBJECT
public:
    explicit CommitGraphModel(QObject* parent = nullptr);

    void buildLayout(const std::vector<Commit>& commits,
                     const std::vector<Branch>& branches,
                     const std::vector<Tag>& tags,
                     const QString& headHash);

    int totalRows() const { return static_cast<int>(m_nodes.size()); }
    int totalColumns() const { return m_totalColumns; }
    const CommitGraphNode* nodeAt(int row) const;
    const std::vector<CommitGraphLine>& lines() const { return m_lines; }
    QColor branchColor(const QString& name) const;
    void clear();

signals:
    void layoutChanged();

private:
    QColor colorForIndex(int i) const;
    std::vector<CommitGraphNode> m_nodes;
    std::vector<CommitGraphLine> m_lines;
    int m_totalColumns = 0;
    mutable QHash<QString, QColor> m_branchColorCache;
};

#endif
