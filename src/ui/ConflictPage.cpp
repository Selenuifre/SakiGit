#include "ConflictPage.h"
#include "ConflictResolver.h"
#include "PageHeaderWidget.h"
#include "uiutils.h"

#include <QAbstractItemModel>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

ConflictPage::ConflictPage(QWidget* parent)
    : QWidget(parent),
      m_headerWidget(nullptr),
      m_conflictListView(nullptr),
      m_conflictResolver(nullptr),
      m_splitter(nullptr)
{
    setupUi();
}

void ConflictPage::setModel(QAbstractItemModel* model)
{
    m_conflictListView->setModel(model);
}

QAbstractItemModel* ConflictPage::model() const
{
    return m_conflictListView->model();
}

QListView* ConflictPage::listView() const
{
    return m_conflictListView;
}

void ConflictPage::showConflictFile(const QString& repoPath, const QString& filePath)
{
    if (m_conflictResolver) {
        m_conflictResolver->loadFile(repoPath, filePath);
    }
}

ConflictResolver* ConflictPage::conflictResolver() const
{
    return m_conflictResolver;
}

QPushButton* ConflictPage::refreshButton() const
{
    return m_headerWidget ? m_headerWidget->refreshButton() : nullptr;
}

void ConflictPage::setConflictStatus(const QString& text)
{
    if (m_headerWidget) {
        m_headerWidget->setStatusText(text);
    }
}

void ConflictPage::onFileActivated(const QModelIndex& index)
{
    if (!index.isValid()) return;
    const QString filePath = index.data(Qt::DisplayRole).toString();
    if (!filePath.isEmpty()) {
        emit conflictFileSelected(filePath);
    }
}

void ConflictPage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    // 标题行（含冲突状态标签）
    m_headerWidget = new PageHeaderWidget(tr("Conflicts"), true, this);
    layout->addWidget(m_headerWidget);

    // 左右分栏
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setChildrenCollapsible(false);

    // 左侧：冲突文件列表
    auto* leftPanel = new QWidget(m_splitter);
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    auto* listLabel = new QLabel(tr("Conflict Files"), leftPanel);
    listLabel->setObjectName(QStringLiteral("sectionLabel"));
    m_conflictListView = createListView(leftPanel, QStringLiteral("conflictListView"));

    leftLayout->addWidget(listLabel);
    leftLayout->addWidget(m_conflictListView, 1);

    // 右侧：冲突编辑器（复用 ConflictResolver）
    m_conflictResolver = new ConflictResolver(m_splitter);

    m_splitter->addWidget(leftPanel);
    m_splitter->addWidget(m_conflictResolver);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    m_splitter->setSizes({260, 650});

    layout->addWidget(m_splitter, 1);

    // 信号
    connect(m_conflictListView, &QListView::activated,
            this, &ConflictPage::onFileActivated);
}
