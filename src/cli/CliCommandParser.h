#ifndef CLICOMMANDPARSER_H
#define CLICOMMANDPARSER_H

#include "CliCommand.h"

#include <QMap>
#include <QString>
#include <QStringList>

#include <memory>
#include <vector>

// 命令行参数解析器。
// 支持子命令模式（类似 git 的 commit、push），
// 解析 --repo <path>、--staged、--strict、--help 等通用选项。
class CliCommandParser
{
public:
    CliCommandParser();

    // 注册一个子命令（如 "commit-message"、"review"）
    void registerCommand(std::unique_ptr<CliCommand> command);

    // 解析结果
    struct ParseResult {
        bool        ok = false;       // 解析成功
        CliCommand* command = nullptr; // 匹配到的子命令（不持有所有权）
        QStringList commandArgs;      // 传递给子命令的参数
        QString     errorMessage;     // 解析失败时的错误信息
        bool        showHelp = false; // 用户请求帮助
        QString     helpCommandName;  // 请求帮助的子命令名（空 = 全局帮助）
    };

    // 解析命令行参数。
    // args[0] 应为可执行文件名，args[1] 起为子命令和选项。
    ParseResult parse(const QStringList& args) const;

    // 打印全局帮助信息（列出所有已注册的子命令）
    QString globalHelp() const;

    // 打印特定子命令的帮助信息
    QString commandHelp(const QString& commandName) const;

    // 返回已注册的命令名列表
    QStringList registeredCommandNames() const;

private:
    // 解析通用选项（--repo, --staged, --strict, --help）
    // 返回剩余未识别的参数
    QStringList extractCommonOptions(const QStringList& args,
                                     QString& repoPath,
                                     bool& staged,
                                     bool& strict,
                                     bool& showHelp) const;

    QMap<QString, CliCommand*> m_commands;
    std::vector<std::unique_ptr<CliCommand>> m_commandStorage;
};

#endif // CLICOMMANDPARSER_H
