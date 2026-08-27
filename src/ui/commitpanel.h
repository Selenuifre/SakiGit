#ifndef COMMITPANEL_H
#define COMMITPANEL_H

#include <QWidget>

class QLabel;
class QPlainTextEdit;
class QPushButton;

// 提交面板——提交消息输入框 + AI 生成按钮 + 提交按钮。
// 规范接口：setMessage(text), clear(), 信号 commitRequested(message), messageChanged(text)
class CommitPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CommitPanel(QWidget* parent = nullptr);

    // 预填提交消息
    void setMessage(const QString& message);

    // 获取当前消息文本
    QString message() const;

    // 清空消息输入框
    void clear();

    // 设置提交按钮是否可用
    void setCommitEnabled(bool enabled);

    // 设置输入框是否只读
    void setReadOnly(bool readOnly);

    // AI 生成按钮状态控制（Phase 5）
    void setAIEnabled(bool enabled);
    void setGenerating(bool generating);
    void setAIStatusMessage(const QString& message, bool isError = false);
    void clearAIStatus();

signals:
    // 用户点击"提交"按钮
    void commitRequested(const QString& message);

    // 提交消息文本发生变化
    void messageChanged(const QString& text);

    // 用户点击"AI Generate"按钮（Phase 5）
    void generateAIRequested();

    // 用户点击"Review"按钮（Phase 6 - AI Code Review）
    void reviewRequested();

private slots:
    void handleCommitClicked();
    void handleTextChanged();
    void handleAIGenerateClicked();
    void handleReviewClicked();

private:
    QPlainTextEdit* m_commitMessageEdit;
    QPushButton*    m_commitButton;
    QPushButton*    m_aiGenerateButton;
    QPushButton*    m_reviewButton;
    QLabel*         m_aiStatusLabel;
};

#endif // COMMITPANEL_H
