#include "MergeDialog.h"

#include <QComboBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

MergeDialog::MergeDialog(const QString& currentBranch,
                           const QStringList& branchNames,
                           QWidget* parent)
    : QDialog(parent),
      m_titleLabel(nullptr),
      m_currentBranchLabel(nullptr),
      m_branchCombo(nullptr),
      m_infoLabel(nullptr),
      m_autoStashCheck(nullptr),
      m_mergeButton(nullptr),
      m_cancelButton(nullptr)
{
    setupUi(currentBranch, branchNames);
}

QString MergeDialog::selectedBranch() const
{
    return m_branchCombo ? m_branchCombo->currentText() : QString();
}

bool MergeDialog::autoStash() const
{
    return m_autoStashCheck ? m_autoStashCheck->isChecked() : false;
}

void MergeDialog::setCurrentBranch(const QString& branchName)
{
    if (m_currentBranchLabel) {
        m_currentBranchLabel->setText(tr("Current branch: %1").arg(branchName));
    }
}

void MergeDialog::setBranchNames(const QStringList& branchNames)
{
    if (!m_branchCombo) {
        return;
    }
    m_branchCombo->clear();
    m_branchCombo->addItems(branchNames);
    m_mergeButton->setEnabled(m_branchCombo->count() > 0);
}

void MergeDialog::setResultInfo(const QString& text)
{
    if (m_infoLabel) {
        m_infoLabel->setText(text);
    }
}

void MergeDialog::setupUi(const QString& currentBranch,
                            const QStringList& branchNames)
{
    setWindowTitle(tr("Merge Branch"));
    setMinimumWidth(420);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // 标题
    m_titleLabel = new QLabel(tr("Merge Branch"), this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(titleFont.pointSize() + 2);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    mainLayout->addWidget(m_titleLabel);

    // 当前分支显示
    m_currentBranchLabel = new QLabel(tr("Current branch: %1").arg(currentBranch), this);
    m_currentBranchLabel->setStyleSheet(QStringLiteral("color: #768390;"));
    mainLayout->addWidget(m_currentBranchLabel);

    // 目标分支选择
    auto* branchLayout = new QHBoxLayout;
    auto* branchLabel = new QLabel(tr("Merge from branch:"), this);
    m_branchCombo = new QComboBox(this);
    m_branchCombo->setEditable(false);
    m_branchCombo->setMinimumWidth(200);
    m_branchCombo->addItems(branchNames);
    branchLayout->addWidget(branchLabel);
    branchLayout->addWidget(m_branchCombo, 1);
    mainLayout->addLayout(branchLayout);

    // 信息提示区
    m_infoLabel = new QLabel(this);
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setStyleSheet(QStringLiteral(
        "background: #2D333B; border: 1px solid #444C56; padding: 8px; border-radius: 4px; color: #ADBAC7;"));
    m_infoLabel->setMinimumHeight(40);
    mainLayout->addWidget(m_infoLabel);

    // 自动暂存选项
    m_autoStashCheck = new QCheckBox(tr("Auto-stash local changes before merge"), this);
    mainLayout->addWidget(m_autoStashCheck);

    // 按钮行
    auto* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();

    m_cancelButton = new QPushButton(tr("Cancel"), this);
    m_mergeButton = new QPushButton(tr("Merge"), this);
    m_mergeButton->setDefault(true);
    m_mergeButton->setEnabled(m_branchCombo->count() > 0);
    m_mergeButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #2DA44E; color: white; padding: 6px 20px; border-radius: 4px; }"
        "QPushButton:hover { background: #347D39; }"
        "QPushButton:disabled { background: #373E47; }"));

    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_mergeButton);
    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(m_branchCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MergeDialog::onBranchSelected);
    connect(m_mergeButton, &QPushButton::clicked,
            this, &MergeDialog::onMergeClicked);
    connect(m_cancelButton, &QPushButton::clicked,
            this, &QDialog::reject);
}

void MergeDialog::onBranchSelected(int index)
{
    Q_UNUSED(index)
    m_mergeButton->setEnabled(m_branchCombo->currentIndex() >= 0);
}

void MergeDialog::onMergeClicked()
{
    const QString branch = selectedBranch();
    if (branch.isEmpty()) {
        return;
    }

    emit mergeConfirmed(branch);
    accept();
}
