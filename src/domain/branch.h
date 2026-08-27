#ifndef BRANCH_H
#define BRANCH_H

#include <QDateTime>
#include <QMetaType>
#include <QString>

#include "gittypes.h"
#include "Operators.h"

class Branch : public EqualityOperators<Branch>
{
public:
    Branch();
    explicit Branch(const QString& name);
    Branch(const QString& name, GitTypes::BranchType type);

    QString name() const;
    void setName(const QString& name);

    QString fullName() const;
    void setFullName(const QString& fullName);

    GitTypes::BranchType type() const;
    void setType(GitTypes::BranchType type);

    bool isLocal() const;
    bool isRemote() const;

    bool isCurrent() const;
    void setCurrent(bool current);

    QString remoteName() const;
    void setRemoteName(const QString& remoteName);

    QString upstreamName() const;
    void setUpstreamName(const QString& upstreamName);

    bool hasUpstream() const;

    int aheadCount() const;
    void setAheadCount(int aheadCount);

    int behindCount() const;
    void setBehindCount(int behindCount);

    bool isSynced() const;
    bool hasUnpushedCommits() const;
    bool hasUnpulledCommits() const;
    bool hasDiverged() const;

    QString headCommitHash() const;
    void setHeadCommitHash(const QString& headCommitHash);

    QString shortHeadCommitHash(int length = 7) const;

    QString lastCommitSummary() const;
    void setLastCommitSummary(const QString& lastCommitSummary);

    QDateTime lastCommitDate() const;
    void setLastCommitDate(const QDateTime& lastCommitDate);

    bool isValid() const;

    QString displayName() const;
    QString syncStatusText() const;

    bool operator==(const Branch& other) const;

private:
    QString m_name;
    QString m_fullName;

    GitTypes::BranchType m_type;
    bool m_current;

    QString m_remoteName;
    QString m_upstreamName;

    int m_aheadCount;
    int m_behindCount;

    QString m_headCommitHash;
    QString m_lastCommitSummary;
    QDateTime m_lastCommitDate;
};

Q_DECLARE_METATYPE(Branch)

#endif // BRANCH_H
