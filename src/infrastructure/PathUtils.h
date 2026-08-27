#ifndef PATHUTILS_H
#define PATHUTILS_H

#include <QDir>
#include <QString>

// 路径处理工具函数，供 infrastructure 和 domain 层共享使用。

// 修剪空白并规范化路径分隔符，返回 QDir::cleanPath 结果。
// 提取自 gitcommandexecutor.cpp 匿名命名空间的 cleanWorkingDirectory()。
inline QString cleanWorkingDirectory(const QString& workingDirectory)
{
    const QString trimmedDirectory = workingDirectory.trimmed();

    if (trimmedDirectory.isEmpty()) {
        return QString();
    }

    return QDir::cleanPath(trimmedDirectory);
}

// 将反斜杠统一转换为正斜杠（Git 格式），不涉及文件系统操作。
// 原为 GitTypes::normalizePath 的语义，现已重命名为 cleanPathSeparators。
inline QString normalizeSeparators(const QString& path)
{
    QString cleanPath = path.trimmed();
    cleanPath.replace(QLatin1Char('\\'), QLatin1Char('/'));
    return cleanPath;
}

#endif // PATHUTILS_H
