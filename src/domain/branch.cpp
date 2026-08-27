#include "DomainUtils.h"
#include "branch.h"

Branch::Branch()
    : m_type(GitTypes::BranchType::Local),
    m_current(false),
    m_aheadCount(0),
    m_behindCount(0)
{
}

Branch::Branch(const QString& name)
    : Branch()
{
    // 先从完整 ref 名推断分支类型
    const GitTypes::RefType refType = GitTypes::refTypeFromName(name);
    if (refType == GitTypes::RefType::RemoteBranch) {
        setType(GitTypes::BranchType::Remote);
    } else if (refType == GitTypes::RefType::LocalBranch) {
        setType(GitTypes::BranchType::Local);
    }

    setName(name);
}

Branch::Branch(const QString& name, GitTypes::BranchType type)
    : Branch()
{
    setName(name);
    setType(type);
}

QString Branch::name() const
{
    return m_name;
}

void Branch::setName(const QString& name)
{
    m_name = GitTypes::shortRefName(name.trimmed());

    if (m_fullName.isEmpty()) {
        assignTrimmed(m_fullName, name);
    }

    if (m_type == GitTypes::BranchType::Remote && m_remoteName.isEmpty()) {
        m_remoteName = GitTypes::remoteNameFromBranchName(m_name);
    }
}

QString Branch::fullName() const
{
    return m_fullName;
}

void Branch::setFullName(const QString& fullName)
{
    assignTrimmed(m_fullName, fullName);

    if (m_name.isEmpty()) {
        m_name = GitTypes::shortRefName(m_fullName);
    }

    // 从完整 ref 名自动推断分支类型
    const GitTypes::RefType refType = GitTypes::refTypeFromName(m_fullName);
    if (refType == GitTypes::RefType::RemoteBranch) {
        m_type = GitTypes::BranchType::Remote;
    } else if (refType == GitTypes::RefType::LocalBranch) {
        m_type = GitTypes::BranchType::Local;
    }
}

GitTypes::BranchType Branch::type() const
{
    return m_type;
}

void Branch::setType(GitTypes::BranchType type)
{
    m_type = type;

    if (m_type == GitTypes::BranchType::Remote && m_remoteName.isEmpty()) {
        m_remoteName = GitTypes::remoteNameFromBranchName(m_name);
    }
}

bool Branch::isLocal() const
{
    return m_type == GitTypes::BranchType::Local;
}

bool Branch::isRemote() const
{
    return m_type == GitTypes::BranchType::Remote;
}

bool Branch::isCurrent() const
{
    return m_current;
}

void Branch::setCurrent(bool current)
{
    m_current = current;
}

QString Branch::remoteName() const
{
    return m_remoteName;
}

void Branch::setRemoteName(const QString& remoteName)
{
    assignTrimmed(m_remoteName, remoteName);
}

QString Branch::upstreamName() const
{
    return m_upstreamName;
}

void Branch::setUpstreamName(const QString& upstreamName)
{
    assignTrimmed(m_upstreamName, upstreamName);
}

bool Branch::hasUpstream() const
{
    return !m_upstreamName.isEmpty();
}

int Branch::aheadCount() const
{
    return m_aheadCount;
}

void Branch::setAheadCount(int aheadCount)
{
    m_aheadCount = aheadCount < 0 ? 0 : aheadCount;
}

int Branch::behindCount() const
{
    return m_behindCount;
}

void Branch::setBehindCount(int behindCount)
{
    m_behindCount = behindCount < 0 ? 0 : behindCount;
}

bool Branch::isSynced() const
{
    return m_aheadCount == 0 && m_behindCount == 0;
}

bool Branch::hasUnpushedCommits() const
{
    return m_aheadCount > 0;
}

bool Branch::hasUnpulledCommits() const
{
    return m_behindCount > 0;
}

bool Branch::hasDiverged() const
{
    return m_aheadCount > 0 && m_behindCount > 0;
}

QString Branch::headCommitHash() const
{
    return m_headCommitHash;
}

void Branch::setHeadCommitHash(const QString& headCommitHash)
{
    assignTrimmed(m_headCommitHash, headCommitHash);
}

QString Branch::shortHeadCommitHash(int length) const
{
    return GitTypes::shortHash(m_headCommitHash, length);
}

QString Branch::lastCommitSummary() const
{
    return m_lastCommitSummary;
}

void Branch::setLastCommitSummary(const QString& lastCommitSummary)
{
    assignTrimmed(m_lastCommitSummary, lastCommitSummary);
}

QDateTime Branch::lastCommitDate() const
{
    return m_lastCommitDate;
}

void Branch::setLastCommitDate(const QDateTime& lastCommitDate)
{
    m_lastCommitDate = lastCommitDate;
}

bool Branch::isValid() const
{
    return !m_name.isEmpty();
}

QString Branch::displayName() const
{
    if (!m_name.isEmpty()) {
        return m_name;
    }

    return QStringLiteral("Unnamed branch");
}

QString Branch::syncStatusText() const
{
    if (!hasUpstream()) {
        return QStringLiteral("No upstream");
    }

    if (isSynced()) {
        return QStringLiteral("Synced");
    }

    if (hasDiverged()) {
        return QStringLiteral("%1 ahead, %2 behind")
        .arg(m_aheadCount)
            .arg(m_behindCount);
    }

    if (hasUnpushedCommits()) {
        return QStringLiteral("%1 ahead").arg(m_aheadCount);
    }

    if (hasUnpulledCommits()) {
        return QStringLiteral("%1 behind").arg(m_behindCount);
    }

    return QStringLiteral("Unknown");
}

bool Branch::operator==(const Branch& other) const
{
    if (!m_fullName.isEmpty() || !other.m_fullName.isEmpty()) {
        return m_fullName == other.m_fullName
               && m_type == other.m_type;
    }

    return m_type == other.m_type && m_name == other.m_name;
}

