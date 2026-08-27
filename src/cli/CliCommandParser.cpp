#include "CliCommandParser.h"

#include <QTextStream>

CliCommandParser::CliCommandParser() = default;

void CliCommandParser::registerCommand(std::unique_ptr<CliCommand> command)
{
    if (!command) {
        return;
    }

    const QString cmdName = command->name();
    m_commands[cmdName] = command.get();
    m_commandStorage.push_back(std::move(command));
}

CliCommandParser::ParseResult CliCommandParser::parse(const QStringList& args) const
{
    ParseResult result;

    if (args.size() < 2) {
        result.ok = false;
        result.showHelp = true;
        result.errorMessage = QStringLiteral("No subcommand specified.");
        return result;
    }

    // args[0] 是可执行文件名，从 args[1] 开始解析
    const QString firstArg = args.at(1);

    // 检查 --help（全局级别）
    if (firstArg == QStringLiteral("--help") || firstArg == QStringLiteral("-h")) {
        result.ok = true;
        result.showHelp = true;
        return result;
    }

    // 检查是否是子命令
    const auto it = m_commands.find(firstArg);
    if (it == m_commands.end()) {
        result.ok = false;
        result.showHelp = true;
        result.errorMessage = QStringLiteral("Unknown subcommand: '%1'").arg(firstArg);
        return result;
    }

    result.command = it.value();

    // 收集子命令的参数（args[2] 起）
    QStringList commandArgs;
    for (int i = 2; i < args.size(); ++i) {
        commandArgs.append(args.at(i));
    }

    // 检查子命令帮助
    for (const auto& arg : commandArgs) {
        if (arg == QStringLiteral("--help") || arg == QStringLiteral("-h")) {
            result.ok = true;
            result.showHelp = true;
            result.helpCommandName = firstArg;
            return result;
        }
    }

    result.commandArgs = commandArgs;
    result.ok = true;
    return result;
}

QStringList CliCommandParser::registeredCommandNames() const
{
    return m_commands.keys();
}

QString CliCommandParser::globalHelp() const
{
    QString help;
    QTextStream out(&help);

    out << "SakiGit - AI-assisted Git command line tool\n"
        << "\n"
        << "Usage: SakiGit <command> [options]\n"
        << "\n"
        << "Commands:\n";

    for (const auto& cmd : m_commandStorage) {
        out << "  " << cmd->name();
        // 对齐描述（命令名最长 20 字符）
        const int padding = 22 - cmd->name().length();
        if (padding > 0) {
            out << QString(padding, ' ');
        }
        out << cmd->description() << "\n";
    }

    out << "\n"
        << "Common options:\n"
        << "  --repo <path>     Repository path (default: current directory)\n"
        << "  --staged          Review staged changes only (for 'review' command)\n"
        << "  --strict          Non-zero exit on critical/high findings (for 'review')\n"
        << "  --help, -h        Show help for a command\n"
        << "\n"
        << "Run 'SakiGit <command> --help' for command-specific options.\n"
        << "Without any command, SakiGit launches the graphical interface.\n";

    return help;
}

QString CliCommandParser::commandHelp(const QString& commandName) const
{
    const auto it = m_commands.find(commandName);
    if (it == m_commands.end()) {
        return QStringLiteral("Unknown command: '%1'\n").arg(commandName);
    }

    QString help;
    QTextStream out(&help);

    out << "Usage: SakiGit " << it.value()->usage() << "\n"
        << "\n"
        << it.value()->description() << "\n";

    return help;
}

QStringList CliCommandParser::extractCommonOptions(const QStringList& args,
                                                    QString& repoPath,
                                                    bool& staged,
                                                    bool& strict,
                                                    bool& showHelp) const
{
    QStringList remaining;

    for (int i = 0; i < args.size(); ++i) {
        const QString& arg = args.at(i);

        if (arg == QStringLiteral("--help") || arg == QStringLiteral("-h")) {
            showHelp = true;
        } else if (arg == QStringLiteral("--staged")) {
            staged = true;
        } else if (arg == QStringLiteral("--strict")) {
            strict = true;
        } else if (arg == QStringLiteral("--repo")) {
            if (i + 1 < args.size()) {
                repoPath = args.at(i + 1);
                ++i; // 跳过下一个参数（repo 路径）
            }
        } else {
            remaining.append(arg);
        }
    }

    return remaining;
}
