#ifndef GENERATECOMMITCOMMAND_H
#define GENERATECOMMITCOMMAND_H

#include "CliCommand.h"

class CommitMessageService;
class SettingsService;

// commit-message 子命令。
// 获取暂存区 diff，调用 AI 服务生成符合 Conventional Commits 的提交信息。
// 结果以 JSON 格式输出到 stdout，错误信息输出到 stderr。
class GenerateCommitCommand : public CliCommand
{
public:
    GenerateCommitCommand(CommitMessageService* commitMessageService,
                          SettingsService* settingsService);

    QString name() const override;
    QString description() const override;
    QString usage() const override;
    CliResult run(const QStringList& args) override;

private:
    CommitMessageService* m_commitMessageService; // 不持有所有权
    SettingsService*      m_settingsService;      // 不持有所有权

    // 解析 --repo 参数，默认返回 "."
    QString extractRepoPath(const QStringList& args) const;

    // 将 CommitMessageSuggestion 格式化为 JSON 输出
    QString formatOutput(const class CommitMessageSuggestion& suggestion) const;
};

#endif // GENERATECOMMITCOMMAND_H
