#ifndef REVIEWPANEL_H
#define REVIEWPANEL_H

#include <QWidget>

class QAbstractItemModel;
class QLabel;
class QListView;
class QPushButton;
class QTextEdit;

// AI Code Review 结果展示面板。
// 以列表形式展示审查发现，支持按严重度过滤和详情查看。
class ReviewPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ReviewPanel(QWidget* parent = nullptr);

    // 设置发现列表数据源
    void setModel(QAbstractItemModel* model);
    QAbstractItemModel* model() const;

    // 返回列表视图
    QListView* listView() const;

    // 设置加载状态
    void setReviewing(bool reviewing);

    // 设置底部统计文本
    void setSummaryText(const QString& text);

    // 设置占位文本（未开始审查时显示）
    void setPlaceholderText(const QString& text);

signals:
    // 用户请求开始审查暂存区变更
    void reviewStagedRequested();

    // 用户请求开始审查工作区变更
    void reviewWorkingTreeRequested();

    // 用户请求取消审查
    void cancelRequested();

    // 用户点击某个发现，请求导航到对应文件/行
    void findingActivated(const QString& filePath, int lineNumber);

private slots:
    void handleFindingClicked(const QModelIndex& index);
    void handleReviewStagedClicked();
    void handleReviewWorkingTreeClicked();
    void handleCancelClicked();

private:
    void setupUi();

    QListView*   m_findingListView;
    QPushButton* m_reviewStagedButton;
    QPushButton* m_reviewWorkingTreeButton;
    QPushButton* m_cancelButton;
    QLabel*      m_placeholderLabel;
    QLabel*      m_summaryLabel;
    QTextEdit*   m_detailView;
    QAbstractItemModel* m_model;
};

#endif // REVIEWPANEL_H
