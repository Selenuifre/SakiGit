#ifndef CODEHOSTINGPLATFORM_H
#define CODEHOSTINGPLATFORM_H

#include <QMetaType>
#include <QString>

// 代码托管平台枚举，用于标识不同的代码托管服务。
enum class CodeHostingPlatform
{
    GitHub,   // github.com / GitHub Enterprise
    Gitee,    // gitee.com / Gitee Enterprise
    GitLab,   // gitlab.com / 自托管 GitLab
    Unknown
};

// 将平台枚举转换为可读名称
inline QString codeHostingPlatformName(CodeHostingPlatform platform)
{
    switch (platform) {
    case CodeHostingPlatform::GitHub:
        return QStringLiteral("GitHub");
    case CodeHostingPlatform::Gitee:
        return QStringLiteral("Gitee");
    case CodeHostingPlatform::GitLab:
        return QStringLiteral("GitLab");
    default:
        return QStringLiteral("Unknown");
    }
}

// 从名称字符串解析平台枚举
inline CodeHostingPlatform codeHostingPlatformFromName(const QString& name)
{
    const QString lower = name.trimmed().toLower();
    if (lower == QStringLiteral("github"))
        return CodeHostingPlatform::GitHub;
    if (lower == QStringLiteral("gitee"))
        return CodeHostingPlatform::Gitee;
    if (lower == QStringLiteral("gitlab"))
        return CodeHostingPlatform::GitLab;
    return CodeHostingPlatform::Unknown;
}

Q_DECLARE_METATYPE(CodeHostingPlatform)

#endif // CODEHOSTINGPLATFORM_H
