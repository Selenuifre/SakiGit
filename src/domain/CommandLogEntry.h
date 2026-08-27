#ifndef COMMANDLOGENTRY_H
#define COMMANDLOGENTRY_H

#include <QDateTime>
#include <QMetaType>
#include <QString>

// 命令行日志条目，记录每次 Git 命令执行的完整信息。
// 由 GitCommandExecutor 回调产生，经 TerminalService 存入 TerminalOutputModel。
struct CommandLogEntry
{
    QString commandLine;    // 完整命令行，如 "git status --porcelain"
    QString workingDir;     // 执行时的工作目录
    int exitCode = 0;       // 进程退出码，0 = 成功
    QString output;         // stdout + stderr 合并文本
    QDateTime timestamp;    // 命令执行时间
    bool isUserInput = false; // true=用户在终端窗口手动输入, false=GUI 操作自动触发

    CommandLogEntry() = default;

    CommandLogEntry(const QString& cmd, const QString& wd, int ec,
                    const QString& out, bool user = false)
        : commandLine(cmd), workingDir(wd), exitCode(ec), output(out),
          timestamp(QDateTime::currentDateTime()), isUserInput(user)
    {
    }

    // 用于终端输出区显示的格式化行（含时间戳）
    QString displayLine() const
    {
        const QString prefix = isUserInput
            ? QStringLiteral(">") : QStringLiteral("$");
        return QStringLiteral("[%1] %2 %3")
            .arg(timestamp.toString(QStringLiteral("HH:mm:ss")),
                 prefix, commandLine);
    }

    // 输出是否包含错误
    bool hasError() const { return exitCode != 0; }
    bool isValid() const { return !commandLine.isEmpty(); }
};

Q_DECLARE_METATYPE(CommandLogEntry)

#endif // COMMANDLOGENTRY_H
