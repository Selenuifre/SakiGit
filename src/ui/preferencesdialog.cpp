#include "preferencesdialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

PreferencesDialog::PreferencesDialog(QWidget* parent)
    : QDialog(parent),
    m_defaultRepoPathEdit(nullptr),
    m_userNameEdit(nullptr),
    m_userEmailEdit(nullptr),
    m_aiProviderCombo(nullptr),
    m_aiApiKeyEdit(nullptr),
    m_aiModelEdit(nullptr)
{
    setObjectName(QStringLiteral("preferencesDialog"));
    setWindowTitle(tr("Preferences"));
    resize(520, 420);

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    auto* formLayout = new QFormLayout;
    formLayout->setSpacing(8);

    // 默认仓库路径
    auto* pathLayout = new QHBoxLayout;
    m_defaultRepoPathEdit = new QLineEdit(this);
    m_defaultRepoPathEdit->setPlaceholderText(tr("Default directory for repositories"));
    pathLayout->addWidget(m_defaultRepoPathEdit, 1);

    auto* browseButton = new QPushButton(tr("Browse..."), this);
    browseButton->setFixedWidth(100);
    pathLayout->addWidget(browseButton);

    formLayout->addRow(tr("Default Repository Path:"), pathLayout);

    // Git 用户名
    m_userNameEdit = new QLineEdit(this);
    m_userNameEdit->setPlaceholderText(tr("Your Git user name"));
    formLayout->addRow(tr("Git User Name:"), m_userNameEdit);

    // Git 邮箱
    m_userEmailEdit = new QLineEdit(this);
    m_userEmailEdit->setPlaceholderText(tr("Your Git email address"));
    formLayout->addRow(tr("Git Email:"), m_userEmailEdit);

    // --- AI 配置分隔 ---
    auto* aiSeparator = new QLabel(tr("— AI Commit Message Settings —"), this);
    aiSeparator->setStyleSheet(QStringLiteral("font-weight: bold; margin-top: 8px;"));
    formLayout->addRow(aiSeparator);

    // AI Provider（可编辑下拉框：既可下拉选择，也可手动输入）
    m_aiProviderCombo = new QComboBox(this);
    m_aiProviderCombo->setEditable(true);
    m_aiProviderCombo->setInsertPolicy(QComboBox::NoInsert);
    m_aiProviderCombo->addItem(QStringLiteral("OpenAI"),           QStringLiteral("openai"));
    m_aiProviderCombo->addItem(QStringLiteral("Anthropic"),        QStringLiteral("anthropic"));
    m_aiProviderCombo->addItem(QStringLiteral("Azure OpenAI"),     QStringLiteral("azure"));
    m_aiProviderCombo->addItem(QStringLiteral("Google Gemini"),    QStringLiteral("gemini"));
    m_aiProviderCombo->addItem(QStringLiteral("DeepSeek"),         QStringLiteral("deepseek"));
    m_aiProviderCombo->addItem(QStringLiteral("Moonshot (Kimi)"),  QStringLiteral("moonshot"));
    m_aiProviderCombo->addItem(QStringLiteral("Zhipu (GLM)"),      QStringLiteral("zhipu"));
    m_aiProviderCombo->addItem(QStringLiteral("Qwen (Tongyi)"),    QStringLiteral("qwen"));
    m_aiProviderCombo->addItem(QStringLiteral("Ollama (Local)"),   QStringLiteral("ollama"));
    m_aiProviderCombo->addItem(QStringLiteral("Custom / Other"),   QStringLiteral("custom"));
    m_aiProviderCombo->setCurrentIndex(-1);
    m_aiProviderCombo->lineEdit()->setPlaceholderText(
        tr("Select or type provider (e.g., openai, anthropic, deepseek...)"));
    formLayout->addRow(tr("AI Provider:"), m_aiProviderCombo);

    // AI API Key
    m_aiApiKeyEdit = new QLineEdit(this);
    m_aiApiKeyEdit->setPlaceholderText(tr("Your AI API key (e.g., sk-...)"));
    m_aiApiKeyEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow(tr("AI API Key:"), m_aiApiKeyEdit);

    // AI Model
    m_aiModelEdit = new QLineEdit(this);
    m_aiModelEdit->setPlaceholderText(tr("Model name (e.g., gpt-4o, gpt-4o-mini, claude-opus-4-8)"));
    formLayout->addRow(tr("AI Model:"), m_aiModelEdit);

    layout->addLayout(formLayout);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);

    connect(browseButton, &QPushButton::clicked, this, &PreferencesDialog::handleBrowse);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PreferencesDialog::handleAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void PreferencesDialog::setDefaultRepositoryPath(const QString& path)
{
    m_defaultRepoPathEdit->setText(path.trimmed());
}

void PreferencesDialog::setGitUserName(const QString& name)
{
    m_userNameEdit->setText(name.trimmed());
}

void PreferencesDialog::setGitUserEmail(const QString& email)
{
    m_userEmailEdit->setText(email.trimmed());
}

QString PreferencesDialog::defaultRepositoryPath() const
{
    return m_defaultRepoPathEdit->text().trimmed();
}

QString PreferencesDialog::gitUserName() const
{
    return m_userNameEdit->text().trimmed();
}

QString PreferencesDialog::gitUserEmail() const
{
    return m_userEmailEdit->text().trimmed();
}

// --- AI 配置 getter/setter ---

void PreferencesDialog::setAIProvider(const QString& provider)
{
    if (!m_aiProviderCombo) return;
    const QString cleaned = provider.trimmed();
    // 先尝试按 data 匹配下拉项
    const int idx = m_aiProviderCombo->findData(cleaned);
    if (idx >= 0) {
        m_aiProviderCombo->setCurrentIndex(idx);
    } else {
        // 自定义值：直接设置编辑文本
        m_aiProviderCombo->setCurrentIndex(-1);
        m_aiProviderCombo->setEditText(cleaned);
    }
}

void PreferencesDialog::setAIApiKey(const QString& apiKey)
{
    if (m_aiApiKeyEdit) {
        m_aiApiKeyEdit->setText(apiKey.trimmed());
    }
}

void PreferencesDialog::setAIModel(const QString& model)
{
    if (m_aiModelEdit) {
        m_aiModelEdit->setText(model.trimmed());
    }
}

QString PreferencesDialog::aiProvider() const
{
    if (!m_aiProviderCombo) return {};
    // 优先返回 data（匹配预设项），若用户手动输入则为空
    const QString data = m_aiProviderCombo->currentData().toString();
    if (!data.isEmpty()) return data;
    return m_aiProviderCombo->currentText().trimmed();
}

QString PreferencesDialog::aiApiKey() const
{
    return m_aiApiKeyEdit
               ? m_aiApiKeyEdit->text().trimmed()
               : QString();
}

QString PreferencesDialog::aiModel() const
{
    return m_aiModelEdit
               ? m_aiModelEdit->text().trimmed()
               : QString();
}

void PreferencesDialog::handleBrowse()
{
    const QString dir = QFileDialog::getExistingDirectory(
        this, tr("Select Default Repository Path"),
        m_defaultRepoPathEdit->text());

    if (!dir.isEmpty()) {
        m_defaultRepoPathEdit->setText(dir);
    }
}

void PreferencesDialog::handleAccept()
{
    emit settingsChanged();
    accept();
}
