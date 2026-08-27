#include "DomainUtils.h"
#include "commit.h"

#include "gittypes.h"

Commit::Commit()
{
}

Commit::Commit(const QString& hash)
{
    setHash(hash);
}

Commit::Commit(const QString& hash, const QString& summary)
{
    setHash(hash);
    setSummary(summary);
}

QString Commit::hash() const
{
    return m_hash;
}

void Commit::setHash(const QString& hash)
{
    assignTrimmed(m_hash, hash);
}

QString Commit::shortHash(int length) const
{
    return GitTypes::shortHash(m_hash, length);
}

QString Commit::summary() const
{
    return m_summary;
}

void Commit::setSummary(const QString& summary)
{
    assignTrimmed(m_summary, summary);
}

QString Commit::body() const
{
    return m_body;
}

void Commit::setBody(const QString& body)
{
    assignTrimmed(m_body, body);
}

QString Commit::message() const
{
    if (m_body.isEmpty()) {
        return m_summary;
    }

    if (m_summary.isEmpty()) {
        return m_body;
    }

    return m_summary + QStringLiteral("\n\n") + m_body;
}

void Commit::setMessage(const QString& message)
{
    m_summary = summaryFromMessage(message);
    m_body = bodyFromMessage(message);
}

QString Commit::authorName() const
{
    return m_authorName;
}

void Commit::setAuthorName(const QString& authorName)
{
    assignTrimmed(m_authorName, authorName);
}

QString Commit::authorEmail() const
{
    return m_authorEmail;
}

void Commit::setAuthorEmail(const QString& authorEmail)
{
    assignTrimmed(m_authorEmail, authorEmail);
}

QDateTime Commit::authorDate() const
{
    return m_authorDate;
}

void Commit::setAuthorDate(const QDateTime& authorDate)
{
    m_authorDate = authorDate;
}

QString Commit::committerName() const
{
    return m_committerName;
}

void Commit::setCommitterName(const QString& committerName)
{
    assignTrimmed(m_committerName, committerName);
}

QString Commit::committerEmail() const
{
    return m_committerEmail;
}

void Commit::setCommitterEmail(const QString& committerEmail)
{
    assignTrimmed(m_committerEmail, committerEmail);
}

QDateTime Commit::committerDate() const
{
    return m_committerDate;
}

void Commit::setCommitterDate(const QDateTime& committerDate)
{
    m_committerDate = committerDate;
}

QStringList Commit::parentHashes() const
{
    return m_parentHashes;
}

void Commit::setParentHashes(const QStringList& parentHashes)
{
    m_parentHashes.clear();

    for (const QString& parentHash : parentHashes) {
        addParentHash(parentHash);
    }
}

void Commit::addParentHash(const QString& parentHash)
{
    addUniqueTrimmed(m_parentHashes, parentHash);
}

QStringList Commit::changedFiles() const
{
    return m_changedFiles;
}

void Commit::setChangedFiles(const QStringList& changedFiles)
{
    m_changedFiles.clear();

    for (const QString& filePath : changedFiles) {
        addChangedFile(filePath);
    }
}

void Commit::addChangedFile(const QString& filePath)
{
    addUniqueTrimmed(m_changedFiles, filePath);
}

bool Commit::isValid() const
{
    return !m_hash.isEmpty();
}

bool Commit::isMergeCommit() const
{
    return m_parentHashes.size() > 1;
}

bool Commit::hasBody() const
{
    return !m_body.isEmpty();
}

QString Commit::displayTitle() const
{
    if (!m_summary.isEmpty()) {
        return m_summary;
    }

    if (!m_hash.isEmpty()) {
        return shortHash();
    }

    return QStringLiteral("Untitled commit");
}

QString Commit::displayAuthor() const
{
    if (!m_authorName.isEmpty()) {
        return m_authorName;
    }

    if (!m_authorEmail.isEmpty()) {
        return m_authorEmail;
    }

    return QStringLiteral("Unknown author");
}

QString Commit::displayDate(const QString& format) const
{
    if (!m_authorDate.isValid()) {
        return QString();
    }

    return m_authorDate.toString(format);
}

QString Commit::summaryFromMessage(const QString& message)
{
    const QString cleanMessage = message.trimmed();

    if (cleanMessage.isEmpty()) {
        return QString();
    }

    const QStringList lines = cleanMessage.split(QLatin1Char('\n'));
    return lines.first().trimmed();
}

QString Commit::bodyFromMessage(const QString& message)
{
    const QString cleanMessage = message.trimmed();

    if (cleanMessage.isEmpty()) {
        return QString();
    }

    const int firstLineEnd = cleanMessage.indexOf(QLatin1Char('\n'));

    if (firstLineEnd < 0) {
        return QString();
    }

    return cleanMessage.mid(firstLineEnd + 1).trimmed();
}

bool Commit::operator==(const Commit& other) const
{
    return m_hash == other.m_hash;
}
