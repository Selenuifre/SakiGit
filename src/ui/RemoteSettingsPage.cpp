#include "RemoteSettingsPage.h"

#include "models/RemoteListModel.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>

RemoteSettingsPage::RemoteSettingsPage(QWidget* parent)
    : QWidget(parent),
    m_remoteListView(nullptr),
    m_nameEdit(nullptr),
    m_urlEdit(nullptr),
    m_addButton(nullptr)
{
    setupUi();
}

void RemoteSettingsPage::setModel(QAbstractItemModel* model)
{
    m_remoteListView->setModel(model);
}

QAbstractItemModel* RemoteSettingsPage::model() const
{
    return m_remoteListView->model();
}

QListView* RemoteSettingsPage::listView() const
{
    return m_remoteListView;
}

void RemoteSettingsPage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    // 标题行
    auto* titleLabel = new QLabel(tr("Remote Repositories"), this);

    // Remote 列表
    m_remoteListView = new QListView(this);
    m_remoteListView->setObjectName(QStringLiteral("remoteListView"));
    m_remoteListView->setAlternatingRowColors(true);
    m_remoteListView->setEditTriggers(QListView::NoEditTriggers);
    m_remoteListView->setContextMenuPolicy(Qt::CustomContextMenu);

    // 添加 remote 区域
    auto* formLayout = new QFormLayout;
    formLayout->setSpacing(4);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(tr("Remote name (e.g. origin)"));
    formLayout->addRow(tr("Name:"), m_nameEdit);

    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setPlaceholderText(tr("Remote URL (e.g. https://github.com/user/repo.git)"));
    formLayout->addRow(tr("URL:"), m_urlEdit);

    m_addButton = new QPushButton(tr("Add Remote"), this);
    formLayout->addRow(QString(), m_addButton);

    layout->addWidget(titleLabel);
    layout->addWidget(m_remoteListView, 1);
    layout->addLayout(formLayout);

    // ---- 信号连接 ----
    connect(m_addButton, &QPushButton::clicked,
            this, &RemoteSettingsPage::handleAddClicked);

    connect(m_remoteListView, &QListView::customContextMenuRequested,
            this, &RemoteSettingsPage::handleContextMenu);
}

void RemoteSettingsPage::handleAddClicked()
{
    const QString name = m_nameEdit->text().trimmed();
    const QString url = m_urlEdit->text().trimmed();

    if (name.isEmpty()) {
        return;
    }

    if (url.isEmpty()) {
        return;
    }

    emit addRequested(name, url);
    m_nameEdit->clear();
    m_urlEdit->clear();
}

void RemoteSettingsPage::handleContextMenu(const QPoint& pos)
{
    const QModelIndex index = m_remoteListView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    const QString remoteName = index.data(RemoteListModel::NameRole).toString();
    const QString remoteUrl = index.data(RemoteListModel::UrlRole).toString();

    QMenu menu(this);
    QAction* renameAction = menu.addAction(tr("Rename..."));
    QAction* setUrlAction = menu.addAction(tr("Change URL..."));
    menu.addSeparator();
    QAction* removeAction = menu.addAction(tr("Remove"));

    QAction* chosenAction = menu.exec(m_remoteListView->viewport()->mapToGlobal(pos));
    if (chosenAction == removeAction) {
        emit removeRequested(remoteName);
    } else if (chosenAction == renameAction) {
        bool ok = false;
        const QString newName = QInputDialog::getText(
            this, tr("Rename Remote"),
            tr("New name for \"%1\":"), QLineEdit::Normal,
            remoteName, &ok);
        if (ok && !newName.trimmed().isEmpty() && newName.trimmed() != remoteName) {
            emit renameRequested(remoteName, newName.trimmed());
        }
    } else if (chosenAction == setUrlAction) {
        bool ok = false;
        const QString newUrl = QInputDialog::getText(
            this, tr("Change Remote URL"),
            tr("New URL for \"%1\":"), QLineEdit::Normal,
            remoteUrl, &ok);
        if (ok && !newUrl.trimmed().isEmpty()) {
            emit setUrlRequested(remoteName, newUrl.trimmed());
        }
    }
}
