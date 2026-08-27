#include "AICommitController.h"

#include "models/CommitMessageSuggestionModel.h"
#include "services/CommitMessageService.h"

#include <QTimer>

AICommitController::AICommitController(CommitMessageService* service,
                                       CommitMessageSuggestionModel* model,
                                       QObject* parent)
    : BaseController(parent)
    , m_service(service)
    , m_model(model)
{
}

void AICommitController::generate(const QString& repoPath)
{
    if (!m_service) {
        emit generationFailed(QStringLiteral("Commit message service is not available."));
        return;
    }

    if (repoPath.trimmed().isEmpty()) {
        emit generationFailed(QStringLiteral("No repository is currently selected."));
        return;
    }

    emit generationStarted();

    // 使用 QTimer::singleShot 延迟到事件循环之后执行，
    // 让 UI 有时间更新 loading 状态
    QTimer::singleShot(50, this, [this, repoPath]() {
        const auto result = m_service->generateFromStagedDiff(repoPath);

        if (result.isFailure()) {
            m_currentSuggestion = CommitMessageSuggestion();
            emit generationFailed(result.errorMessage());
            emit errorOccurred(QStringLiteral("AICommit:generate"), result.errorMessage());
            emit generationFinished();
            return;
        }

        m_currentSuggestion = result.value();

        if (m_model) {
            m_model->clear();
            m_model->addSuggestion(m_currentSuggestion);
        }

        emit generationSucceeded(m_currentSuggestion);
        emit operationFinished(QStringLiteral("AICommit:generate"), true, QString());
        emit generationFinished();
    });
}

QString AICommitController::applySuggestion(int index) const
{
    if (m_model) {
        const CommitMessageSuggestion suggestion = m_model->suggestionAt(index);
        if (suggestion.isValid() && !suggestion.fullMessage().isEmpty()) {
            return suggestion.fullMessage();
        }
    }

    if (m_currentSuggestion.isValid() && !m_currentSuggestion.fullMessage().isEmpty()) {
        return m_currentSuggestion.fullMessage();
    }

    return QString();
}

void AICommitController::checkStagedChanges(const QString& repoPath)
{
    if (!m_service) {
        emit stagedChangesAvailable(false);
        return;
    }

    // 通过尝试获取 staged diff 来检查是否有暂存
    // 直接调用生成但不发送给 AI —— 在 CommitMessageService 内部检查
    // 这里用简单的启发式：尝试生成给一个测试路径
    // 更好的方式是让 GitService 直接提供检查方法
    // 暂时先总是 emit true，由用户点击时懒检查
    Q_UNUSED(repoPath);
    emit stagedChangesAvailable(true);
}

bool AICommitController::hasCurrentSuggestion() const
{
    return m_currentSuggestion.isValid()
           && !m_currentSuggestion.fullMessage().isEmpty();
}

CommitMessageSuggestion AICommitController::currentSuggestion() const
{
    return m_currentSuggestion;
}

CommitMessageSuggestionModel* AICommitController::model() const
{
    return m_model;
}
