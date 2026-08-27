#include "PullRequestsPage.h"

#include "models/PullRequestModel.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QTextEdit>
#include <QVBoxLayout>

PullRequestsPage::PullRequestsPage(QWidget* parent)
    : QWidget(parent),
    m_prListView(nullptr),
    m_titleEdit(nullptr),
    m_headEdit(nullptr),
    m_baseEdit(nullptr),
    m_bodyEdit(nullptr),
    m_createButton(nullptr)
{
    setupUi();
}

void PullRequestsPage::setModel(QAbstractItemModel* model)
{
    m_prListView->setModel(model);
}

QAbstractItemModel* PullRequestsPage::model() const
{
    return m_prListView->model();
}

QListView* PullRequestsPage::listView() const
{
    return m_prListView;
}

void PullRequestsPage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    // 标题 + 刷新按钮
    auto* headerLayout = new QHBoxLayout;
    auto* titleLabel = new QLabel(tr("Pull Requests"), this);
    titleLabel->setObjectName(QStringLiteral("sectionLabel"));
    auto* refreshButton = new QPushButton(tr("Refresh"), this);
    refreshButton->setFixedWidth(80);
    headerLayout->addWidget(titleLabel, 1);
    headerLayout->addWidget(refreshButton);

    // PR 列表
    m_prListView = new QListView(this);
    m_prListView->setObjectName(QStringLiteral("prListView"));
    m_prListView->setAlternatingRowColors(true);
    m_prListView->setEditTriggers(QListView::NoEditTriggers);
    m_prListView->setContextMenuPolicy(Qt::CustomContextMenu);

    // 创建 PR 表单（放在可滚动区域中）
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMaximumHeight(260);

    auto* formWidget = new QWidget(scrollArea);
    auto* formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(4);

    m_titleEdit = new QLineEdit(formWidget);
    m_titleEdit->setPlaceholderText(tr("PR title"));
    formLayout->addRow(tr("Title:"), m_titleEdit);

    m_headEdit = new QLineEdit(formWidget);
    m_headEdit->setPlaceholderText(tr("e.g. feature/my-branch"));
    formLayout->addRow(tr("Head branch:"), m_headEdit);

    m_baseEdit = new QLineEdit(formWidget);
    m_baseEdit->setPlaceholderText(tr("e.g. main"));
    m_baseEdit->setText(QStringLiteral("main"));
    formLayout->addRow(tr("Base branch:"), m_baseEdit);

    m_bodyEdit = new QTextEdit(formWidget);
    m_bodyEdit->setPlaceholderText(tr("PR description (optional)"));
    m_bodyEdit->setMaximumHeight(80);
    formLayout->addRow(tr("Body:"), m_bodyEdit);

    m_createButton = new QPushButton(tr("Create Pull Request"), formWidget);
    formLayout->addRow(QString(), m_createButton);

    scrollArea->setWidget(formWidget);

    layout->addLayout(headerLayout);
    layout->addWidget(m_prListView, 1);
    layout->addWidget(scrollArea);

    // ---- 信号连接 ----
    connect(refreshButton, &QPushButton::clicked,
            this, &PullRequestsPage::refreshRequested);
    connect(m_createButton, &QPushButton::clicked,
            this, &PullRequestsPage::handleCreateClicked);

    connect(m_prListView, &QListView::customContextMenuRequested,
            this, &PullRequestsPage::handleContextMenu);
}

void PullRequestsPage::handleCreateClicked()
{
    const QString title = m_titleEdit->text().trimmed();
    const QString head = m_headEdit->text().trimmed();
    const QString base = m_baseEdit->text().trimmed();
    const QString body = m_bodyEdit->toPlainText().trimmed();

    if (title.isEmpty() || head.isEmpty() || base.isEmpty()) {
        return;
    }

    emit createPullRequestRequested(title, body, head, base);
    m_titleEdit->clear();
    m_bodyEdit->clear();
}

void PullRequestsPage::handleContextMenu(const QPoint& pos)
{
    const QModelIndex index = m_prListView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    const int prNumber = index.data(PullRequestModel::NumberRole).toInt();
    const QString state = index.data(PullRequestModel::StateRole).toString();

    QMenu menu(this);
    if (state == QStringLiteral("Open")) {
        QAction* mergeAction = menu.addAction(tr("Merge"));
        QAction* chosenAction = menu.exec(m_prListView->viewport()->mapToGlobal(pos));
        if (chosenAction == mergeAction) {
            emit mergeRequested(prNumber);
        }
    }
}
