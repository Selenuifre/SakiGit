#include "RebaseDialog.h"

#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

RebaseDialog::RebaseDialog(const QString& currentBranch,
                             const QStringList& branchNames,
                             QWidget* parent)
    : QDialog(parent),
      m_warningLabel(nullptr),
      m_currentBranchLabel(nullptr),
      m_branchCombo(nullptr),
      m_infoLabel(nullptr),
      m_riskAckCheck(nullptr),
      m_rebaseButton(nullptr),
      m_cancelButton(nullptr)
{
    setupUi(currentBranch, branchNames);
}

QString RebaseDialog::selectedBranch() const
{
    return m_branchCombo ? m_branchCombo->currentText() : QString();
}

bool RebaseDialog::riskAcknowledged() const
{
    return m_riskAckCheck ? m_riskAckCheck->isChecked() : false;
}

void RebaseDialog::setCurrentBranch(const QString& branchName)
{
    if (m_currentBranchLabel) {
        m_currentBranchLabel->setText(tr("Current branch: %1").arg(branchName));
    }
}

void RebaseDialog::setBranchNames(const QStringList& branchNames)
{
    if (!m_branchCombo) {
        return;
    }
    m_branchCombo->clear();
    m_branchCombo->addItems(branchNames);
}

void RebaseDialog::setupUi(const QString& currentBranch,
                             const QStringList& branchNames)
{
    setWindowTitle(tr("Rebase Branch"));
    setMinimumWidth(460);
    setModal(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // 标题
    auto* titleLabel = new QLabel(tr("Rebase Branch"), this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(titleFont.pointSize() + 2);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // 警告区域
    m_warningLabel = new QLabel(this);
    m_warningLabel->setWordWrap(true);
    m_warningLabel->setText(
        tr("<b>WARNING: Rebase rewrites commit history.</b><br><br>"
           "If this branch has already been pushed to a remote, "
           "you will need to force-push after rebase.<br><br>"
           "Only rebase branches that you alone are working on."));
    m_warningLabel->setStyleSheet(QStringLiteral(
        "background: #3D2E00; border: 1px solid #D29922; "
        "padding: 10px; border-radius: 4px; color: #E3B341;"));
    mainLayout->addWidget(m_warningLabel);

    // 当前分支显示
    m_currentBranchLabel = new QLabel(tr("Current branch: %1").arg(currentBranch), this);
    m_currentBranchLabel->setStyleSheet(QStringLiteral("color: #768390;"));
    mainLayout->addWidget(m_currentBranchLabel);

    // 基准分支选择
    auto* branchLayout = new QHBoxLayout;
    auto* branchLabel = new QLabel(tr("Rebase onto:"), this);
    m_branchCombo = new QComboBox(this);
    m_branchCombo->setEditable(false);
    m_branchCombo->setMinimumWidth(200);
    m_branchCombo->addItems(branchNames);
    branchLayout->addWidget(branchLabel);
    branchLayout->addWidget(m_branchCombo, 1);
    mainLayout->addLayout(branchLayout);

    // 信息提示区
    m_infoLabel = new QLabel(
        tr("The current branch's commits will be re-applied on top of the target branch."),
        this);
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setStyleSheet(QStringLiteral(
        "background: #2D333B; border: 1px solid #444C56; padding: 8px; border-radius: 4px; color: #ADBAC7;"));
    m_infoLabel->setMinimumHeight(36);
    mainLayout->addWidget(m_infoLabel);

    // 风险确认复选框
    m_riskAckCheck = new QCheckBox(
        tr("I understand the risks of rebase and want to proceed."), this);
    mainLayout->addWidget(m_riskAckCheck);

    // 按钮行
    auto* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();

    m_cancelButton = new QPushButton(tr("Cancel"), this);
    m_rebaseButton = new QPushButton(tr("Start Rebase"), this);
    m_rebaseButton->setEnabled(false);
    m_rebaseButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #DA3633; color: white; padding: 6px 20px; border-radius: 4px; }"
        "QPushButton:hover { background: #B62324; }"
        "QPushButton:disabled { background: #373E47; }"));

    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_rebaseButton);
    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(m_branchCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RebaseDialog::onBranchSelected);
    connect(m_riskAckCheck, &QCheckBox::toggled, this, [this]() {
        m_rebaseButton->setEnabled(m_branchCombo->currentIndex() >= 0
                                   && m_riskAckCheck->isChecked());
    });
    connect(m_rebaseButton, &QPushButton::clicked,
            this, &RebaseDialog::onRebaseClicked);
    connect(m_cancelButton, &QPushButton::clicked,
            this, &QDialog::reject);
}

void RebaseDialog::onBranchSelected(int index)
{
    Q_UNUSED(index)
    m_rebaseButton->setEnabled(m_branchCombo->currentIndex() >= 0
                               && m_riskAckCheck->isChecked());
}

void RebaseDialog::onRebaseClicked()
{
    const QString branch = selectedBranch();
    if (branch.isEmpty()) {
        return;
    }

    emit rebaseConfirmed(branch);
    accept();
}
