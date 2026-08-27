#ifndef REVIEWCOMMAND_H
#define REVIEWCOMMAND_H

#include "CliCommand.h"

#include <vector>

class CodeReviewService;
class ReviewFinding;
class SettingsService;

// review 子命令。
// 获取 diff（暂存区或工作区），调用 AI 服务进行代码审查。
// 结果以 JSON 格式输出到 stdout，错误信息输出到 stderr。
class ReviewCommand : public CliCommand
{
public:
    ReviewCommand(CodeReviewService* codeReviewService,
                  SettingsService* settingsService);

    QString name() const override;
    QString description() const override;
    QString usage() const override;
    CliResult run(const QStringList& args) override;

private:
    CodeReviewService* m_codeReviewService; // 不持有所有权
    SettingsService*   m_settingsService;   // 不持有所有权

    // 解析 --repo 参数，默认返回 "."
    QString extractRepoPath(const QStringList& args) const;

    // 解析 --staged 标志
    bool extractStaged(const QStringList& args) const;

    // 解析 --strict 标志
    bool extractStrict(const QStringList& args) const;

    // 将 vector<ReviewFinding> 格式化为 JSON 输出
    QString formatOutput(const std::vector<ReviewFinding>& findings,
                         bool strictMode) const;
};

#endif // REVIEWCOMMAND_H
