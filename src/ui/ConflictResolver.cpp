#include "ConflictResolver.h"

#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QStyle>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextStream>
#include <QToolBar>
#include <QVBoxLayout>

ConflictResolver::ConflictResolver(QWidget* parent)
    : QWidget(parent),
      m_filePathLabel(nullptr),
      m_statusLabel(nullptr),
      m_editor(nullptr),
      m_toolBar(nullptr),
      m_acceptOursButton(nullptr),
      m_acceptTheirsButton(nullptr),
      m_saveButton(nullptr),
      m_markResolvedButton(nullptr),
      m_placeholderWidget(nullptr)
{
    setupUi();
}

void ConflictResolver::loadFile(const QString& repoPath, const QString& filePath)
{
    m_repoPath = repoPath;
    m_filePath = filePath;

    const QString fullPath = repoPath + QStringLiteral("/") + filePath;
    QFile file(fullPath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_editor->clear();
        m_editor->setEnabled(false);
        m_filePathLabel->setText(tr("Cannot read: %1").arg(filePath));
        m_statusLabel->setText(tr("File not accessible"));
        m_statusLabel->setStyleSheet(QStringLiteral("color: #DA3633; font-weight: bold;"));
        return;
    }

    QTextStream stream(&file);
    const QString content = stream.readAll();
    file.close();

    m_editor->setPlainText(content);
    m_editor->setEnabled(true);
    m_filePathLabel->setText(filePath);
    m_statusLabel->setText(tr("Conflict — edit to resolve"));
    m_statusLabel->setStyleSheet(QStringLiteral("color: #DA3633; font-weight: bold;"));

    // 高亮冲突区域
    applyConflictHighlighting();

    m_placeholderWidget->setVisible(false);
    m_editor->setVisible(true);
}

void ConflictResolver::clear()
{
    m_editor->clear();
    m_editor->setEnabled(false);
    m_filePathLabel->clear();
    m_statusLabel->clear();
    m_repoPath.clear();
    m_filePath.clear();

    m_placeholderWidget->setVisible(true);
    m_editor->setVisible(false);
}

QString ConflictResolver::currentFilePath() const
{
    return m_filePath;
}

void ConflictResolver::setPlaceholderText(const QString& text)
{
    // placeholder is handled via m_placeholderWidget visibility
    Q_UNUSED(text)
}

void ConflictResolver::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(6);

    // 顶部信息栏
    auto* headerLayout = new QHBoxLayout;

    m_filePathLabel = new QLabel(this);
    m_filePathLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QFont headerFont = m_filePathLabel->font();
    headerFont.setBold(true);
    m_filePathLabel->setFont(headerFont);

    m_statusLabel = new QLabel(this);

    headerLayout->addWidget(m_filePathLabel, 1);
    headerLayout->addWidget(m_statusLabel);

    // 工具栏
    m_toolBar = new QToolBar(this);
    m_toolBar->setIconSize(QSize(16, 16));

    m_acceptOursButton = new QPushButton(tr("Accept Ours (HEAD)"), this);
    m_acceptOursButton->setToolTip(tr("Replace all conflicts with the current branch (HEAD) version"));
    m_acceptOursButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #1F6FEB; color: white; padding: 4px 12px; border-radius: 3px; }"
        "QPushButton:hover { background: #388BFD; }"));

    m_acceptTheirsButton = new QPushButton(tr("Accept Theirs (Incoming)"), this);
    m_acceptTheirsButton->setToolTip(tr("Replace all conflicts with the incoming branch version"));
    m_acceptTheirsButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #DA3633; color: white; padding: 4px 12px; border-radius: 3px; }"
        "QPushButton:hover { background: #B62324; }"));

    m_saveButton = new QPushButton(tr("Save"), this);
    m_saveButton->setToolTip(tr("Save the current content to the file"));
    m_saveButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #2DA44E; color: white; padding: 4px 12px; border-radius: 3px; }"
        "QPushButton:hover { background: #347D39; }"));

    m_markResolvedButton = new QPushButton(tr("Mark Resolved"), this);
    m_markResolvedButton->setToolTip(tr("Save and git add this file to mark it as resolved"));
    m_markResolvedButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #F0883E; color: white; padding: 4px 12px; border-radius: 3px; }"
        "QPushButton:hover { background: #D27D2D; }"));

    m_toolBar->addWidget(m_acceptOursButton);
    m_toolBar->addWidget(m_acceptTheirsButton);
    m_toolBar->addSeparator();
    m_toolBar->addWidget(m_saveButton);
    m_toolBar->addWidget(m_markResolvedButton);

    // 编辑器
    m_editor = new QPlainTextEdit(this);
    m_editor->setObjectName(QStringLiteral("conflictEditor"));
    m_editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_editor->setTabStopDistance(40);
    m_editor->setVisible(false);
    QFont editorFont = QFont(QStringLiteral("Cascadia Code,Consolas,SF Mono,Menlo,DejaVu Sans Mono,monospace"));
    editorFont.setPointSize(10);
    m_editor->setFont(editorFont);

    // 占位提示
    m_placeholderWidget = new QWidget(this);
    auto* placeholderLayout = new QVBoxLayout(m_placeholderWidget);
    placeholderLayout->setAlignment(Qt::AlignCenter);
    auto* placeholderLabel = new QLabel(tr("Select a conflicted file to resolve"), m_placeholderWidget);
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLabel->setStyleSheet(QStringLiteral("color: #636E7B; font-size: 14px;"));
    placeholderLayout->addWidget(placeholderLabel);

    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(m_toolBar);
    mainLayout->addWidget(m_editor, 1);
    mainLayout->addWidget(m_placeholderWidget, 1);

    // 信号连接
    connect(m_acceptOursButton, &QPushButton::clicked,
            this, &ConflictResolver::onAcceptOurs);
    connect(m_acceptTheirsButton, &QPushButton::clicked,
            this, &ConflictResolver::onAcceptTheirs);
    connect(m_saveButton, &QPushButton::clicked,
            this, &ConflictResolver::onSave);
    connect(m_markResolvedButton, &QPushButton::clicked,
            this, &ConflictResolver::onMarkResolved);
}

void ConflictResolver::applyConflictHighlighting()
{
    // 清除已有格式
    QTextCursor cursor(m_editor->document());
    cursor.select(QTextCursor::Document);
    QTextCharFormat defaultFormat;
    defaultFormat.setBackground(QColor(0xF0, 0xFF, 0xF0)); // 淡绿色背景（非冲突行）
    cursor.mergeCharFormat(defaultFormat);

    // 遍历每一行，查找冲突标记并设置颜色
    enum class ConflictZone { None, Ours, Theirs };
    ConflictZone zone = ConflictZone::None;

    QTextBlock block = m_editor->document()->firstBlock();
    while (block.isValid()) {
        const QString line = block.text();

        QTextCharFormat format;
        if (line.startsWith(QStringLiteral("<<<<<<<"))) {
            zone = ConflictZone::Ours;
            format.setBackground(QColor(0xFF, 0xCC, 0xCC)); // 红色 — 冲突分隔符
        } else if (line.startsWith(QStringLiteral("=======")) && zone == ConflictZone::Ours) {
            zone = ConflictZone::Theirs;
            format.setBackground(QColor(0xFF, 0xCC, 0xCC)); // 红色 — 冲突分隔符
        } else if (line.startsWith(QStringLiteral(">>>>>>>"))) {
            zone = ConflictZone::None;
            format.setBackground(QColor(0xFF, 0xCC, 0xCC)); // 红色 — 冲突分隔符
        } else if (zone == ConflictZone::Ours) {
            format.setBackground(QColor(0xFF, 0xD6, 0xD6)); // 浅红色 — ours 区域
        } else if (zone == ConflictZone::Theirs) {
            format.setBackground(QColor(0xD6, 0xD6, 0xFF)); // 浅蓝色 — theirs 区域
        } else {
            format.setBackground(QColor(0xF0, 0xFF, 0xF0)); // 淡绿色 — 非冲突行
        }

        QTextCursor blockCursor(block);
        blockCursor.movePosition(QTextCursor::StartOfBlock);
        blockCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        blockCursor.mergeCharFormat(format);

        block = block.next();
    }
}

void ConflictResolver::onAcceptOurs()
{
    replaceAllWithOurs();
    applyConflictHighlighting();

    m_statusLabel->setText(tr("Accepted Ours — save to keep changes"));
    m_statusLabel->setStyleSheet(QStringLiteral("color: #539BF5; font-weight: bold;"));
}

void ConflictResolver::onAcceptTheirs()
{
    replaceAllWithTheirs();
    applyConflictHighlighting();

    m_statusLabel->setText(tr("Accepted Theirs — save to keep changes"));
    m_statusLabel->setStyleSheet(QStringLiteral("color: #DA3633; font-weight: bold;"));
}

void ConflictResolver::onSave()
{
    if (m_repoPath.isEmpty() || m_filePath.isEmpty()) {
        return;
    }

    const QString fullPath = m_repoPath + QStringLiteral("/") + m_filePath;
    QFile file(fullPath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        m_statusLabel->setText(tr("Save failed: cannot write to file"));
        m_statusLabel->setStyleSheet(QStringLiteral("color: #DA3633; font-weight: bold;"));
        return;
    }

    QTextStream stream(&file);
    stream << m_editor->toPlainText();
    file.close();

    m_statusLabel->setText(tr("Saved — resolve remaining conflicts then Mark Resolved"));
    m_statusLabel->setStyleSheet(QStringLiteral("color: #2DA44E; font-weight: bold;"));

    // 重新高亮（用户可能手动修改了冲突标记）
    applyConflictHighlighting();
}

void ConflictResolver::onMarkResolved()
{
    if (m_repoPath.isEmpty() || m_filePath.isEmpty()) {
        return;
    }

    // 先保存
    onSave();

    // 通知外部执行 git add
    emit markResolvedRequested(m_repoPath, m_filePath);

    m_statusLabel->setText(tr("Marked as resolved"));
    m_statusLabel->setStyleSheet(QStringLiteral("color: #2DA44E; font-weight: bold;"));
}

QString ConflictResolver::editorContent() const
{
    return m_editor->toPlainText();
}

void ConflictResolver::replaceAllWithOurs()
{
    const QString content = m_editor->toPlainText();
    QStringList lines = content.split(QLatin1Char('\n'));

    QStringList result;
    enum class Zone { None, Ours, Theirs };
    Zone zone = Zone::None;

    for (const QString& line : lines) {
        if (line.startsWith(QStringLiteral("<<<<<<<"))) {
            zone = Zone::Ours;
            continue; // 跳过标记行
        }

        if (zone == Zone::Ours && line.startsWith(QStringLiteral("======="))) {
            zone = Zone::Theirs;
            continue; // 跳过标记行，开始跳过 theirs
        }

        if (zone == Zone::Theirs && line.startsWith(QStringLiteral(">>>>>>>"))) {
            zone = Zone::None;
            continue; // 跳过标记行
        }

        if (zone == Zone::Ours) {
            result.append(line); // 保留 ours 行
        } else if (zone == Zone::None) {
            result.append(line); // 保留非冲突行
        }
        // zone == Theirs: 跳过（丢弃 theirs 行）
    }

    m_editor->setPlainText(result.join(QLatin1Char('\n')));
}

void ConflictResolver::replaceAllWithTheirs()
{
    const QString content = m_editor->toPlainText();
    QStringList lines = content.split(QLatin1Char('\n'));

    QStringList result;
    enum class Zone { None, Ours, Theirs };
    Zone zone = Zone::None;

    for (const QString& line : lines) {
        if (line.startsWith(QStringLiteral("<<<<<<<"))) {
            zone = Zone::Ours;
            continue; // 跳过标记行
        }

        if (zone == Zone::Ours && line.startsWith(QStringLiteral("======="))) {
            zone = Zone::Theirs;
            continue; // 跳过标记行
        }

        if (zone == Zone::Theirs && line.startsWith(QStringLiteral(">>>>>>>"))) {
            zone = Zone::None;
            continue; // 跳过标记行
        }

        if (zone == Zone::Theirs) {
            result.append(line); // 保留 theirs 行
        } else if (zone == Zone::None) {
            result.append(line); // 保留非冲突行
        }
        // zone == Ours: 跳过（丢弃 ours 行）
    }

    m_editor->setPlainText(result.join(QLatin1Char('\n')));
}
