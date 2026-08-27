#include "TerminalController.h"
#include "models/TerminalOutputModel.h"
#include "services/TerminalService.h"

TerminalController::TerminalController(TerminalService* terminalService,
                                         TerminalOutputModel* outputModel,
                                         QObject* parent)
    : BaseController(parent),
      m_terminalService(terminalService),
      m_outputModel(outputModel)
{
}

Result<QString> TerminalController::executeCommand(const QString& repoPath,
                                                     const QString& command)
{
    if (!m_terminalService) {
        const QString msg = QStringLiteral("Terminal service is not available.");
        emit errorOccurred(QStringLiteral("executeCommand"), msg);
        return Result<QString>::failure(msg);
    }

    const Result<QString> result = m_terminalService->executeUserCommand(repoPath, command);

    if (result.isFailure()) {
        emit commandFinished(false, result.errorMessage());
        emit errorOccurred(QStringLiteral("executeCommand"), result.errorMessage());
        return result;
    }

    emit commandFinished(true, result.value());
    return result;
}

void TerminalController::clear()
{
    if (m_outputModel)
        m_outputModel->clear();
}

TerminalOutputModel* TerminalController::outputModel() const
{
    return m_outputModel;
}
