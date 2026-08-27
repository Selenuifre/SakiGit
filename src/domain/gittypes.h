#ifndef GITTYPES_H
#define GITTYPES_H

#include <QMetaType>
#include <QString>
#include <QStringList>

namespace GitTypes {

// Git 对象类型，后续可用于 log、refs、tag、树对象等解析结果。
enum class ObjectType {
    Unknown,
    Commit,
    Tree,
    Blob,
    Tag
};

// Git 引用类型，对应本地分支、远程分支、标签和 HEAD 等常见 ref。
enum class RefType {
    Unknown,
    LocalBranch,
    RemoteBranch,
    Tag,
    Head
};

// 仓库当前状态，用于表达是否可用以及是否存在工作区异常。
enum class RepositoryState {
    Unknown,
    Ready,
    Missing,
    NotGitRepository,
    BareRepository,
    DetachedHead,
    Merging,
    Rebasing,
    CherryPicking,
    Reverting
};

// 远程代码托管平台。
enum class RemoteProvider {
    Unknown,
    GitHub,
    GitLab,
    Bitbucket,
    Other
};

// 分支类型。
enum class BranchType {
    Local,
    Remote
};

// 文件变更类型。
enum class FileStatus {
    Unknown,
    Added,
    Modified,
    Deleted,
    Renamed,
    Copied,
    TypeChanged,
    Untracked,
    Ignored,
    Conflicted
};

// 文件暂存状态。
enum class StageState {
    Unknown,
    Unstaged,
    Staged,
    PartiallyStaged,
    Conflict
};

// Diff 行类型。
enum class DiffLineType {
    Unknown,
    Context,
    Added,
    Removed,
    HunkHeader,
    NoNewline
};

// 常用 Git ref 前缀。
inline constexpr const char* LocalBranchRefPrefix = "refs/heads/";
inline constexpr const char* RemoteBranchRefPrefix = "refs/remotes/";
inline constexpr const char* TagRefPrefix = "refs/tags/";

inline QString cleanPathSeparators(const QString& path)
{
    QString cleanPath = path.trimmed();
    cleanPath.replace(QLatin1Char('\\'), QLatin1Char('/'));
    return cleanPath;
}

inline QString cleanDiffPath(const QString& path)
{
    QString cleanPath = cleanPathSeparators(path);

    if (cleanPath.startsWith(QStringLiteral("a/")) || cleanPath.startsWith(QStringLiteral("b/"))) {
        cleanPath.remove(0, 2);
    }

    if (cleanPath == QStringLiteral("/dev/null")) {
        return QString();
    }

    return cleanPath;
}

inline QString shortHash(const QString& hash, int length = 7)
{
    const QString cleanHash = hash.trimmed();

    if (length <= 0 || cleanHash.length() <= length) {
        return cleanHash;
    }

    return cleanHash.left(length);
}

inline QString shortRefName(const QString& refName)
{
    QString cleanRef = refName.trimmed();

    if (cleanRef.startsWith(QString::fromLatin1(LocalBranchRefPrefix))) {
        cleanRef.remove(0, QString::fromLatin1(LocalBranchRefPrefix).length());
        return cleanRef;
    }

    if (cleanRef.startsWith(QString::fromLatin1(RemoteBranchRefPrefix))) {
        cleanRef.remove(0, QString::fromLatin1(RemoteBranchRefPrefix).length());
        return cleanRef;
    }

    if (cleanRef.startsWith(QString::fromLatin1(TagRefPrefix))) {
        cleanRef.remove(0, QString::fromLatin1(TagRefPrefix).length());
        return cleanRef;
    }

    return cleanRef;
}

inline RefType refTypeFromName(const QString& refName)
{
    const QString cleanRef = refName.trimmed();

    if (cleanRef == QStringLiteral("HEAD")) {
        return RefType::Head;
    }

    if (cleanRef.startsWith(QString::fromLatin1(LocalBranchRefPrefix))) {
        return RefType::LocalBranch;
    }

    if (cleanRef.startsWith(QString::fromLatin1(RemoteBranchRefPrefix))) {
        return RefType::RemoteBranch;
    }

    if (cleanRef.startsWith(QString::fromLatin1(TagRefPrefix))) {
        return RefType::Tag;
    }

    return RefType::Unknown;
}

inline QString remoteNameFromBranchName(const QString& branchName)
{
    const QString cleanName = branchName.trimmed();
    const int slashIndex = cleanName.indexOf(QLatin1Char('/'));

    if (slashIndex <= 0) {
        return QString();
    }

    return cleanName.left(slashIndex);
}

inline QString localNameFromRemoteBranchName(const QString& branchName)
{
    const QString cleanName = branchName.trimmed();
    const int slashIndex = cleanName.indexOf(QLatin1Char('/'));

    if (slashIndex < 0 || slashIndex + 1 >= cleanName.length()) {
        return cleanName;
    }

    return cleanName.mid(slashIndex + 1);
}

inline RemoteProvider remoteProviderFromUrl(const QString& remoteUrl)
{
    const QString url = remoteUrl.trimmed().toLower();

    if (url.contains(QStringLiteral("github.com"))) {
        return RemoteProvider::GitHub;
    }

    if (url.contains(QStringLiteral("gitlab.com"))) {
        return RemoteProvider::GitLab;
    }

    if (url.contains(QStringLiteral("bitbucket.org"))) {
        return RemoteProvider::Bitbucket;
    }

    if (!url.isEmpty()) {
        return RemoteProvider::Other;
    }

    return RemoteProvider::Unknown;
}

inline FileStatus fileStatusFromPorcelain(const QString& code)
{
    const QString cleanCode = code.left(2);

    if (cleanCode == QStringLiteral("??")) {
        return FileStatus::Untracked;
    }

    if (cleanCode == QStringLiteral("!!")) {
        return FileStatus::Ignored;
    }

    if (cleanCode.contains(QLatin1Char('U'))
        || cleanCode == QStringLiteral("AA")
        || cleanCode == QStringLiteral("DD")
        || cleanCode == QStringLiteral("AU")
        || cleanCode == QStringLiteral("UA")
        || cleanCode == QStringLiteral("DU")
        || cleanCode == QStringLiteral("UD")) {
        return FileStatus::Conflicted;
    }

    if (cleanCode.contains(QLatin1Char('R'))) {
        return FileStatus::Renamed;
    }

    if (cleanCode.contains(QLatin1Char('C'))) {
        return FileStatus::Copied;
    }

    if (cleanCode.contains(QLatin1Char('A'))) {
        return FileStatus::Added;
    }

    if (cleanCode.contains(QLatin1Char('D'))) {
        return FileStatus::Deleted;
    }

    if (cleanCode.contains(QLatin1Char('T'))) {
        return FileStatus::TypeChanged;
    }

    if (cleanCode.contains(QLatin1Char('M'))) {
        return FileStatus::Modified;
    }

    return FileStatus::Unknown;
}

inline StageState stageStateFromPorcelain(const QString& code)
{
    const QString cleanCode = code.left(2);

    if (cleanCode.length() < 2) {
        return StageState::Unknown;
    }

    if (cleanCode == QStringLiteral("??")) {
        return StageState::Unstaged;
    }

    if (cleanCode.contains(QLatin1Char('U'))
        || cleanCode == QStringLiteral("AA")
        || cleanCode == QStringLiteral("DD")) {
        return StageState::Conflict;
    }

    const QChar indexStatus = cleanCode.at(0);
    const QChar worktreeStatus = cleanCode.at(1);

    const bool hasIndexChange = indexStatus != QLatin1Char(' ');
    const bool hasWorktreeChange = worktreeStatus != QLatin1Char(' ');

    if (hasIndexChange && hasWorktreeChange) {
        return StageState::PartiallyStaged;
    }

    if (hasIndexChange) {
        return StageState::Staged;
    }

    if (hasWorktreeChange) {
        return StageState::Unstaged;
    }

    return StageState::Unknown;
}

inline DiffLineType diffLineTypeFromLine(const QString& line)
{
    if (line.startsWith(QStringLiteral("@@"))) {
        return DiffLineType::HunkHeader;
    }

    if (line.startsWith(QStringLiteral("\\ No newline"))) {
        return DiffLineType::NoNewline;
    }

    if (line.startsWith(QLatin1Char('+')) && !line.startsWith(QStringLiteral("+++"))) {
        return DiffLineType::Added;
    }

    if (line.startsWith(QLatin1Char('-')) && !line.startsWith(QStringLiteral("---"))) {
        return DiffLineType::Removed;
    }

    if (line.startsWith(QLatin1Char(' '))) {
        return DiffLineType::Context;
    }

    return DiffLineType::Unknown;
}

inline QString toString(ObjectType type)
{
    switch (type) {
    case ObjectType::Commit:
        return QStringLiteral("Commit");
    case ObjectType::Tree:
        return QStringLiteral("Tree");
    case ObjectType::Blob:
        return QStringLiteral("Blob");
    case ObjectType::Tag:
        return QStringLiteral("Tag");
    case ObjectType::Unknown:
    default:
        return QStringLiteral("Unknown");
    }
}

inline QString toString(RefType type)
{
    switch (type) {
    case RefType::LocalBranch:
        return QStringLiteral("LocalBranch");
    case RefType::RemoteBranch:
        return QStringLiteral("RemoteBranch");
    case RefType::Tag:
        return QStringLiteral("Tag");
    case RefType::Head:
        return QStringLiteral("HEAD");
    case RefType::Unknown:
    default:
        return QStringLiteral("Unknown");
    }
}

inline QString toString(RepositoryState state)
{
    switch (state) {
    case RepositoryState::Ready:
        return QStringLiteral("Ready");
    case RepositoryState::Missing:
        return QStringLiteral("Missing");
    case RepositoryState::NotGitRepository:
        return QStringLiteral("NotGitRepository");
    case RepositoryState::BareRepository:
        return QStringLiteral("BareRepository");
    case RepositoryState::DetachedHead:
        return QStringLiteral("DetachedHead");
    case RepositoryState::Merging:
        return QStringLiteral("Merging");
    case RepositoryState::Rebasing:
        return QStringLiteral("Rebasing");
    case RepositoryState::CherryPicking:
        return QStringLiteral("CherryPicking");
    case RepositoryState::Reverting:
        return QStringLiteral("Reverting");
    case RepositoryState::Unknown:
    default:
        return QStringLiteral("Unknown");
    }
}

inline QString toString(RemoteProvider provider)
{
    switch (provider) {
    case RemoteProvider::GitHub:
        return QStringLiteral("GitHub");
    case RemoteProvider::GitLab:
        return QStringLiteral("GitLab");
    case RemoteProvider::Bitbucket:
        return QStringLiteral("Bitbucket");
    case RemoteProvider::Other:
        return QStringLiteral("Other");
    case RemoteProvider::Unknown:
    default:
        return QStringLiteral("Unknown");
    }
}

inline QString toString(BranchType type)
{
    switch (type) {
    case BranchType::Remote:
        return QStringLiteral("Remote");
    case BranchType::Local:
    default:
        return QStringLiteral("Local");
    }
}

inline QString toString(FileStatus status)
{
    switch (status) {
    case FileStatus::Added:
        return QStringLiteral("Added");
    case FileStatus::Modified:
        return QStringLiteral("Modified");
    case FileStatus::Deleted:
        return QStringLiteral("Deleted");
    case FileStatus::Renamed:
        return QStringLiteral("Renamed");
    case FileStatus::Copied:
        return QStringLiteral("Copied");
    case FileStatus::TypeChanged:
        return QStringLiteral("Type changed");
    case FileStatus::Untracked:
        return QStringLiteral("Untracked");
    case FileStatus::Ignored:
        return QStringLiteral("Ignored");
    case FileStatus::Conflicted:
        return QStringLiteral("Conflicted");
    case FileStatus::Unknown:
    default:
        return QStringLiteral("Unknown");
    }
}

inline QString toString(StageState state)
{
    switch (state) {
    case StageState::Unstaged:
        return QStringLiteral("Unstaged");
    case StageState::Staged:
        return QStringLiteral("Staged");
    case StageState::PartiallyStaged:
        return QStringLiteral("Partially staged");
    case StageState::Conflict:
        return QStringLiteral("Conflict");
    case StageState::Unknown:
    default:
        return QStringLiteral("Unknown");
    }
}

inline QString toString(DiffLineType type)
{
    switch (type) {
    case DiffLineType::Context:
        return QStringLiteral("Context");
    case DiffLineType::Added:
        return QStringLiteral("Added");
    case DiffLineType::Removed:
        return QStringLiteral("Removed");
    case DiffLineType::HunkHeader:
        return QStringLiteral("Hunk header");
    case DiffLineType::NoNewline:
        return QStringLiteral("No newline");
    case DiffLineType::Unknown:
    default:
        return QStringLiteral("Unknown");
    }
}

} // namespace GitTypes

Q_DECLARE_METATYPE(GitTypes::ObjectType)
Q_DECLARE_METATYPE(GitTypes::RefType)
Q_DECLARE_METATYPE(GitTypes::RepositoryState)
Q_DECLARE_METATYPE(GitTypes::RemoteProvider)
Q_DECLARE_METATYPE(GitTypes::BranchType)
Q_DECLARE_METATYPE(GitTypes::FileStatus)
Q_DECLARE_METATYPE(GitTypes::StageState)
Q_DECLARE_METATYPE(GitTypes::DiffLineType)

#endif // GITTYPES_H
