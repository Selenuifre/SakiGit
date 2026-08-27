#ifndef MERGEDIALOG_H
#define MERGEDIALOG_H

#include <QDialog>
#include <QString>
#include <QStringList>

class QComboBox;
class QLabel;
class QPushButton;
class QCheckBox;

class MergeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MergeDialog(const QString& currentBranch,
                         const QStringList& branchNames,
                         QWidget* parent = nullptr);

    // 返回用户选择的目标分支名
    QString selectedBranch() const;

    // 返回是否勾选了"自动暂存"选项
    bool autoStash() const;

    // 设置当前分支名
    void setCurrentBranch(const QString& branchName);

    // 设置可选分支列表
    void setBranchNames(const QStringList& branchNames);

    // 设置操作结果信息
    void setResultInfo(const QString& text);

signals:
    // 用户确认合并
    void mergeConfirmed(const QString& branchName);

private slots:
    void onBranchSelected(int index);
    void onMergeClicked();

private:
    void setupUi(const QString& currentBranch,
                 const QStringList& branchNames);

    QLabel* m_titleLabel;
    QLabel* m_currentBranchLabel;
    QComboBox* m_branchCombo;
    QLabel* m_infoLabel;
    QCheckBox* m_autoStashCheck;
    QPushButton* m_mergeButton;
    QPushButton* m_cancelButton;
};

#endif // MERGEDIALOG_H
