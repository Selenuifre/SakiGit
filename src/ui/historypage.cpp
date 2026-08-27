#include "historypage.h"
#include "CommitGraphView.h"
#include "diffviewer.h"
#include "models/CommitGraphModel.h"
#include "models/commithistorymodel.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QMenu>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSplitter>
#include <QStackedWidget>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

// ===== Two-line commit item delegate =====
class CommitItemDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, false);

        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, QColor(0x3A, 0x42, 0x4A));
        } else if (index.row() % 2 == 0) {
            painter->fillRect(option.rect, QColor(0x2D, 0x33, 0x3B));
        } else {
            painter->fillRect(option.rect, QColor(0x25, 0x2A, 0x31));
        }

        const QString summary = index.data(CommitHistoryModel::SummaryRole).toString();
        const QString shortHash = index.data(CommitHistoryModel::ShortHashRole).toString();

        QFont bigFont = option.font;
        bigFont.setPointSize(10);
        bigFont.setBold(true);
        painter->setFont(bigFont);
        painter->setPen(option.state & QStyle::State_Selected
                            ? QColor(0xFF, 0xFF, 0xFF)
                            : QColor(0xAD, 0xBA, 0xC7));

        QRect bigRect = option.rect.adjusted(10, 4, -8, -(option.rect.height() / 2));
        QString elided = painter->fontMetrics().elidedText(
            summary.isEmpty() ? QStringLiteral("(no message)") : summary,
            Qt::ElideRight, bigRect.width());
        painter->drawText(bigRect, Qt::AlignLeft | Qt::AlignVCenter, elided);

        QFont smallFont = option.font;
        smallFont.setPointSize(8);
        painter->setFont(smallFont);
        painter->setPen(option.state & QStyle::State_Selected
                            ? QColor(0xBB, 0xBB, 0xBB)
                            : QColor(0x76, 0x83, 0x90));

        QRect smallRect = option.rect.adjusted(10, option.rect.height() / 2, -8, -2);
        painter->drawText(smallRect, Qt::AlignLeft | Qt::AlignVCenter, shortHash);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& /*index*/) const override
    {
        return QSize(option.rect.width(), 44);
    }
};

// ===== HistoryPage =====

HistoryPage::HistoryPage(QWidget* parent)
    : QWidget(parent),
    m_commitListView(nullptr),
    m_commitDetailView(nullptr),
    m_mainSplitter(nullptr),
    m_rightStack(nullptr),
    m_graphScrollArea(nullptr),
    m_diffViewer(nullptr),
    m_graphView(nullptr),
    m_changedFilesList(nullptr)
{
    setupUi();
}

void HistoryPage::setModel(QAbstractItemModel* model)
{
    m_commitListView->setModel(model);
}

QAbstractItemModel* HistoryPage::model() const
{
    return m_commitListView->model();
}

void HistoryPage::setDiffModel(QAbstractItemModel* model)
{
    m_diffViewer->setModel(model);
}

void HistoryPage::setGraphModel(CommitGraphModel* model)
{
    m_graphView->setModel(model);
    if (auto* hm = qobject_cast<CommitHistoryModel*>(m_commitListView->model())) {
        m_graphView->setCommitHistoryModel(hm);
    }
}

void HistoryPage::setCommitDetail(const QString& detail)
{
    m_commitDetailView->setPlainText(detail);
}

void HistoryPage::setChangedFiles(const QStringList& files)
{
    if (!m_changedFilesList) return;
    if (auto* old = m_changedFilesList->model()) old->deleteLater();
    m_changedFilesList->setModel(new QStringListModel(files, m_changedFilesList));
}

DiffViewer* HistoryPage::diffViewer() const { return m_diffViewer; }
CommitGraphView* HistoryPage::graphView() const { return m_graphView; }
QListView* HistoryPage::listView() const { return m_commitListView; }

void HistoryPage::setupUi()
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ===== Main horizontal splitter: list | right panel =====
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->setChildrenCollapsible(false);

    // ------ Left: commit list ------
    auto* leftPanel = new QWidget(m_mainSplitter);
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(2);

    auto* historyLabel = new QLabel(tr("Commits"), leftPanel);
    historyLabel->setObjectName(QStringLiteral("sectionLabel"));

    m_commitListView = new QListView(leftPanel);
    m_commitListView->setObjectName(QStringLiteral("commitHistoryListView"));
    m_commitListView->setEditTriggers(QListView::NoEditTriggers);
    m_commitListView->setSelectionMode(QListView::SingleSelection);
    m_commitListView->setItemDelegate(new CommitItemDelegate(m_commitListView));
    m_commitListView->setVerticalScrollMode(QListView::ScrollPerPixel);

    leftLayout->addWidget(historyLabel);
    leftLayout->addWidget(m_commitListView, 1);

    // ------ Right: toggle button + stacked (graph | diff) ------
    auto* rightPanel = new QWidget(m_mainSplitter);
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(4);

    // Toggle Switch: custom sliding switch for Graph / Diff
    auto* toggleBar = new QHBoxLayout;
    toggleBar->setContentsMargins(0, 0, 0, 0);

    // Custom toggle switch using a checkable QPushButton styled as pill
    auto* toggleSwitch = new QPushButton(tr("Graph"), rightPanel);
    toggleSwitch->setObjectName(QStringLiteral("historyToggleSwitch"));
    toggleSwitch->setCheckable(true);
    toggleSwitch->setChecked(true);
    toggleSwitch->setFixedSize(52, 24);
    toggleSwitch->setCursor(Qt::PointingHandCursor);

    auto* toggleLabel = new QLabel(tr("Diff"), rightPanel);
    toggleLabel->setObjectName(QStringLiteral("historyToggleLabel"));

    toggleBar->addWidget(toggleSwitch);
    toggleBar->addWidget(toggleLabel);
    toggleBar->addStretch(1);
    rightLayout->addLayout(toggleBar);

    // Stacked widget: graph | diff+files
    m_rightStack = new QStackedWidget(rightPanel);

    // Page 0: Graph
    m_graphScrollArea = new QScrollArea(m_rightStack);
    m_graphScrollArea->setObjectName(QStringLiteral("historyGraphPage"));
    m_graphScrollArea->setWidgetResizable(false);
    m_graphScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_graphScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    m_graphView = new CommitGraphView(m_graphScrollArea);
    m_graphView->setObjectName(QStringLiteral("commitGraphView"));
    m_graphScrollArea->setWidget(m_graphView);
    m_rightStack->addWidget(m_graphScrollArea); // index 0

    // Page 1: Diff viewer + changed files list (QHBoxLayout, no splitter sizing issues)
    auto* diffPanel = new QWidget(m_rightStack);
    auto* diffLayout = new QHBoxLayout(diffPanel);
    diffLayout->setContentsMargins(0, 0, 0, 0);
    diffLayout->setSpacing(0);

    // Changed files list (fixed width on the left)
    auto* filesPanel = new QWidget(diffPanel);
    filesPanel->setFixedWidth(180);
    auto* filesLayout = new QVBoxLayout(filesPanel);
    filesLayout->setContentsMargins(0, 0, 0, 0);
    filesLayout->setSpacing(2);
    auto* filesLabel = new QLabel(tr("Changed Files"), filesPanel);
    filesLabel->setObjectName(QStringLiteral("sectionLabel"));
    m_changedFilesList = new QListView(filesPanel);
    m_changedFilesList->setObjectName(QStringLiteral("changedFilesListView"));
    m_changedFilesList->setEditTriggers(QListView::NoEditTriggers);
    m_changedFilesList->setSelectionMode(QListView::SingleSelection);
    filesLayout->addWidget(filesLabel);
    filesLayout->addWidget(m_changedFilesList, 1);

    m_diffViewer = new DiffViewer(diffPanel);
    m_diffViewer->setPlaceholderText(tr("Select a commit to view its diff"));

    diffLayout->addWidget(filesPanel);
    diffLayout->addWidget(m_diffViewer, 1);

    m_rightStack->addWidget(diffPanel); // index 1

    rightLayout->addWidget(m_rightStack, 1);

    // Toggle switch logic
    connect(toggleSwitch, &QPushButton::clicked, this, &HistoryPage::toggleRightPanel);

    // Assemble
    m_mainSplitter->addWidget(leftPanel);
    m_mainSplitter->addWidget(rightPanel);
    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);
    m_mainSplitter->setSizes({280, 880});

    rootLayout->addWidget(m_mainSplitter, 1);

    // Hidden detail view (kept for setCommitDetail)
    m_commitDetailView = new QPlainTextEdit;
    m_commitDetailView->setVisible(false);

    // ---- Signals ----
    connect(m_commitListView, &QListView::clicked,
            this, &HistoryPage::handleCommitClicked);

    connect(m_graphView, &CommitGraphView::commitNodeClicked, this,
            [this](const QString& hash) {
        if (auto* hm = qobject_cast<CommitHistoryModel*>(m_commitListView->model())) {
            QModelIndex idx = hm->indexForHash(hash);
            if (idx.isValid()) {
                m_commitListView->scrollTo(idx);
                m_commitListView->setCurrentIndex(idx);
                emit commitSelected(idx);
            }
        }
    });

    connect(m_graphView, &CommitGraphView::commitNodeRightClicked,
            this, &HistoryPage::handleGraphContextMenu);

    // Click changed file → emit with commit hash
    connect(m_changedFilesList, &QListView::clicked, this, [this](const QModelIndex& idx) {
        const QString path = idx.data(Qt::DisplayRole).toString();
        if (!path.isEmpty() && !m_currentCommitHash.isEmpty())
            emit fileInCommitClicked(m_currentCommitHash, path);
    });
}

void HistoryPage::toggleRightPanel()
{
    m_showingGraph = !m_showingGraph;
    if (m_showingGraph) {
        m_rightStack->setCurrentIndex(0);
    } else {
        m_rightStack->setCurrentIndex(1);
    }
    // Update toggle button and label
    if (auto* ts = findChild<QPushButton*>(QStringLiteral("historyToggleSwitch")))
        ts->setText(m_showingGraph ? tr("Graph") : tr("Diff"));
    if (auto* tl = findChild<QLabel*>(QStringLiteral("historyToggleLabel")))
        tl->setText(m_showingGraph ? tr("Diff") : tr("Graph"));
}

void HistoryPage::handleCommitClicked(const QModelIndex& index)
{
    const QString hash = index.data(CommitHistoryModel::HashRole).toString();
    m_currentCommitHash = hash;
    if (!hash.isEmpty()) {
        centerGraphOnCommit(hash);
        m_graphView->setSelectedHash(hash);
    }

    // Populate changed files list from the commit model
    const QStringList files = index.data(CommitHistoryModel::ChangedFilesRole).toStringList();
    if (m_changedFilesList) {
        if (auto* old = m_changedFilesList->model()) old->deleteLater();
        m_changedFilesList->setModel(new QStringListModel(files, m_changedFilesList));
    }

    emit commitSelected(index);
}

void HistoryPage::centerGraphOnCommit(const QString& hash)
{
    if (!m_graphView || !m_graphView->model() || !m_graphScrollArea) return;

    auto* model = m_graphView->model();
    for (int r = 0; r < model->totalRows(); ++r) {
        auto* node = model->nodeAt(r);
        if (node && node->hash == hash) {
            QPointF pt = m_graphView->nodePt(r, node->column);
            int cx = static_cast<int>(pt.x() - m_graphScrollArea->viewport()->width() / 2.0);
            int cy = static_cast<int>(pt.y() - m_graphScrollArea->viewport()->height() / 2.0);
            m_graphScrollArea->horizontalScrollBar()->setValue(qMax(0, cx));
            m_graphScrollArea->verticalScrollBar()->setValue(qMax(0, cy));
            break;
        }
    }
}

void HistoryPage::handleGraphContextMenu(const QString& hash) {
    const CommitGraphNode* found = nullptr;
    for (int r = 0; r < m_graphView->model()->totalRows(); ++r) {
        auto* n = m_graphView->model()->nodeAt(r);
        if (n && n->hash == hash) { found = n; break; }
    }
    if (!found) return;

    if (!m_contextMenu) m_contextMenu = new QMenu(this);
    m_contextMenu->clear();

    QAction* detailsAction = m_contextMenu->addAction(tr("Show Details"));
    m_contextMenu->addSeparator();
    QAction* copyAction = m_contextMenu->addAction(tr("Copy Hash"));
    m_contextMenu->addSeparator();
    QAction* checkoutAction = m_contextMenu->addAction(tr("Checkout this commit"));
    QAction* branchAction  = m_contextMenu->addAction(tr("Create branch here..."));
    m_contextMenu->addSeparator();
    QAction* resetSoftAction = m_contextMenu->addAction(tr("Reset --soft to here"));
    QAction* resetHardAction = m_contextMenu->addAction(tr("Reset --hard to here"));

    QAction* chosen = m_contextMenu->exec(QCursor::pos());

    if (chosen == detailsAction) {
        const QString detail = m_commitDetailView->toPlainText();
        QDialog dlg(this);
        dlg.setWindowTitle(tr("Commit Details — %1").arg(found->shortHash));
        dlg.resize(600, 450);
        auto* dl = new QVBoxLayout(&dlg);
        auto* detailEdit = new QPlainTextEdit(&dlg);
        detailEdit->setReadOnly(true);
        detailEdit->setPlainText(detail.isEmpty()
            ? tr("Select a commit in the list to view its details.") : detail);
        dl->addWidget(detailEdit, 1);
        auto* bb = new QDialogButtonBox(QDialogButtonBox::Close, &dlg);
        connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        dl->addWidget(bb);
        dlg.exec();
    } else if (chosen == copyAction) {
        QApplication::clipboard()->setText(hash);
    } else if (chosen == checkoutAction) {
        emit checkoutCommitRequested(hash);
    } else if (chosen == branchAction) {
        emit createBranchAtCommitRequested(hash);
    } else if (chosen == resetSoftAction) {
        emit resetToCommitRequested(hash, QStringLiteral("soft"));
    } else if (chosen == resetHardAction) {
        emit resetToCommitRequested(hash, QStringLiteral("hard"));
    }
}
