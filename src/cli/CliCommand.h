#ifndef CLICOMMAND_H
#define CLICOMMAND_H

#include "CliResult.h"

#include <QString>
#include <QStringList>

// CLI 子命令抽象基类。
// 每个子命令（commit-message、review）实现此接口，
// 便于 CliCommandParser 统一管理和扩展。
class CliCommand
{
public:
    virtual ~CliCommand() = default;

    // 返回子命令名称（如 "commit-message", "review"）
    virtual QString name() const = 0;

    // 返回子命令的简短描述
    virtual QString description() const = 0;

    // 返回子命令的使用说明（参数格式）
    virtual QString usage() const = 0;

    // 执行命令
    // args: 该子命令的参数列表（不含命令名本身）
    // 返回 CliResult，其中 exitCode = 0 表示成功
    virtual CliResult run(const QStringList& args) = 0;
};

#endif // CLICOMMAND_H
