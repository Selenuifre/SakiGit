#include "DomainUtils.h"
#include "diff.h"

#include <QRegularExpression>
#include <QStringList>

DiffLine::DiffLine()
    : m_type(GitTypes::DiffLineType::Unknown),
    m_oldLineNumber(-1),
    m_newLineNumber(-1)
{
}

DiffLine::DiffLine(GitTypes::DiffLineType type, const QString& text)
    : DiffLine()
{
    setType(type);
    setText(text);
}

DiffLine::DiffLine(GitTypes::DiffLineType type, const QString& text, int oldLineNumber, int newLineNumber)
    : DiffLine(type, text)
{
    setOldLineNumber(oldLineNumber);
    setNewLineNumber(newLineNumber);
}

GitTypes::DiffLineType DiffLine::type() const
{
    return m_type;
}

void DiffLine::setType(GitTypes::DiffLineType type)
{
    m_type = type;
}

QString DiffLine::text() const
{
    return m_text;
}

void DiffLine::setText(const QString& text)
{
    m_text = text;
}

int DiffLine::oldLineNumber() const
{
    return m_oldLineNumber;
}

void DiffLine::setOldLineNumber(int oldLineNumber)
{
    m_oldLineNumber = oldLineNumber;
}

int DiffLine::newLineNumber() const
{
    return m_newLineNumber;
}

void DiffLine::setNewLineNumber(int newLineNumber)
{
    m_newLineNumber = newLineNumber;
}

bool DiffLine::isAdded() const
{
    return m_type == GitTypes::DiffLineType::Added;
}

bool DiffLine::isRemoved() const
{
    return m_type == GitTypes::DiffLineType::Removed;
}

bool DiffLine::isContext() const
{
    return m_type == GitTypes::DiffLineType::Context;
}

bool DiffLine::isHunkHeader() const
{
    return m_type == GitTypes::DiffLineType::HunkHeader;
}

QString DiffLine::displayPrefix() const
{
    switch (m_type) {
    case GitTypes::DiffLineType::Added:
        return QStringLiteral("+");
    case GitTypes::DiffLineType::Removed:
        return QStringLiteral("-");
    case GitTypes::DiffLineType::Context:
        return QStringLiteral(" ");
    case GitTypes::DiffLineType::NoNewline:
        return QStringLiteral("\\");
    case GitTypes::DiffLineType::HunkHeader:
    case GitTypes::DiffLineType::Unknown:
    default:
        return QString();
    }
}

DiffHunk::DiffHunk()
    : m_oldStart(0),
    m_oldLineCount(0),
    m_newStart(0),
    m_newLineCount(0)
{
}

DiffHunk::DiffHunk(const QString& header)
    : DiffHunk()
{
    setHeader(header);
    parseHeader();
}

QString DiffHunk::header() const
{
    return m_header;
}

void DiffHunk::setHeader(const QString& header)
{
    assignTrimmed(m_header, header);
}

int DiffHunk::oldStart() const
{
    return m_oldStart;
}

void DiffHunk::setOldStart(int oldStart)
{
    m_oldStart = oldStart;
}

int DiffHunk::oldLineCount() const
{
    return m_oldLineCount;
}

void DiffHunk::setOldLineCount(int oldLineCount)
{
    m_oldLineCount = oldLineCount;
}

int DiffHunk::newStart() const
{
    return m_newStart;
}

void DiffHunk::setNewStart(int newStart)
{
    m_newStart = newStart;
}

int DiffHunk::newLineCount() const
{
    return m_newLineCount;
}

void DiffHunk::setNewLineCount(int newLineCount)
{
    m_newLineCount = newLineCount;
}

QList<DiffLine> DiffHunk::lines() const
{
    return m_lines;
}

void DiffHunk::setLines(const QList<DiffLine>& lines)
{
    m_lines = lines;
}

void DiffHunk::addLine(const DiffLine& line)
{
    m_lines.append(line);
}

bool DiffHunk::isValid() const
{
    return !m_header.isEmpty();
}

int DiffHunk::addedLineCount() const
{
    int count = 0;

    for (const DiffLine& line : m_lines) {
        if (line.isAdded()) {
            ++count;
        }
    }

    return count;
}

int DiffHunk::removedLineCount() const
{
    int count = 0;

    for (const DiffLine& line : m_lines) {
        if (line.isRemoved()) {
            ++count;
        }
    }

    return count;
}

void DiffHunk::parseHeader()
{
    static const QRegularExpression regex(
        QStringLiteral("^@@\\s+-(\\d+)(?:,(\\d+))?\\s+\\+(\\d+)(?:,(\\d+))?\\s+@@"));

    const QRegularExpressionMatch match = regex.match(m_header);

    if (!match.hasMatch()) {
        return;
    }

    m_oldStart = match.captured(1).toInt();
    m_oldLineCount = match.captured(2).isEmpty() ? 1 : match.captured(2).toInt();
    m_newStart = match.captured(3).toInt();
    m_newLineCount = match.captured(4).isEmpty() ? 1 : match.captured(4).toInt();
}

DiffHunk DiffHunk::fromHeader(const QString& header)
{
    return DiffHunk(header);
}

FileDiff::FileDiff()
    : m_binary(false),
    m_newFile(false),
    m_deletedFile(false),
    m_renamed(false)
{
}

FileDiff::FileDiff(const QString& path)
    : FileDiff()
{
    setNewPath(path);
}

QString FileDiff::oldPath() const
{
    return m_oldPath;
}

void FileDiff::setOldPath(const QString& oldPath)
{
    m_oldPath = cleanGitPath(oldPath);
}

QString FileDiff::newPath() const
{
    return m_newPath;
}

void FileDiff::setNewPath(const QString& newPath)
{
    m_newPath = cleanGitPath(newPath);
}

QString FileDiff::displayPath() const
{
    if (m_renamed && !m_oldPath.isEmpty() && !m_newPath.isEmpty()) {
        return m_oldPath + QStringLiteral(" -> ") + m_newPath;
    }

    if (!m_newPath.isEmpty()) {
        return m_newPath;
    }

    return m_oldPath;
}

bool FileDiff::isBinary() const
{
    return m_binary;
}

void FileDiff::setBinary(bool binary)
{
    m_binary = binary;
}

bool FileDiff::isNewFile() const
{
    return m_newFile;
}

void FileDiff::setNewFile(bool newFile)
{
    m_newFile = newFile;
}

bool FileDiff::isDeletedFile() const
{
    return m_deletedFile;
}

void FileDiff::setDeletedFile(bool deletedFile)
{
    m_deletedFile = deletedFile;
}

bool FileDiff::isRenamed() const
{
    return m_renamed;
}

void FileDiff::setRenamed(bool renamed)
{
    m_renamed = renamed;
}

QList<DiffHunk> FileDiff::hunks() const
{
    return m_hunks;
}

void FileDiff::setHunks(const QList<DiffHunk>& hunks)
{
    m_hunks = hunks;
}

void FileDiff::addHunk(const DiffHunk& hunk)
{
    m_hunks.append(hunk);
}

bool FileDiff::isValid() const
{
    return !m_oldPath.isEmpty() || !m_newPath.isEmpty();
}

int FileDiff::addedLineCount() const
{
    int count = 0;

    for (const DiffHunk& hunk : m_hunks) {
        count += hunk.addedLineCount();
    }

    return count;
}

int FileDiff::removedLineCount() const
{
    int count = 0;

    for (const DiffHunk& hunk : m_hunks) {
        count += hunk.removedLineCount();
    }

    return count;
}

QString FileDiff::cleanGitPath(const QString& path)
{
    return GitTypes::cleanDiffPath(path);
}

Diff::Diff()
{
}

Diff::Diff(const QString& rawText)
{
    setRawText(rawText);
}

QString Diff::rawText() const
{
    return m_rawText;
}

void Diff::setRawText(const QString& rawText)
{
    m_rawText = rawText;
}

QList<FileDiff> Diff::files() const
{
    return m_files;
}

void Diff::setFiles(const QList<FileDiff>& files)
{
    m_files = files;
}

void Diff::addFile(const FileDiff& fileDiff)
{
    m_files.append(fileDiff);
}

void Diff::clear()
{
    m_rawText.clear();
    m_files.clear();
}

bool Diff::isEmpty() const
{
    return m_files.isEmpty() && m_rawText.trimmed().isEmpty();
}

int Diff::fileCount() const
{
    return m_files.size();
}

int Diff::addedLineCount() const
{
    int count = 0;

    for (const FileDiff& file : m_files) {
        count += file.addedLineCount();
    }

    return count;
}

int Diff::removedLineCount() const
{
    int count = 0;

    for (const FileDiff& file : m_files) {
        count += file.removedLineCount();
    }

    return count;
}

Diff Diff::fromUnifiedDiff(const QString& text)
{
    Diff diff(text);

    FileDiff currentFile;
    DiffHunk currentHunk;

    bool hasCurrentFile = false;
    bool hasCurrentHunk = false;

    int oldLineNumber = 0;
    int newLineNumber = 0;

    const QStringList lines = text.split(QLatin1Char('\n'));

    for (const QString& line : lines) {
        if (line.startsWith(QStringLiteral("diff --git "))) {
            if (hasCurrentHunk) {
                currentFile.addHunk(currentHunk);
                currentHunk = DiffHunk();
                hasCurrentHunk = false;
            }

            if (hasCurrentFile) {
                diff.addFile(currentFile);
            }

            currentFile = FileDiff();
            hasCurrentFile = true;
            continue;
        }

        if (!hasCurrentFile) {
            continue;
        }

        if (line.startsWith(QStringLiteral("new file mode"))) {
            currentFile.setNewFile(true);
            continue;
        }

        if (line.startsWith(QStringLiteral("deleted file mode"))) {
            currentFile.setDeletedFile(true);
            continue;
        }

        if (line.startsWith(QStringLiteral("similarity index"))) {
            currentFile.setRenamed(true);
            continue;
        }

        if (line.startsWith(QStringLiteral("Binary files "))) {
            currentFile.setBinary(true);
            continue;
        }

        if (line.startsWith(QStringLiteral("--- "))) {
            currentFile.setOldPath(line.mid(4));
            continue;
        }

        if (line.startsWith(QStringLiteral("+++ "))) {
            currentFile.setNewPath(line.mid(4));
            continue;
        }

        if (line.startsWith(QStringLiteral("@@"))) {
            if (hasCurrentHunk) {
                currentFile.addHunk(currentHunk);
            }

            currentHunk = DiffHunk::fromHeader(line);
            hasCurrentHunk = true;

            oldLineNumber = currentHunk.oldStart();
            newLineNumber = currentHunk.newStart();

            continue;
        }

        if (!hasCurrentHunk) {
            continue;
        }

        const GitTypes::DiffLineType type = GitTypes::diffLineTypeFromLine(line);
        QString content = line;

        if (!content.isEmpty()
            && (type == GitTypes::DiffLineType::Added
                || type == GitTypes::DiffLineType::Removed
                || type == GitTypes::DiffLineType::Context)) {
            content.remove(0, 1);
        }

        DiffLine diffLine(type, content);

        if (type == GitTypes::DiffLineType::Added) {
            diffLine.setNewLineNumber(newLineNumber++);
        } else if (type == GitTypes::DiffLineType::Removed) {
            diffLine.setOldLineNumber(oldLineNumber++);
        } else if (type == GitTypes::DiffLineType::Context) {
            diffLine.setOldLineNumber(oldLineNumber++);
            diffLine.setNewLineNumber(newLineNumber++);
        }

        currentHunk.addLine(diffLine);
    }

    if (hasCurrentHunk) {
        currentFile.addHunk(currentHunk);
    }

    if (hasCurrentFile) {
        diff.addFile(currentFile);
    }

    return diff;
}
