#include "commitpanel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

CommitPanel::CommitPanel(QWidget* parent)
    : QWidget(parent),
    m_commitMessageEdit(nullptr),
    m_commitButton(nullptr),
    m_aiGenerateButton(nullptr),
    m_reviewButton(nullptr),
    m_aiStatusLabel(nullptr)
{
    setObjectName(QStringLiteral("commitPanel"));
    setMinimumHeight(130);
    setMaximumHeight(240);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    // 标题行：label + AI 生成按钮 + 提交按钮
    auto* headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins(0, 0, 0, 0);

    auto* titleLabel = new QLabel(tr("Commit Message"), this);
    titleLabel->setObjectName(QStringLiteral("commitPanelTitle"));

    // AI 生成按钮（Phase 5）
    m_aiGenerateButton = new QPushButton(tr("AI Generate"), this);
    m_aiGenerateButton->setObjectName(QStringLiteral("aiGenerateButton"));
    m_aiGenerateButton->setToolTip(
        tr("Generate a commit message from staged changes using AI"));
    m_aiGenerateButton->setMinimumWidth(110);

    // Code Review 按钮（Phase 6）
    m_reviewButton = new QPushButton(tr("Review"), this);
    m_reviewButton->setObjectName(QStringLiteral("reviewButton"));
    m_reviewButton->setToolTip(
        tr("Review staged changes using AI before committing"));
    m_reviewButton->setMinimumWidth(80);

    m_commitButton = new QPushButton(
        QIcon(QStringLiteral(":/icons/action_commit.svg")),
        tr("Commit"), this);
    m_commitButton->setObjectName(QStringLiteral("commitButton"));
    m_commitButton->setEnabled(false);

    headerLayout->addWidget(titleLabel, 1);
    headerLayout->addWidget(m_reviewButton);
    headerLayout->addWidget(m_aiGenerateButton);
    headerLayout->addWidget(m_commitButton);

    // AI 状态标签
    m_aiStatusLabel = new QLabel(this);
    m_aiStatusLabel->setObjectName(QStringLiteral("aiStatusLabel"));
    m_aiStatusLabel->setVisible(false);
    m_aiStatusLabel->setWordWrap(true);

    // 消息输入框
    m_commitMessageEdit = new QPlainTextEdit(this);
    m_commitMessageEdit->setObjectName(QStringLiteral("commitMessageEdit"));
    m_commitMessageEdit->setPlaceholderText(tr("Enter commit message..."));
    m_commitMessageEdit->setMaximumBlockCount(200);

    layout->addLayout(headerLayout);
    layout->addWidget(m_aiStatusLabel);
    layout->addWidget(m_commitMessageEdit, 1);

    connect(m_commitButton, &QPushButton::clicked,
            this, &CommitPanel::handleCommitClicked);
    connect(m_aiGenerateButton, &QPushButton::clicked,
            this, &CommitPanel::handleAIGenerateClicked);
    connect(m_reviewButton, &QPushButton::clicked,
            this, &CommitPanel::handleReviewClicked);
    connect(m_commitMessageEdit, &QPlainTextEdit::textChanged,
            this, &CommitPanel::handleTextChanged);
}

void CommitPanel::setMessage(const QString& message)
{
    m_commitMessageEdit->setPlainText(message);
}

QString CommitPanel::message() const
{
    return m_commitMessageEdit->toPlainText();
}

void CommitPanel::clear()
{
    m_commitMessageEdit->clear();
    m_commitButton->setEnabled(false);
}

void CommitPanel::setCommitEnabled(bool enabled)
{
    m_commitButton->setEnabled(enabled);
}

void CommitPanel::setReadOnly(bool readOnly)
{
    m_commitMessageEdit->setReadOnly(readOnly);
    m_commitButton->setVisible(!readOnly);
}

// --- AI 生成按钮状态控制（Phase 5） ---

void CommitPanel::setAIEnabled(bool enabled)
{
    if (m_aiGenerateButton) {
        m_aiGenerateButton->setEnabled(enabled);
        m_aiGenerateButton->setVisible(enabled);
    }
}

void CommitPanel::setGenerating(bool generating)
{
    if (m_aiGenerateButton) {
        m_aiGenerateButton->setEnabled(!generating);
        if (generating) {
            m_aiGenerateButton->setText(tr("Generating..."));
        } else {
            m_aiGenerateButton->setText(tr("AI Generate"));
        }
    }
}

void CommitPanel::setAIStatusMessage(const QString& message, bool isError)
{
    if (!m_aiStatusLabel) {
        return;
    }

    m_aiStatusLabel->setText(message);
    m_aiStatusLabel->setVisible(!message.isEmpty());

    if (isError) {
        m_aiStatusLabel->setStyleSheet(
            QStringLiteral("color: #F47067; font-size: 11px; padding: 2px 4px;"));
    } else {
        m_aiStatusLabel->setStyleSheet(
            QStringLiteral("color: #57AB5A; font-size: 11px; padding: 2px 4px;"));
    }
}

void CommitPanel::clearAIStatus()
{
    if (m_aiStatusLabel) {
        m_aiStatusLabel->clear();
        m_aiStatusLabel->setVisible(false);
    }
}

void CommitPanel::handleCommitClicked()
{
    const QString msg = message().trimmed();

    if (msg.isEmpty()) {
        return;
    }

    emit commitRequested(msg);
}

void CommitPanel::handleTextChanged()
{
    const QString& text = m_commitMessageEdit->toPlainText();
    m_commitButton->setEnabled(!text.trimmed().isEmpty());
    emit messageChanged(text);
}

void CommitPanel::handleAIGenerateClicked()
{
    emit generateAIRequested();
}

void CommitPanel::handleReviewClicked()
{
    emit reviewRequested();
}
