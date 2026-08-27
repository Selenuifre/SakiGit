#include "repositorysidebar.h"

#include "models/repositorylistmodel.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QPoint>
#include <QSortFilterProxyModel>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

RepositorySidebar::RepositorySidebar(QWidget* parent)
    : QWidget(parent),
    m_emptyLabel(nullptr),
    m_filterEdit(nullptr),
    m_listView(nullptr),
    m_contextMenu(nullptr),
    m_openAction(nullptr),
    m_cloneAction(nullptr),
    m_refreshAction(nullptr),
    m_removeAction(nullptr),
    m_proxyModel(new QSortFilterProxyModel(this)),
    m_emptyText(tr("No repositories")),
    m_busy(false)
{
    setupUi();
    setupActions();
    updateEmptyState();
    updateBusyState();
}

RepositorySidebar::~RepositorySidebar()
{
}

void RepositorySidebar::setRepositoryModel(QAbstractItemModel* model)
{
    if (m_proxyModel->sourceModel() == model) {
        return;
    }

    if (m_proxyModel->sourceModel()) {
        disconnect(m_proxyModel->sourceModel(), nullptr, this, nullptr);
    }

    m_proxyModel->setSourceModel(model);

    if (model) {
        connect(model, &QAbstractItemModel::rowsInserted,
                this, &RepositorySidebar::updateEmptyState);
        connect(model, &QAbstractItemModel::rowsRemoved,
                this, &RepositorySidebar::updateEmptyState);
        connect(model, &QAbstractItemModel::modelReset,
                this, &RepositorySidebar::updateEmptyState);
        connect(model, &QAbstractItemModel::layoutChanged,
                this, &RepositorySidebar::updateEmptyState);
    }

    updateEmptyState();
}

QAbstractItemModel* RepositorySidebar::repositoryModel() const
{
    return m_proxyModel->sourceModel();
}

QSortFilterProxyModel* RepositorySidebar::proxyModel() const
{
    return m_proxyModel;
}

QListView* RepositorySidebar::listView() const
{
    return m_listView;
}

void RepositorySidebar::setCurrentRepositoryIndex(const QModelIndex& sourceIndex)
{
    const QModelIndex proxyIndex = mapFromSourceIndex(sourceIndex);

    if (!proxyIndex.isValid()) {
        m_listView->clearSelection();
        m_listView->setCurrentIndex(QModelIndex());
        return;
    }

    m_listView->setCurrentIndex(proxyIndex);
}

QModelIndex RepositorySidebar::currentRepositoryIndex() const
{
    return mapToSourceIndex(m_listView->currentIndex());
}

void RepositorySidebar::setFilterText(const QString& text)
{
    if (m_filterEdit->text() == text) {
        return;
    }

    m_filterEdit->setText(text);
}

QString RepositorySidebar::filterText() const
{
    return m_filterEdit->text();
}

void RepositorySidebar::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }

    m_busy = busy;
    updateBusyState();
}

bool RepositorySidebar::isBusy() const
{
    return m_busy;
}

void RepositorySidebar::setEmptyText(const QString& text)
{
    const QString cleanText = text.trimmed();
    m_emptyText = cleanText.isEmpty() ? tr("No repositories") : cleanText;
    updateEmptyState();
}

QString RepositorySidebar::emptyText() const
{
    return m_emptyText;
}

void RepositorySidebar::handleFilterTextChanged(const QString& text)
{
    m_proxyModel->setFilterFixedString(text.trimmed());
    updateEmptyState();
    emit filterTextChanged(text);
}

void RepositorySidebar::handleRepositoryActivated(const QModelIndex& proxyIndex)
{
    const QModelIndex sourceIndex = mapToSourceIndex(proxyIndex);

    if (sourceIndex.isValid()) {
        emit repositoryActivated(sourceIndex);
    }
}

void RepositorySidebar::handleCurrentChanged(const QModelIndex& current,
                                             const QModelIndex& previous)
{
    Q_UNUSED(previous)

    const QModelIndex sourceIndex = mapToSourceIndex(current);

    if (sourceIndex.isValid()) {
        emit repositorySelectionChanged(sourceIndex);
    }
}

void RepositorySidebar::showContextMenu(const QPoint& position)
{
    m_contextProxyIndex = m_listView->indexAt(position);
    const bool hasRepository = m_contextProxyIndex.isValid();

    m_removeAction->setEnabled(hasRepository && !m_busy);

    m_contextMenu->exec(m_listView->viewport()->mapToGlobal(position));
}

void RepositorySidebar::handleRemoveAction()
{
    const QModelIndex sourceIndex = contextSourceIndex();

    if (sourceIndex.isValid()) {
        emit removeRepositoryRequested(sourceIndex);
    }
}

void RepositorySidebar::updateEmptyState()
{
    const bool hasRows = m_proxyModel->rowCount() > 0;
    m_emptyLabel->setVisible(!hasRows);
    m_emptyLabel->setText(m_filterEdit->text().trimmed().isEmpty()
                              ? m_emptyText
                              : tr("No matching repositories"));
}

void RepositorySidebar::setupUi()
{
    setObjectName(QStringLiteral("RepositorySidebar"));
    setMinimumWidth(180);
    setMaximumWidth(300);

    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterRole(RepositoryListModel::DisplayNameRole);
    // 不使用 dynamicSortFilter，避免 dataChanged 信号被代理模型的全量重排吞掉

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(6);

    // Filter input at top
    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setClearButtonEnabled(true);
    m_filterEdit->setPlaceholderText(tr("Filter repositories"));

    // Repository list view (main content)
    m_listView = new QListView(this);
    m_listView->setObjectName(QStringLiteral("repositorySidebarListView"));
    m_listView->setModel(m_proxyModel);
    m_listView->setAlternatingRowColors(true);
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listView->setEditTriggers(QListView::NoEditTriggers);
    m_listView->setSelectionMode(QListView::SingleSelection);

    // Empty state label
    m_emptyLabel = new QLabel(m_emptyText, this);
    m_emptyLabel->setObjectName(QStringLiteral("repositorySidebarEmptyLabel"));
    m_emptyLabel->setAlignment(Qt::AlignCenter);

    // Bottom button bar: Add + Clone
    auto* bottomBar = new QHBoxLayout;
    bottomBar->setContentsMargins(0, 0, 0, 0);
    bottomBar->setSpacing(4);

    auto* addButton = new QToolButton(this);
    addButton->setToolTip(tr("Add local repository"));
    addButton->setIcon(QIcon(QStringLiteral(":/icons/action_open.svg")));
    addButton->setText(tr("Add"));
    addButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    addButton->setObjectName(QStringLiteral("sidebarAddButton"));

    auto* cloneButton = new QToolButton(this);
    cloneButton->setToolTip(tr("Clone repository"));
    cloneButton->setIcon(QIcon(QStringLiteral(":/icons/action_clone.svg")));
    cloneButton->setText(tr("Clone"));
    cloneButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    cloneButton->setObjectName(QStringLiteral("sidebarCloneButton"));

    bottomBar->addWidget(addButton);
    bottomBar->addWidget(cloneButton);
    bottomBar->addStretch(1);

    rootLayout->addWidget(m_filterEdit);
    rootLayout->addWidget(m_listView, 1);
    rootLayout->addWidget(m_emptyLabel);
    rootLayout->addLayout(bottomBar);
}

void RepositorySidebar::setupActions()
{
    m_openAction = new QAction(tr("Open"), this);
    m_cloneAction = new QAction(tr("Clone"), this);
    m_refreshAction = new QAction(tr("Refresh"), this);
    m_removeAction = new QAction(tr("Remove"), this);

    m_contextMenu = new QMenu(this);
    m_contextMenu->addAction(m_openAction);
    m_contextMenu->addAction(m_cloneAction);
    m_contextMenu->addAction(m_refreshAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_removeAction);

    // Bottom bar buttons: Add → open, Clone → clone
    // The buttons are accessed by object name since they're local to setupUi
    QToolButton* addBtn = findChild<QToolButton*>(QStringLiteral("sidebarAddButton"));
    QToolButton* cloneBtn = findChild<QToolButton*>(QStringLiteral("sidebarCloneButton"));

    if (addBtn) {
        connect(addBtn, &QToolButton::clicked,
                this, &RepositorySidebar::openRepositoryRequested);
    }
    if (cloneBtn) {
        connect(cloneBtn, &QToolButton::clicked,
                this, &RepositorySidebar::cloneRepositoryRequested);
    }

    connect(m_openAction, &QAction::triggered,
            this, &RepositorySidebar::openRepositoryRequested);
    connect(m_cloneAction, &QAction::triggered,
            this, &RepositorySidebar::cloneRepositoryRequested);
    connect(m_refreshAction, &QAction::triggered,
            this, &RepositorySidebar::refreshRequested);
    connect(m_removeAction, &QAction::triggered,
            this, &RepositorySidebar::handleRemoveAction);

    connect(m_filterEdit, &QLineEdit::textChanged,
            this, &RepositorySidebar::handleFilterTextChanged);
    connect(m_listView, &QListView::activated,
            this, &RepositorySidebar::handleRepositoryActivated);
    connect(m_listView, &QListView::customContextMenuRequested,
            this, &RepositorySidebar::showContextMenu);
    connect(m_listView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &RepositorySidebar::handleCurrentChanged);

    connect(m_proxyModel, &QSortFilterProxyModel::rowsInserted,
            this, &RepositorySidebar::updateEmptyState);
    connect(m_proxyModel, &QSortFilterProxyModel::rowsRemoved,
            this, &RepositorySidebar::updateEmptyState);
    connect(m_proxyModel, &QSortFilterProxyModel::modelReset,
            this, &RepositorySidebar::updateEmptyState);
}

QModelIndex RepositorySidebar::mapToSourceIndex(const QModelIndex& proxyIndex) const
{
    if (!proxyIndex.isValid()) {
        return QModelIndex();
    }

    return m_proxyModel->mapToSource(proxyIndex);
}

QModelIndex RepositorySidebar::mapFromSourceIndex(const QModelIndex& sourceIndex) const
{
    if (!sourceIndex.isValid()) {
        return QModelIndex();
    }

    return m_proxyModel->mapFromSource(sourceIndex);
}

QModelIndex RepositorySidebar::contextSourceIndex() const
{
    return mapToSourceIndex(m_contextProxyIndex);
}

void RepositorySidebar::updateBusyState()
{
    m_filterEdit->setEnabled(!m_busy);

    m_openAction->setEnabled(!m_busy);
    m_cloneAction->setEnabled(!m_busy);
    m_refreshAction->setEnabled(!m_busy);
    m_removeAction->setEnabled(!m_busy);

    QToolButton* addBtn = findChild<QToolButton*>(QStringLiteral("sidebarAddButton"));
    QToolButton* cloneBtn = findChild<QToolButton*>(QStringLiteral("sidebarCloneButton"));
    if (addBtn) addBtn->setEnabled(!m_busy);
    if (cloneBtn) cloneBtn->setEnabled(!m_busy);
}
