#include "DomainUtils.h"
#include "ConflictFile.h"

ConflictFile::ConflictFile() = default;

ConflictFile::ConflictFile(const QString& path)
    : m_path(path.trimmed())
{
}

QString ConflictFile::path() const
{
    return m_path;
}

void ConflictFile::setPath(const QString& path)
{
    assignTrimmed(m_path, path);
}

QString ConflictFile::conflictType() const
{
    return m_conflictType;
}

void ConflictFile::setConflictType(const QString& type)
{
    assignTrimmed(m_conflictType, type);
}

bool ConflictFile::exists() const
{
    return m_exists;
}

void ConflictFile::setExists(bool exists)
{
    m_exists = exists;
}

bool ConflictFile::isValid() const
{
    return !m_path.isEmpty();
}
