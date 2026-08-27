#include "StashPage.h"
#include "PageHeaderWidget.h"
#include "uiutils.h"

#include "models/StashListModel.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>

StashPage::StashPage(QWidget* parent)
    : QWidget(parent),
    m_headerWidget(nullptr),
    m_stashListView(nullptr),
    m_messageEdit(nullptr),
    m_saveButton(nullptr)
{
    setupUi();
}

void StashPage::setModel(QAbstractItemModel* model)
{
    m_stashListView->setModel(model);
}

QAbstractItemModel* StashPage::model() const
{
    return m_stashListView->model();
}

QListView* StashPage::listView() const
{
    return m_stashListView;
}

void StashPage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    // 标题行
    m_headerWidget = new PageHeaderWidget(tr("Stashes"), false, this);
    layout->addWidget(m_headerWidget);

    // Stash 列表
    m_stashListView = createListView(this, QStringLiteral("stashListView"));

    // 创建 stash 区域
    auto* saveLayout = new QHBoxLayout;
    saveLayout->setContentsMargins(0, 0, 0, 0);

    m_messageEdit = new QLineEdit(this);
    m_messageEdit->setPlaceholderText(tr("Stash message (optional)"));
    m_messageEdit->setClearButtonEnabled(true);

    m_saveButton = new QPushButton(tr("Save Stash"), this);

    saveLayout->addWidget(m_messageEdit, 1);
    saveLayout->addWidget(m_saveButton);

    layout->addWidget(m_stashListView, 1);
    layout->addLayout(saveLayout);

    // ---- 信号连接 ----
    connect(m_stashListView, &QListView::activated,
            this, &StashPage::stashSelected);

    connect(m_stashListView, &QListView::customContextMenuRequested,
            this, &StashPage::handleContextMenu);

    connect(m_saveButton, &QPushButton::clicked,
            this, &StashPage::handleSaveClicked);

    connect(m_headerWidget, &PageHeaderWidget::refreshRequested,
            this, &StashPage::refreshRequested);
}

void StashPage::handleSaveClicked()
{
    const QString message = m_messageEdit->text().trimmed();
    emit saveStashRequested(message);
    m_messageEdit->clear();
}

void StashPage::handleContextMenu(const QPoint& pos)
{
    const QModelIndex index = m_stashListView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    const int stashIndex = index.data(StashListModel::IndexRole).toInt();

    QMenu menu(this);
    QAction* diffAction = menu.addAction(tr("Show Diff"));
    menu.addSeparator();
    QAction* applyAction = menu.addAction(tr("Apply"));
    menu.addSeparator();
    QAction* dropAction = menu.addAction(tr("Drop"));

    QAction* chosenAction = menu.exec(m_stashListView->viewport()->mapToGlobal(pos));
    if (chosenAction == diffAction) {
        emit showDiffRequested(stashIndex);
    } else if (chosenAction == applyAction) {
        emit applyStashRequested(stashIndex);
    } else if (chosenAction == dropAction) {
        emit dropStashRequested(stashIndex);
    }
}
