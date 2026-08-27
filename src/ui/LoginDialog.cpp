#include "LoginDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent),
    m_platformCombo(nullptr),
    m_tokenEdit(nullptr),
    m_infoLabel(nullptr),
    m_statusLabel(nullptr),
    m_loginButton(nullptr)
{
    setupUi();
    updateHelpText();
    updateTokenPlaceholder();
}

QString LoginDialog::token() const
{
    return m_tokenEdit->text().trimmed();
}

CodeHostingPlatform LoginDialog::selectedPlatform() const
{
    const int idx = m_platformCombo ? m_platformCombo->currentIndex() : 0;
    switch (idx) {
    case 1:  return CodeHostingPlatform::Gitee;
    case 2:  return CodeHostingPlatform::GitLab;
    default: return CodeHostingPlatform::GitHub;
    }
}

void LoginDialog::setupUi()
{
    setObjectName(QStringLiteral("loginDialog"));
    setWindowTitle(tr("Code Hosting Login"));
    setMinimumWidth(500);
    resize(520, 280);

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    // 平台选择
    auto* platformLayout = new QHBoxLayout;
    auto* platformLabel = new QLabel(tr("Platform:"), this);
    m_platformCombo = new QComboBox(this);
    m_platformCombo->addItem(QStringLiteral("GitHub"));
    m_platformCombo->addItem(QStringLiteral("Gitee"));
    m_platformCombo->addItem(QStringLiteral("GitLab"));
    platformLayout->addWidget(platformLabel);
    platformLayout->addWidget(m_platformCombo, 1);

    // 帮助信息
    m_infoLabel = new QLabel(this);
    m_infoLabel->setWordWrap(true);

    // Token 输入
    auto* formLayout = new QFormLayout;
    m_tokenEdit = new QLineEdit(this);
    m_tokenEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow(tr("Token:"), m_tokenEdit);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setVisible(false);

    auto* buttonBox = new QDialogButtonBox(this);
    m_loginButton = buttonBox->addButton(tr("Login"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(QDialogButtonBox::Cancel);

    layout->addLayout(platformLayout);
    layout->addWidget(m_infoLabel);
    layout->addLayout(formLayout);
    layout->addWidget(m_statusLabel);
    layout->addWidget(buttonBox);

    connect(m_platformCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LoginDialog::handlePlatformChanged);
    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::handleLoginClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void LoginDialog::handlePlatformChanged(int /*index*/)
{
    updateHelpText();
    updateTokenPlaceholder();
    m_statusLabel->setVisible(false);
}

void LoginDialog::handleLoginClicked()
{
    const QString t = token();
    if (t.isEmpty()) {
        m_statusLabel->setText(tr("Please enter a token."));
        m_statusLabel->setStyleSheet(QStringLiteral("color: #F47067;"));
        m_statusLabel->setVisible(true);
        return;
    }

    emit loginRequested(selectedPlatform(), t);
}

void LoginDialog::updateHelpText()
{
    switch (selectedPlatform()) {
    case CodeHostingPlatform::GitHub:
        m_infoLabel->setText(tr(
            "Enter your GitHub Personal Access Token to log in.\n"
            "Create one at GitHub → Settings → Developer settings → Personal access tokens.\n"
            "Required scopes: repo, read:user."));
        break;
    case CodeHostingPlatform::Gitee:
        m_infoLabel->setText(tr(
            "Enter your Gitee Personal Access Token to log in.\n"
            "Create one at Gitee → Settings → Private Tokens.\n"
            "Required scopes: projects, pull_requests, user_info."));
        break;
    case CodeHostingPlatform::GitLab:
        m_infoLabel->setText(tr(
            "Enter your GitLab Personal Access Token to log in.\n"
            "Create one at GitLab → Settings → Access Tokens.\n"
            "Required scopes: api, read_user."));
        break;
    default:
        break;
    }
}

void LoginDialog::updateTokenPlaceholder()
{
    switch (selectedPlatform()) {
    case CodeHostingPlatform::GitHub:
        m_tokenEdit->setPlaceholderText(tr("ghp_xxxxxxxxxxxxxxxxxxxx"));
        break;
    case CodeHostingPlatform::Gitee:
        m_tokenEdit->setPlaceholderText(tr("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"));
        break;
    case CodeHostingPlatform::GitLab:
        m_tokenEdit->setPlaceholderText(tr("glpat-xxxxxxxxxxxxxxxxxxxx"));
        break;
    default:
        break;
    }
}
