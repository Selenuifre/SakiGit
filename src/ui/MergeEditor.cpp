#include "MergeEditor.h"
#include "ConflictResolver.h"

#include <QVBoxLayout>

MergeEditor::MergeEditor(QWidget* parent)
    : QWidget(parent),
      m_conflictResolver(nullptr)
{
    setupUi();
}

void MergeEditor::loadFile(const QString& repoPath, const QString& filePath)
{
    if (m_conflictResolver) {
        m_conflictResolver->loadFile(repoPath, filePath);
    }
}

void MergeEditor::clear()
{
    if (m_conflictResolver) {
        m_conflictResolver->clear();
    }
}

ConflictResolver* MergeEditor::conflictResolver() const
{
    return m_conflictResolver;
}

void MergeEditor::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_conflictResolver = new ConflictResolver(this);
    layout->addWidget(m_conflictResolver);

    // 转发 markResolvedRequested 信号
    connect(m_conflictResolver, &ConflictResolver::markResolvedRequested,
            this, &MergeEditor::markResolvedRequested);
}
