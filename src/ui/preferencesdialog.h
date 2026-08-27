#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

class QComboBox;
class QLineEdit;
class QPushButton;

// 设置对话框——编辑 Git 用户名、邮箱、默认仓库路径和 AI 配置。
// 规范接口：setSettings(SettingsService*) 的替代：
//   使用 setter 来预填值，信号 settingsChanged() 通知外部保存
class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget* parent = nullptr);

    // 预填各字段值
    void setDefaultRepositoryPath(const QString& path);
    void setGitUserName(const QString& name);
    void setGitUserEmail(const QString& email);
    void setAIProvider(const QString& provider);
    void setAIApiKey(const QString& apiKey);
    void setAIModel(const QString& model);

    // 读取各字段值
    QString defaultRepositoryPath() const;
    QString gitUserName() const;
    QString gitUserEmail() const;
    QString aiProvider() const;
    QString aiApiKey() const;
    QString aiModel() const;

signals:
    // 用户点击保存后发出
    void settingsChanged();

private slots:
    void handleBrowse();
    void handleAccept();

private:
    QLineEdit* m_defaultRepoPathEdit;
    QLineEdit* m_userNameEdit;
    QLineEdit* m_userEmailEdit;
    QComboBox* m_aiProviderCombo;
    QLineEdit* m_aiApiKeyEdit;
    QLineEdit* m_aiModelEdit;
};

#endif // PREFERENCESDIALOG_H
