#include "clonedialog.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

CloneDialog::CloneDialog(QWidget* parent)
    : QDialog(parent),
    m_urlEdit(nullptr),
    m_targetPathEdit(nullptr),
    m_okButton(nullptr)
{
    setObjectName(QStringLiteral("cloneDialog"));
    setWindowTitle(tr("Clone Repository"));
    resize(520, 200);

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    auto* formLayout = new QFormLayout;
    formLayout->setSpacing(8);

    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setPlaceholderText(tr("https://github.com/user/repo.git"));
    formLayout->addRow(tr("Repository URL:"), m_urlEdit);

    auto* pathLayout = new QHBoxLayout;
    m_targetPathEdit = new QLineEdit(this);
    m_targetPathEdit->setPlaceholderText(tr("Select local target directory"));
    pathLayout->addWidget(m_targetPathEdit, 1);

    auto* browseButton = new QPushButton(tr("Browse..."), this);
    browseButton->setFixedWidth(100);
    pathLayout->addWidget(browseButton);

    formLayout->addRow(tr("Target Path:"), pathLayout);

    layout->addLayout(formLayout);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_okButton = buttonBox->button(QDialogButtonBox::Ok);
    m_okButton->setEnabled(false);
    layout->addWidget(buttonBox);

    // 校验：URL 和路径均非空时启用 OK
    auto enableOk = [this]() {
        m_okButton->setEnabled(
            !m_urlEdit->text().trimmed().isEmpty()
            && !m_targetPathEdit->text().trimmed().isEmpty());
    };

    connect(m_urlEdit, &QLineEdit::textChanged, this, enableOk);
    connect(m_targetPathEdit, &QLineEdit::textChanged, this, enableOk);
    connect(browseButton, &QPushButton::clicked, this, &CloneDialog::handleBrowse);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &CloneDialog::handleAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void CloneDialog::setDefaultPath(const QString& path)
{
    if (!path.isEmpty() && m_targetPathEdit->text().isEmpty()) {
        m_targetPathEdit->setText(path);
    }
}

QString CloneDialog::url() const
{
    return m_urlEdit->text().trimmed();
}

void CloneDialog::setUrl(const QString& url)
{
    m_urlEdit->setText(url.trimmed());
}

QString CloneDialog::targetPath() const
{
    return m_targetPathEdit->text().trimmed();
}

void CloneDialog::setTargetPath(const QString& targetPath)
{
    m_targetPathEdit->setText(targetPath.trimmed());
}

void CloneDialog::handleAccept()
{
    const QString cleanUrl = url();
    const QString cleanPath = targetPath();

    if (cleanUrl.isEmpty() || cleanPath.isEmpty()) {
        QMessageBox::warning(this, tr("Incomplete"),
                             tr("Please enter both a repository URL and a target path."));
        return;
    }

    emit cloneRequested(cleanUrl, cleanPath);
    accept();
}

void CloneDialog::handleBrowse()
{
    const QString dir = QFileDialog::getExistingDirectory(
        this, tr("Select Target Directory"), m_targetPathEdit->text());

    if (!dir.isEmpty()) {
        m_targetPathEdit->setText(dir);
    }
}
