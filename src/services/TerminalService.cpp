#include "TerminalService.h"
#include "services/gitservice.h"
#include "models/TerminalOutputModel.h"

#include <QDateTime>
#include <QStringList>

TerminalService::TerminalService(GitService* gitService)
    : BaseService(gitService)
{
}

void TerminalService::onCommandExecuted(const QString& commandLine,
                                         const QString& workingDir,
                                         int exitCode,
                                         const QString& output)
{
    if (!m_outputModel)
        return;

    CommandLogEntry entry(commandLine, workingDir, exitCode, output, false);
    m_outputModel->appendEntry(entry);
}

Result<QString> TerminalService::executeUserCommand(const QString& repositoryPath,
                                                      const QString& commandLine)
{
    if (!isInitialized())
        return serviceNotInitialized<QString>();

    const QString trimmed = commandLine.trimmed();
    if (trimmed.isEmpty())
        return Result<QString>::failure(QStringLiteral("Command is empty."));

    // Parse "git <args>..." → extract args after "git"
    QStringList args;
    const QStringList parts = trimmed.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (parts.isEmpty())
        return Result<QString>::failure(QStringLiteral("Invalid command."));

    // Skip "git" prefix if present
    int startIdx = 0;
    if (parts.at(0).toLower() == QStringLiteral("git"))
        startIdx = 1;

    for (int i = startIdx; i < parts.size(); ++i)
        args.append(parts.at(i));

    if (args.isEmpty())
        return Result<QString>::failure(QStringLiteral("No git command specified."));

    // Execute via GitCommandExecutor
    const Result<GitCommandExecutor::CommandResult> result =
        m_gitService->executor().run(args, repositoryPath);

    // Log the user command through normal callback path
    // Note: executor's callback will fire automatically
    CommandLogEntry entry(trimmed, repositoryPath,
        result.isSuccess() ? result.value().exitCode : -1,
        result.isSuccess() ? result.value().standardOutput : result.errorMessage(),
        true);
    if (m_outputModel)
        m_outputModel->appendEntry(entry);

    if (result.isFailure())
        return Result<QString>::failure(result.errorMessage());

    return Result<QString>::success(result.value().standardOutput);
}

void TerminalService::setOutputModel(TerminalOutputModel* model)
{
    m_outputModel = model;
}

TerminalOutputModel* TerminalService::outputModel() const
{
    return m_outputModel;
}

void TerminalService::clear()
{
    if (m_outputModel)
        m_outputModel->clear();
}
