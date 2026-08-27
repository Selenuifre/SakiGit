#include "DomainUtils.h"
#include "Stash.h"

#include "gittypes.h"

Stash::Stash()
    : m_index(-1)
{
}

Stash::Stash(int index)
    : m_index(index < 0 ? -1 : index)
{
}

Stash::Stash(int index, const QString& message)
    : m_index(index < 0 ? -1 : index),
    m_message(message.trimmed())
{
}

int Stash::index() const
{
    return m_index;
}

void Stash::setIndex(int index)
{
    m_index = index < 0 ? -1 : index;
}

QString Stash::message() const
{
    return m_message;
}

void Stash::setMessage(const QString& message)
{
    assignTrimmed(m_message, message);
}

QString Stash::branchName() const
{
    return m_branchName;
}

void Stash::setBranchName(const QString& branchName)
{
    m_branchName = GitTypes::shortRefName(branchName.trimmed());
}

QString Stash::commitHash() const
{
    return m_commitHash;
}

void Stash::setCommitHash(const QString& commitHash)
{
    assignTrimmed(m_commitHash, commitHash);
}

QString Stash::shortCommitHash(int length) const
{
    return GitTypes::shortHash(m_commitHash, length);
}

QDateTime Stash::date() const
{
    return m_date;
}

void Stash::setDate(const QDateTime& date)
{
    m_date = date;
}

bool Stash::isValid() const
{
    return m_index >= 0;
}

QString Stash::displayName() const
{
    if (m_index < 0) {
        return QStringLiteral("Unknown stash");
    }

    return QStringLiteral("stash@{%1}").arg(m_index);
}

QString Stash::displayBranch() const
{
    if (!m_branchName.isEmpty()) {
        return m_branchName;
    }

    return QStringLiteral("(unknown branch)");
}

bool Stash::operator==(const Stash& other) const
{
    return m_index == other.m_index;
}
