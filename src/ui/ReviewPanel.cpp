#include "ReviewPanel.h"
#include "models/ReviewFindingModel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>

ReviewPanel::ReviewPanel(QWidget* parent)
    : QWidget(parent)
    , m_findingListView(nullptr)
    , m_reviewStagedButton(nullptr)
    , m_reviewWorkingTreeButton(nullptr)
    , m_cancelButton(nullptr)
    , m_placeholderLabel(nullptr)
    , m_summaryLabel(nullptr)
    , m_detailView(nullptr)
    , m_model(nullptr)
{
    setupUi();
}

void ReviewPanel::setModel(QAbstractItemModel* model)
{
    m_model = model;
    m_findingListView->setModel(model);
}

QAbstractItemModel* ReviewPanel::model() const
{
    return m_model;
}

QListView* ReviewPanel::listView() const
{
    return m_findingListView;
}

void ReviewPanel::setReviewing(bool reviewing)
{
    m_reviewStagedButton->setEnabled(!reviewing);
    m_reviewStagedButton->setVisible(!reviewing);
    m_reviewWorkingTreeButton->setEnabled(!reviewing);
    m_reviewWorkingTreeButton->setVisible(!reviewing);
    m_cancelButton->setVisible(reviewing);

    if (reviewing) {
        m_placeholderLabel->setText(tr("AI is reviewing your code, please wait..."));
        m_placeholderLabel->setVisible(true);
    } else {
        m_placeholderLabel->setVisible(false);
    }
}

void ReviewPanel::setSummaryText(const QString& text)
{
    m_summaryLabel->setText(text);
    m_summaryLabel->setVisible(!text.isEmpty());
}

void ReviewPanel::setPlaceholderText(const QString& text)
{
    m_placeholderLabel->setText(text);
}

void ReviewPanel::handleFindingClicked(const QModelIndex& index)
{
    if (!index.isValid() || !m_model) {
        return;
    }

    const QString filePath = index.data(ReviewFindingModel::FilePathRole).toString();
    const int lineNumber = index.data(ReviewFindingModel::LineNumberRole).toInt();
    const QString severity = index.data(ReviewFindingModel::SeverityRole).toString();
    const QString category = index.data(ReviewFindingModel::CategoryRole).toString();
    const QString message = index.data(ReviewFindingModel::MessageRole).toString();
    const QString suggestion = index.data(ReviewFindingModel::SuggestionRole).toString();

    // 构建详情 HTML
    QString detailHtml;
    detailHtml += QStringLiteral("<h3>%1</h3>").arg(
        index.data(ReviewFindingModel::TitleRole).toString().toHtmlEscaped());
    detailHtml += QStringLiteral("<p><b>File:</b> %1").arg(filePath.toHtmlEscaped());
    if (lineNumber >= 0) {
        detailHtml += QStringLiteral(" <b>Line:</b> %1").arg(lineNumber);
    }
    detailHtml += QStringLiteral("</p>");
    detailHtml += QStringLiteral("<p><b>Severity:</b> <span style='color:%1'>%2</span> &nbsp; "
                                 "<b>Category:</b> %3</p>")
                      .arg(severity == QStringLiteral("critical") ? QStringLiteral("#d32f2f")
                           : severity == QStringLiteral("high")   ? QStringLiteral("#e64a19")
                           : severity == QStringLiteral("medium") ? QStringLiteral("#f57c00")
                           : severity == QStringLiteral("low")    ? QStringLiteral("#388e3c")
                           :                                        QStringLiteral("#1976d2"),
                           severity.toHtmlEscaped(),
                           category.toHtmlEscaped());

    if (!message.isEmpty()) {
        detailHtml += QStringLiteral("<p><b>Description:</b><br>%1</p>")
                          .arg(message.toHtmlEscaped().replace(QStringLiteral("\n"), QStringLiteral("<br>")));
    }

    if (!suggestion.isEmpty()) {
        detailHtml += QStringLiteral("<p><b>Suggestion:</b><br>%1</p>")
                          .arg(suggestion.toHtmlEscaped().replace(QStringLiteral("\n"), QStringLiteral("<br>")));
    }

    m_detailView->setHtml(detailHtml);
    m_placeholderLabel->setVisible(false);

    emit findingActivated(filePath, lineNumber);
}

void ReviewPanel::handleReviewStagedClicked()
{
    emit reviewStagedRequested();
}

void ReviewPanel::handleReviewWorkingTreeClicked()
{
    emit reviewWorkingTreeRequested();
}

void ReviewPanel::handleCancelClicked()
{
    emit cancelRequested();
}

void ReviewPanel::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(6);

    // 标题栏
    auto* headerLayout = new QHBoxLayout;
    auto* titleLabel = new QLabel(tr("AI Code Review"), this);
    titleLabel->setObjectName(QStringLiteral("sectionLabel"));

    m_reviewStagedButton = new QPushButton(tr("Review Staged"), this);
    m_reviewStagedButton->setToolTip(tr("Review staged changes using AI"));

    m_reviewWorkingTreeButton = new QPushButton(tr("Review Changes"), this);
    m_reviewWorkingTreeButton->setToolTip(tr("Review working tree changes using AI"));

    m_cancelButton = new QPushButton(tr("Cancel"), this);
    m_cancelButton->setToolTip(tr("Cancel the ongoing review"));
    m_cancelButton->setVisible(false);

    headerLayout->addWidget(titleLabel, 1);
    headerLayout->addWidget(m_reviewStagedButton);
    headerLayout->addWidget(m_reviewWorkingTreeButton);
    headerLayout->addWidget(m_cancelButton);

    // 占位文本
    m_placeholderLabel = new QLabel(
        tr("Click 'Review Staged' or 'Review Changes' to start an AI-powered code review."),
        this);
    m_placeholderLabel->setAlignment(Qt::AlignCenter);
    m_placeholderLabel->setWordWrap(true);
    m_placeholderLabel->setStyleSheet(
        QStringLiteral("color: #636E7B; font-size: 12px; padding: 20px;"));

    // 发现列表 + 详情面板（左右分栏）
    auto* splitter = new QSplitter(Qt::Horizontal, this);

    m_findingListView = new QListView(splitter);
    m_findingListView->setObjectName(QStringLiteral("reviewFindingListView"));
    m_findingListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_findingListView->setSelectionMode(QAbstractItemView::SingleSelection);

    m_detailView = new QTextEdit(splitter);
    m_detailView->setObjectName(QStringLiteral("reviewDetailView"));
    m_detailView->setReadOnly(true);
    m_detailView->setPlaceholderText(tr("Select a finding to view details"));

    splitter->addWidget(m_findingListView);
    splitter->addWidget(m_detailView);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    splitter->setSizes({280, 500});

    // 底部统计栏
    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setObjectName(QStringLiteral("reviewSummaryLabel"));
    m_summaryLabel->setVisible(false);
    m_summaryLabel->setStyleSheet(
        QStringLiteral("color: #768390; font-size: 11px; padding: 2px 4px;"
                       "border-top: 1px solid #373E47;"));

    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(m_placeholderLabel);
    mainLayout->addWidget(splitter, 1);
    mainLayout->addWidget(m_summaryLabel);

    // 信号连接
    connect(m_findingListView, &QListView::clicked,
            this, &ReviewPanel::handleFindingClicked);
    connect(m_reviewStagedButton, &QPushButton::clicked,
            this, &ReviewPanel::handleReviewStagedClicked);
    connect(m_reviewWorkingTreeButton, &QPushButton::clicked,
            this, &ReviewPanel::handleReviewWorkingTreeClicked);
    connect(m_cancelButton, &QPushButton::clicked,
            this, &ReviewPanel::handleCancelClicked);
}
