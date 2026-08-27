#include "TerminalWidget.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QModelIndex>

TerminalWidget::TerminalWidget(QWidget* parent)
    : QWidget(parent),
      m_outputView(nullptr),
      m_inputEdit(nullptr),
      m_executeButton(nullptr),
      m_clearButton(nullptr),
      m_model(nullptr)
{
    setupUi();
}

void TerminalWidget::setModel(QAbstractItemModel* model)
{
    // Disconnect old model
    if (m_model) {
        QObject::disconnect(m_model, &QAbstractItemModel::rowsInserted,
                            this, &TerminalWidget::onRowsInserted);
    }

    m_model = model;

    // Connect new model: whenever rows are inserted, update the output view
    if (m_model) {
        QObject::connect(m_model, &QAbstractItemModel::rowsInserted,
                         this, &TerminalWidget::onRowsInserted);
    }
}

QAbstractItemModel* TerminalWidget::model() const
{
    return m_model;
}

void TerminalWidget::appendOutput(const QString& line)
{
    m_outputView->appendPlainText(line);
    QScrollBar* bar = m_outputView->verticalScrollBar();
    bar->setValue(bar->maximum());
}

void TerminalWidget::clearOutput()
{
    m_outputView->clear();
}

void TerminalWidget::setInputPlaceholder(const QString& text)
{
    if (m_inputEdit)
        m_inputEdit->setPlaceholderText(text);
}

QString TerminalWidget::inputText() const
{
    return m_inputEdit ? m_inputEdit->text() : QString();
}

void TerminalWidget::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    // 标题行
    auto* headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins(8, 6, 8, 2);
    auto* titleLabel = new QLabel(tr("Command Terminal"), this);
    titleLabel->setObjectName(QStringLiteral("terminalTitleLabel"));
    m_clearButton = new QPushButton(tr("Clear"), this);
    m_clearButton->setObjectName(QStringLiteral("terminalClearBtn"));
    m_clearButton->setFixedWidth(70);
    headerLayout->addWidget(titleLabel, 1);
    headerLayout->addWidget(m_clearButton);

    // 输出区
    m_outputView = new QPlainTextEdit(this);
    m_outputView->setObjectName(QStringLiteral("terminalOutputView"));
    m_outputView->setReadOnly(true);
    m_outputView->setUndoRedoEnabled(false);
    m_outputView->setFont(QFont(QStringLiteral("Cascadia Code,Consolas,SF Mono,Menlo,DejaVu Sans Mono,monospace"), 10));

    // 输入区
    auto* inputLayout = new QHBoxLayout;
    inputLayout->setContentsMargins(8, 2, 8, 6);

    auto* promptLabel = new QLabel(QStringLiteral(">"), this);
    promptLabel->setFont(QFont(QStringLiteral("Cascadia Code,Consolas,SF Mono,Menlo,DejaVu Sans Mono,monospace"), 11));
    promptLabel->setStyleSheet(QStringLiteral("color: #539BF5; font-weight: bold;"));

    m_inputEdit = new QLineEdit(this);
    m_inputEdit->setObjectName(QStringLiteral("terminalInputEdit"));
    m_inputEdit->setPlaceholderText(tr("Enter git command (e.g. git log --oneline)"));
    m_inputEdit->setFont(QFont(QStringLiteral("Cascadia Code,Consolas,SF Mono,Menlo,DejaVu Sans Mono,monospace"), 10));

    m_executeButton = new QPushButton(tr("Execute"), this);
    m_executeButton->setObjectName(QStringLiteral("terminalExecuteBtn"));
    m_executeButton->setFixedWidth(80);

    inputLayout->addWidget(promptLabel);
    inputLayout->addWidget(m_inputEdit, 1);
    inputLayout->addWidget(m_executeButton);

    layout->addLayout(headerLayout);
    layout->addWidget(m_outputView, 1);
    layout->addLayout(inputLayout);

    // 信号连接
    connect(m_executeButton, &QPushButton::clicked,
            this, &TerminalWidget::handleExecute);
    connect(m_inputEdit, &QLineEdit::returnPressed,
            this, &TerminalWidget::handleExecute);
    connect(m_clearButton, &QPushButton::clicked,
            this, &TerminalWidget::clearOutput);
    connect(m_clearButton, &QPushButton::clicked,
            this, &TerminalWidget::clearRequested);
}

void TerminalWidget::handleExecute()
{
    const QString cmd = m_inputEdit->text().trimmed();
    if (cmd.isEmpty())
        return;

    emit commandEntered(cmd);
    m_inputEdit->clear();
}

void TerminalWidget::onRowsInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent)
    if (!m_model)
        return;

    // TerminalOutputModel roles: DisplayLineRole=Qt::UserRole+1, OutputRole=Qt::UserRole+2, ExitCodeRole=Qt::UserRole+3
    static const int OutputRole = Qt::UserRole + 2;
    static const int ExitCodeRole = Qt::UserRole + 3;

    for (int row = first; row <= last; ++row) {
        const QModelIndex idx = m_model->index(row, 0);

        // 命令行时间戳行
        const QString cmdLine = m_model->data(idx, Qt::DisplayRole).toString();
        appendOutput(cmdLine);

        // 执行结果输出
        const QString output = m_model->data(idx, OutputRole).toString();
        if (!output.isEmpty()) {
            const QStringList lines = output.split(QLatin1Char('\n'));
            for (const QString& line : lines)
                appendOutput(QStringLiteral("  ") + line);
        }

        // 错误退出码
        const int exitCode = m_model->data(idx, ExitCodeRole).toInt();
        if (exitCode != 0)
            appendOutput(QStringLiteral("  [exit code: %1]").arg(exitCode));
    }
}
