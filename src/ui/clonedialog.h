#ifndef CLONEDIALOG_H
#define CLONEDIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;

// 克隆仓库对话框——输入远程 URL 和目标路径即可克隆。
// 规范接口：setDefaultPath(path), 信号 cloneRequested(url, targetPath)
class CloneDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CloneDialog(QWidget* parent = nullptr);

    // 预填默认克隆目标路径
    void setDefaultPath(const QString& path);

    // 获取/设置 URL
    QString url() const;
    void setUrl(const QString& url);

    // 获取/设置目标路径
    QString targetPath() const;
    void setTargetPath(const QString& targetPath);

signals:
    // 用户确认克隆
    void cloneRequested(const QString& url, const QString& targetPath);

private slots:
    void handleAccept();
    void handleBrowse();

private:
    QLineEdit* m_urlEdit;
    QLineEdit* m_targetPathEdit;
    QPushButton* m_okButton;
};

#endif // CLONEDIALOG_H
