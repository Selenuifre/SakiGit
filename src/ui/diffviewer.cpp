#include "diffviewer.h"

#include <QAbstractItemModel>
#include <QHeaderView>
#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>

DiffViewer::DiffViewer(QWidget* parent)
    : QWidget(parent),
    m_placeholderLabel(nullptr),
    m_tableView(nullptr)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_tableView = new QTableView(this);
    m_tableView->setObjectName(QStringLiteral("diffViewerTable"));
    m_tableView->setAlternatingRowColors(false);
    m_tableView->setShowGrid(false);
    m_tableView->setEditTriggers(QTableView::NoEditTriggers);
    m_tableView->setSelectionBehavior(QTableView::SelectRows);
    m_tableView->setSelectionMode(QTableView::SingleSelection);
    m_tableView->setWordWrap(false);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->verticalHeader()->setDefaultSectionSize(20);
    m_tableView->horizontalHeader()->setStretchLastSection(true);

    m_placeholderLabel = new QLabel(this);
    m_placeholderLabel->setObjectName(QStringLiteral("diffViewerPlaceholder"));
    m_placeholderLabel->setAlignment(Qt::AlignCenter);
    m_placeholderLabel->setText(tr("Select a changed file to view diff"));
    m_placeholderLabel->hide();

    layout->addWidget(m_tableView, 1);
    layout->addWidget(m_placeholderLabel);
}

void DiffViewer::setModel(QAbstractItemModel* model)
{
    QAbstractItemModel* previousModel = m_tableView->model();
    if (previousModel) {
        disconnect(previousModel, nullptr, this, nullptr);
    }

    m_tableView->setModel(model);

    if (model) {
        connect(model, &QAbstractItemModel::modelReset,
                this, &DiffViewer::updateViewState);
        connect(model, &QAbstractItemModel::rowsInserted,
                this, &DiffViewer::updateViewState);
        connect(model, &QAbstractItemModel::rowsRemoved,
                this, &DiffViewer::updateViewState);
    }

    updateViewState();
}

QAbstractItemModel* DiffViewer::model() const
{
    return m_tableView->model();
}

QTableView* DiffViewer::tableView() const
{
    return m_tableView;
}

void DiffViewer::clear()
{
    setModel(nullptr);
}

void DiffViewer::setPlaceholderText(const QString& text)
{
    const QString clean = text.trimmed();
    m_placeholderLabel->setText(clean.isEmpty()
        ? tr("Select a changed file to view diff") : clean);
}

void DiffViewer::updateViewState()
{
    QAbstractItemModel* currentModel = m_tableView->model();
    const bool hasRows = currentModel && currentModel->rowCount() > 0;

    if (!hasRows) {
        m_tableView->hide();
        m_placeholderLabel->show();
        return;
    }

    m_placeholderLabel->hide();
    m_tableView->show();

    QHeaderView* header = m_tableView->horizontalHeader();
    if (header->count() > 0) {
        header->resizeSection(0, 60);
    }
    if (header->count() > 1) {
        header->resizeSection(1, 60);
    }
}
