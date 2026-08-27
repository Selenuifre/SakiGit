#ifndef COMMITMESSAGESUGGESTIONMODEL_H
#define COMMITMESSAGESUGGESTIONMODEL_H

#include "domain/CommitMessageSuggestion.h"

#include <QAbstractListModel>
#include <QHash>
#include <QVector>

// 提交信息建议列表模型。
// 管理 AI 生成的 CommitMessageSuggestion 对象列表，
// 供 UI 展示备选方案或单一建议。
class CommitMessageSuggestionModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // 自定义数据角色
    enum Roles {
        FullMessageRole = Qt::UserRole + 1,
        TypeRole,
        ScopeRole,
        SubjectRole,
        BodyRole,
        RawResponseRole,
        IsValidRole
    };

    explicit CommitMessageSuggestionModel(QObject* parent = nullptr);

    // QAbstractListModel 接口
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 设置建议列表（替换全部已有项）
    void setSuggestions(const QVector<CommitMessageSuggestion>& suggestions);

    // 添加单个建议
    void addSuggestion(const CommitMessageSuggestion& suggestion);

    // 获取指定位置的建议
    CommitMessageSuggestion suggestionAt(int row) const;

    // 获取当前最佳建议（通常是第一个有效的）
    CommitMessageSuggestion bestSuggestion() const;

    // 当前建议数量
    int count() const;

    // 是否有建议
    bool isEmpty() const;

    // 清空所有建议
    void clear();

private:
    QVector<CommitMessageSuggestion> m_suggestions;
};

#endif // COMMITMESSAGESUGGESTIONMODEL_H
