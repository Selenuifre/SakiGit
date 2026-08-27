#ifndef REBASEDIALOG_H
#define REBASEDIALOG_H

#include <QDialog>
#include <QString>
#include <QStringList>

class QComboBox;
class QLabel;
class QPushButton;
class QCheckBox;

class RebaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RebaseDialog(const QString& currentBranch,
                          const QStringList& branchNames,
                          QWidget* parent = nullptr);

    // 返回用户选择的基准分支名
    QString selectedBranch() const;

    // 返回用户是否确认了风险提示
    bool riskAcknowledged() const;

    // 设置当前分支名
    void setCurrentBranch(const QString& branchName);

    // 设置可选分支列表
    void setBranchNames(const QStringList& branchNames);

signals:
    // 用户确认变基
    void rebaseConfirmed(const QString& branchName);

private slots:
    void onBranchSelected(int index);
    void onRebaseClicked();

private:
    void setupUi(const QString& currentBranch,
                 const QStringList& branchNames);

    QLabel* m_warningLabel;
    QLabel* m_currentBranchLabel;
    QComboBox* m_branchCombo;
    QLabel* m_infoLabel;
    QCheckBox* m_riskAckCheck;
    QPushButton* m_rebaseButton;
    QPushButton* m_cancelButton;
};

#endif // REBASEDIALOG_H
