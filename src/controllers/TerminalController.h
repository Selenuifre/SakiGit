#ifndef TERMINALCONTROLLER_H
#define TERMINALCONTROLLER_H

#include "BaseController.h"
#include "infrastructure/result.h"

#include <QString>

class TerminalOutputModel;
class TerminalService;

class TerminalController : public BaseController
{
    Q_OBJECT

public:
    explicit TerminalController(TerminalService* terminalService,
                                 TerminalOutputModel* outputModel,
                                 QObject* parent = nullptr);

    // 执行用户输入的 Git 命令
    Result<QString> executeCommand(const QString& repoPath, const QString& command);

    // 清空终端
    void clear();

    TerminalOutputModel* outputModel() const;

signals:
    void commandFinished(bool success, const QString& output);

private:
    TerminalService* m_terminalService;
    TerminalOutputModel* m_outputModel;
};

#endif // TERMINALCONTROLLER_H
