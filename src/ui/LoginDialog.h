#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include "domain/CodeHostingPlatform.h"

#include <QDialog>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget* parent = nullptr);

    QString token() const;
    CodeHostingPlatform selectedPlatform() const;

signals:
    void loginRequested(CodeHostingPlatform platform, const QString& token);

private slots:
    void handleLoginClicked();
    void handlePlatformChanged(int index);

private:
    void setupUi();
    void updateHelpText();
    void updateTokenPlaceholder();

    QComboBox* m_platformCombo;
    QLineEdit* m_tokenEdit;
    QLabel* m_infoLabel;
    QLabel* m_statusLabel;
    QPushButton* m_loginButton;
};

#endif // LOGINDIALOG_H
