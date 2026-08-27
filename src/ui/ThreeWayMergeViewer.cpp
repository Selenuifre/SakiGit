#include "ThreeWayMergeViewer.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QVBoxLayout>

ThreeWayMergeViewer::ThreeWayMergeViewer(QWidget* parent)
    : QWidget(parent),
      m_baseView(nullptr),
      m_oursView(nullptr),
      m_theirsView(nullptr),
      m_baseLabel(nullptr),
      m_oursLabel(nullptr),
      m_theirsLabel(nullptr)
{
    setupUi();
}

void ThreeWayMergeViewer::loadContent(const MergeFileContent& content)
{
    m_baseView->setPlainText(content.baseContent.isEmpty()
        ? tr("(no base version available)") : content.baseContent);
    m_oursView->setPlainText(content.oursContent.isEmpty()
        ? tr("(no ours version available)") : content.oursContent);
    m_theirsView->setPlainText(content.theirsContent.isEmpty()
        ? tr("(no theirs version available)") : content.theirsContent);
}

void ThreeWayMergeViewer::clear()
{
    m_baseView->clear();
    m_oursView->clear();
    m_theirsView->clear();
}

void ThreeWayMergeViewer::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);

    auto* titleLabel = new QLabel(tr("Three-Way Merge View"), this);
    QFont f = titleLabel->font();
    f.setBold(true);
    titleLabel->setFont(f);
    mainLayout->addWidget(titleLabel);

    auto* splitter = new QSplitter(Qt::Horizontal, this);

    // Base 列
    auto* basePanel = new QWidget(splitter);
    auto* baseLayout = new QVBoxLayout(basePanel);
    baseLayout->setContentsMargins(0, 0, 0, 0);
    m_baseLabel = new QLabel(tr("Base (ancestor)"), basePanel);
    m_baseLabel->setStyleSheet(QStringLiteral(
        "background: #E8E8E8; padding: 2px 4px; font-weight: bold;"));
    m_baseView = new QPlainTextEdit(basePanel);
    m_baseView->setReadOnly(true);
    m_baseView->setFont(QFont(QStringLiteral("Cascadia Code,Consolas,SF Mono,Menlo,DejaVu Sans Mono,monospace"), 9));
    baseLayout->addWidget(m_baseLabel);
    baseLayout->addWidget(m_baseView, 1);

    // Ours 列
    auto* oursPanel = new QWidget(splitter);
    auto* oursLayout = new QVBoxLayout(oursPanel);
    oursLayout->setContentsMargins(0, 0, 0, 0);
    m_oursLabel = new QLabel(tr("Ours (HEAD)"), oursPanel);
    m_oursLabel->setStyleSheet(QStringLiteral(
        "background: #FFD6D6; padding: 2px 4px; font-weight: bold;"));
    m_oursView = new QPlainTextEdit(oursPanel);
    m_oursView->setReadOnly(true);
    m_oursView->setFont(QFont(QStringLiteral("Cascadia Code,Consolas,SF Mono,Menlo,DejaVu Sans Mono,monospace"), 9));
    oursLayout->addWidget(m_oursLabel);
    oursLayout->addWidget(m_oursView, 1);

    // Theirs 列
    auto* theirsPanel = new QWidget(splitter);
    auto* theirsLayout = new QVBoxLayout(theirsPanel);
    theirsLayout->setContentsMargins(0, 0, 0, 0);
    m_theirsLabel = new QLabel(tr("Theirs (incoming)"), theirsPanel);
    m_theirsLabel->setStyleSheet(QStringLiteral(
        "background: #D6D6FF; padding: 2px 4px; font-weight: bold;"));
    m_theirsView = new QPlainTextEdit(theirsPanel);
    m_theirsView->setReadOnly(true);
    m_theirsView->setFont(QFont(QStringLiteral("Cascadia Code,Consolas,SF Mono,Menlo,DejaVu Sans Mono,monospace"), 9));
    theirsLayout->addWidget(m_theirsLabel);
    theirsLayout->addWidget(m_theirsView, 1);

    splitter->addWidget(basePanel);
    splitter->addWidget(oursPanel);
    splitter->addWidget(theirsPanel);
    splitter->setSizes({250, 250, 250});

    mainLayout->addWidget(splitter, 1);
}
