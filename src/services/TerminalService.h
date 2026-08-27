#ifndef TERMINALSERVICE_H
#define TERMINALSERVICE_H

#include "BaseService.h"
#include "domain/CommandLogEntry.h"
#include "infrastructure/result.h"

#include <QString>

class GitService;
class TerminalOutputModel;

// 终端命令服务：接收 GUI 操作自动产生的命令日志 + 执行用户手动输入的命令。
class TerminalService : public BaseService
{
public:
    explicit TerminalService(GitService* gitService);

    // 由 GitCommandExecutor 回调触发（GUI 操作产生）
    void onCommandExecuted(const QString& commandLine, const QString& workingDir,
                           int exitCode, const QString& output);

    // 执行用户在终端窗口手动输入的命令（直接调用 GitCommandExecutor）
    Result<QString> executeUserCommand(const QString& repositoryPath,
                                        const QString& commandLine);

    void setOutputModel(TerminalOutputModel* model);
    TerminalOutputModel* outputModel() const;

    void clear();

private:
    TerminalOutputModel* m_outputModel = nullptr;
};

#endif // TERMINALSERVICE_H
