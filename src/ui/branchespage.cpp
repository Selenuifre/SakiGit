#include "branchespage.h"

#include <QAbstractItemModel>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

BranchesPage::BranchesPage(QWidget* parent)
    : QWidget(parent),
    m_branchListView(nullptr),
    m_newBranchEdit(nullptr),
    m_createButton(nullptr),
    m_refreshButton(nullptr)
{
    setupUi();
}

void BranchesPage::setModel(QAbstractItemModel* model)
{
    m_branchListView->setModel(model);
}

QAbstractItemModel* BranchesPage::model() const
{
    return m_branchListView->model();
}

QListView* BranchesPage::listView() const
{
    return m_branchListView;
}

void BranchesPage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    // 标题行
    auto* headerLayout = new QHBoxLayout;
    auto* titleLabel = new QLabel(tr("Branches"), this);
    titleLabel->setObjectName(QStringLiteral("sectionLabel"));
    titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_refreshButton = new QToolButton(this);
    m_refreshButton->setToolTip(tr("Refresh branches"));
    m_refreshButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));

    headerLayout->addWidget(titleLabel, 1);
    headerLayout->addWidget(m_refreshButton);

    // 分支列表
    m_branchListView = new QListView(this);
    m_branchListView->setObjectName(QStringLiteral("branchListView"));
    m_branchListView->setAlternatingRowColors(true);
    m_branchListView->setEditTriggers(QListView::NoEditTriggers);
    m_branchListView->setContextMenuPolicy(Qt::CustomContextMenu);

    // 创建分支区域
    auto* createLayout = new QHBoxLayout;
    createLayout->setContentsMargins(0, 0, 0, 0);

    m_newBranchEdit = new QLineEdit(this);
    m_newBranchEdit->setPlaceholderText(tr("New branch name"));
    m_newBranchEdit->setClearButtonEnabled(true);

    m_createButton = new QPushButton(tr("Create Branch"), this);
    m_createButton->setEnabled(false);

    createLayout->addWidget(m_newBranchEdit, 1);
    createLayout->addWidget(m_createButton);

    layout->addLayout(headerLayout);
    layout->addWidget(m_branchListView, 1);
    layout->addLayout(createLayout);

    // ---- 信号连接 ----
    connect(m_branchListView, &QListView::activated,
            this, &BranchesPage::branchSelected);

    connect(m_branchListView, &QListView::customContextMenuRequested,
            this, &BranchesPage::handleContextMenu);

    connect(m_newBranchEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_createButton->setEnabled(!text.trimmed().isEmpty());
    });

    connect(m_createButton, &QPushButton::clicked,
            this, &BranchesPage::handleCreateClicked);
}

void BranchesPage::handleCreateClicked()
{
    const QString name = m_newBranchEdit->text().trimmed();
    if (name.isEmpty()) {
        return;
    }

    emit createBranchRequested(name);
    m_newBranchEdit->clear();
}

void BranchesPage::handleContextMenu(const QPoint& pos)
{
    const QModelIndex index = m_branchListView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    // 通过模型角色获取分支名
    const QString branchName = index.data(Qt::DisplayRole).toString();

    QMenu menu(this);
    QAction* checkoutAction = menu.addAction(tr("Checkout"));
    menu.addSeparator();
    QAction* mergeAction = menu.addAction(tr("Merge into current"));
    QAction* rebaseAction = menu.addAction(tr("Rebase current onto..."));
    menu.addSeparator();
    QAction* deleteAction = menu.addAction(tr("Delete"));

    QAction* chosenAction = menu.exec(m_branchListView->viewport()->mapToGlobal(pos));
    if (chosenAction == checkoutAction) {
        emit checkoutRequested(branchName);
    } else if (chosenAction == mergeAction) {
        emit mergeRequested(branchName);
    } else if (chosenAction == rebaseAction) {
        emit rebaseRequested(branchName);
    } else if (chosenAction == deleteAction) {
        emit deleteBranchRequested(branchName);
    }
}
