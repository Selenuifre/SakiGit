#include "uiutils.h"
#include "changespage.h"
#include "commitpanel.h"
#include "ConflictResolver.h"
#include "diffviewer.h"
#include "ReviewPanel.h"
#include "models/filechangemodel.h"

#include <QAbstractItemModel>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QListView>
#include <QMenu>
#include <QSizePolicy>
#include <QSplitter>
#include <QSortFilterProxyModel>
#include <QStackedWidget>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

namespace {

class ChangeSectionProxyModel : public QSortFilterProxyModel
{
public:
    enum class Section {
        Staged,
        Unstaged
    };

    explicit ChangeSectionProxyModel(Section section, QObject* parent = nullptr)
        : QSortFilterProxyModel(parent),
        m_section(section)
    {
        setDynamicSortFilter(true);
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
    {
        if (!sourceModel()) {
            return false;
        }

        const QModelIndex index = sourceModel()->index(sourceRow,
                                                       FileChangeModel::FilePathColumn,
                                                       sourceParent);
        if (!index.isValid()) {
            return false;
        }

        const bool staged = index.data(FileChangeModel::StagedRole).toBool();
        const bool unstaged = index.data(FileChangeModel::UnstagedRole).toBool();
        const bool partiallyStaged = index.data(FileChangeModel::PartiallyStagedRole).toBool();
        const bool conflict = index.data(FileChangeModel::ConflictRole).toBool();

        if (m_section == Section::Staged) {
            return staged || partiallyStaged;
        }

        return unstaged || partiallyStaged || conflict;
    }

private:
    Section m_section;
};

} // namespace

ChangesPage::ChangesPage(QWidget* parent)
    : QWidget(parent),
    m_stagedListView(nullptr),
    m_unstagedListView(nullptr),
    m_stagedProxyModel(nullptr),
    m_unstagedProxyModel(nullptr),
    m_stageAllButton(nullptr),
    m_unstageAllButton(nullptr),
    m_workArea(nullptr),
    m_rightPanelStack(nullptr),
    m_diffViewer(nullptr),
    m_conflictResolver(nullptr),
    m_reviewPanel(nullptr),
    m_commitPanel(nullptr)
{
    setupUi();
}

void ChangesPage::setModel(QAbstractItemModel* model)
{
    m_stagedProxyModel->setSourceModel(model);
    m_unstagedProxyModel->setSourceModel(model);
}

QAbstractItemModel* ChangesPage::model() const
{
    return m_unstagedProxyModel->sourceModel();
}

void ChangesPage::setDiffModel(QAbstractItemModel* model)
{
    m_diffViewer->setModel(model);
}

DiffViewer* ChangesPage::diffViewer() const
{
    return m_diffViewer;
}

ConflictResolver* ChangesPage::conflictResolver() const
{
    return m_conflictResolver;
}

ReviewPanel* ChangesPage::reviewPanel() const
{
    return m_reviewPanel;
}

CommitPanel* ChangesPage::commitPanel() const
{
    return m_commitPanel;
}

QListView* ChangesPage::listView() const
{
    return m_unstagedListView;
}

void ChangesPage::showConflictFile(const QString& repoPath, const QString& filePath)
{
    if (m_conflictResolver) {
        m_conflictResolver->loadFile(repoPath, filePath);
        m_rightPanelStack->setCurrentIndex(1); // ConflictResolver
    }
}

void ChangesPage::showDiffView()
{
    m_rightPanelStack->setCurrentIndex(0); // DiffViewer
}

void ChangesPage::showReviewPanel()
{
    m_rightPanelStack->setCurrentIndex(2); // ReviewPanel
}

void ChangesPage::setupUi()
{
    auto* pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(10);

    // 上半部分：文件列表 + diff 查看器（左右分栏）
    m_workArea = new QSplitter(Qt::Horizontal, this);
    m_workArea->setChildrenCollapsible(false);

    // 左侧：文件变更列表
    auto* changesPanel = new QWidget(m_workArea);
    auto* changesLayout = new QVBoxLayout(changesPanel);
    changesLayout->setContentsMargins(0, 0, 0, 0);
    changesLayout->setSpacing(2);

    auto* stagedHeader = new QHBoxLayout;
    auto* stagedLabel = new QLabel(tr("Staged Changes"), changesPanel);
    stagedLabel->setObjectName(QStringLiteral("sectionLabel"));
    stagedLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_unstageAllButton = new QToolButton(changesPanel);
    m_unstageAllButton->setToolTip(tr("Unstage all changes"));
    m_unstageAllButton->setIcon(QIcon(QStringLiteral(":/icons/action_unstage_all.svg")));

    stagedHeader->addWidget(stagedLabel, 1);
    stagedHeader->addWidget(m_unstageAllButton);

    m_stagedProxyModel = new ChangeSectionProxyModel(
        ChangeSectionProxyModel::Section::Staged, this);
    m_stagedListView = createListView(changesPanel,
                                            QStringLiteral("stagedFileChangeListView"));
    m_stagedListView->setModel(m_stagedProxyModel);

    auto* unstagedHeader = new QHBoxLayout;
    auto* unstagedLabel = new QLabel(tr("Changes"), changesPanel);
    unstagedLabel->setObjectName(QStringLiteral("sectionLabel"));
    unstagedLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_stageAllButton = new QToolButton(changesPanel);
    m_stageAllButton->setToolTip(tr("Stage all changes"));
    m_stageAllButton->setIcon(QIcon(QStringLiteral(":/icons/action_stage_all.svg")));

    unstagedHeader->addWidget(unstagedLabel, 1);
    unstagedHeader->addWidget(m_stageAllButton);

    m_unstagedProxyModel = new ChangeSectionProxyModel(
        ChangeSectionProxyModel::Section::Unstaged, this);
    m_unstagedListView = createListView(changesPanel,
                                              QStringLiteral("unstagedFileChangeListView"));
    m_unstagedListView->setModel(m_unstagedProxyModel);

    changesLayout->addLayout(stagedHeader);
    changesLayout->addWidget(m_stagedListView, 1);
    changesLayout->addLayout(unstagedHeader);
    changesLayout->addWidget(m_unstagedListView, 1);

    // 右侧：Diff 查看器 + 冲突编辑器（QStackedWidget 切换）
    m_rightPanelStack = new QStackedWidget(m_workArea);

    m_diffViewer = new DiffViewer(m_rightPanelStack);
    m_diffViewer->setPlaceholderText(tr("Select a changed file to view diff"));
    m_rightPanelStack->addWidget(m_diffViewer); // index 0

    m_conflictResolver = new ConflictResolver(m_rightPanelStack);
    m_conflictResolver->setPlaceholderText(tr("Select a conflicted file to resolve"));
    m_rightPanelStack->addWidget(m_conflictResolver); // index 1

    // AI Code Review 面板（Phase 6）
    m_reviewPanel = new ReviewPanel(m_rightPanelStack);
    m_reviewPanel->setPlaceholderText(
        tr("Click 'Review Staged' or 'Review Changes' to start an AI-powered code review."));
    m_rightPanelStack->addWidget(m_reviewPanel); // index 2

    m_rightPanelStack->setCurrentIndex(0); // 默认显示 DiffViewer

    m_workArea->addWidget(changesPanel);
    m_workArea->addWidget(m_rightPanelStack);
    m_workArea->setStretchFactor(0, 0);
    m_workArea->setStretchFactor(1, 1);
    m_workArea->setSizes({320, 650});

    // 下半部分：提交面板（含 AI 生成按钮，Phase 5）
    m_commitPanel = new CommitPanel(this);

    pageLayout->addWidget(m_workArea, 1);
    pageLayout->addWidget(m_commitPanel);

    // ---- 信号连接 ----
    connect(m_stagedListView, &QListView::activated,
            this, [this](const QModelIndex& idx) { emitFileActivated(idx, true); });
    connect(m_unstagedListView, &QListView::activated,
            this, [this](const QModelIndex& idx) { emitFileActivated(idx, false); });
    connect(m_stagedListView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex& current) { emitFileActivated(current, true); });
    connect(m_unstagedListView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex& current) { emitFileActivated(current, false); });

    connect(m_stagedListView, &QListView::customContextMenuRequested,
            this, &ChangesPage::handleStagedContextMenu);
    connect(m_unstagedListView, &QListView::customContextMenuRequested,
            this, &ChangesPage::handleUnstagedContextMenu);

    connect(m_stageAllButton, &QToolButton::clicked,
            this, &ChangesPage::stageAllRequested);

    connect(m_unstageAllButton, &QToolButton::clicked,
            this, &ChangesPage::unstageAllRequested);
}

QModelIndex ChangesPage::toSourceIndex(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    if (index.model() == m_stagedProxyModel) {
        return m_stagedProxyModel->mapToSource(index);
    }
    if (index.model() == m_unstagedProxyModel) {
        return m_unstagedProxyModel->mapToSource(index);
    }

    return index;
}

void ChangesPage::emitFileActivated(const QModelIndex& index, bool staged)
{
    const QModelIndex sourceIndex = toSourceIndex(index);
    if (sourceIndex.isValid()) {
        emit fileActivated(sourceIndex, staged);
    }
}

void ChangesPage::handleStagedContextMenu(const QPoint& pos)
{
    const QModelIndex index = m_stagedListView->indexAt(pos);
    const QModelIndex sourceIndex = toSourceIndex(index);
    if (!sourceIndex.isValid()) {
        return;
    }

    QMenu menu(this);

    QAction* unstageAction = menu.addAction(tr("Unstage"));

    QAction* chosenAction = menu.exec(m_stagedListView->viewport()->mapToGlobal(pos));
    if (chosenAction == unstageAction) {
        emit unstageRequested(sourceIndex);
    }
}

void ChangesPage::handleUnstagedContextMenu(const QPoint& pos)
{
    const QModelIndex index = m_unstagedListView->indexAt(pos);
    const QModelIndex sourceIndex = toSourceIndex(index);
    if (!sourceIndex.isValid()) {
        return;
    }

    QMenu menu(this);

    QAction* stageAction = menu.addAction(tr("Stage"));
    menu.addSeparator();
    QAction* discardAction = menu.addAction(tr("Discard Changes"));

    // 仅对未追踪文件显示 Ignore 选项
    const bool isUntracked = sourceIndex.data(FileChangeModel::UntrackedRole).toBool();
    QAction* ignoreAction = nullptr;
    if (isUntracked) {
        ignoreAction = menu.addAction(tr("Ignore"));
    }

    QAction* renameAction = menu.addAction(tr("Rename..."));

    QAction* chosenAction = menu.exec(m_unstagedListView->viewport()->mapToGlobal(pos));
    if (chosenAction == stageAction) {
        emit stageRequested(sourceIndex);
    } else if (chosenAction == discardAction) {
        emit discardRequested(sourceIndex);
    } else if (ignoreAction && chosenAction == ignoreAction) {
        emit ignoreRequested(sourceIndex);
    } else if (chosenAction == renameAction) {
        emit renameRequested(sourceIndex);
    }
}
