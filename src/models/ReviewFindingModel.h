#ifndef REVIEWFINDINGMODEL_H
#define REVIEWFINDINGMODEL_H

#include "domain/ReviewFinding.h"

#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <QHash>
#include <QVector>

// AI Code Review 发现列表模型。
// 管理 AI 生成的 ReviewFinding 对象列表，供 UI 展示审查结果。
class ReviewFindingModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // 自定义数据角色
    enum Roles {
        FilePathRole      = Qt::UserRole + 1,
        LineNumberRole,
        SeverityRole,
        CategoryRole,
        TitleRole,
        MessageRole,
        SuggestionRole,
        CodeSnippetRole,
        IsAiGeneratedRole,
        SeverityWeightRole   // 用于排序，数字越大越严重
    };

    explicit ReviewFindingModel(QObject* parent = nullptr);

    // QAbstractListModel 接口
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 设置发现列表（替换全部已有项，按严重度排序）
    void setFindings(const std::vector<ReviewFinding>& findings);

    // 获取指定位置的发现
    ReviewFinding findingAt(int row) const;

    // 数量统计
    int totalCount() const;
    int criticalCount() const;
    int highCount() const;
    int mediumCount() const;
    int lowCount() const;
    int infoCount() const;

    // 是否有发现
    bool isEmpty() const;

    // 清空所有发现
    void clear();

private:
    QVector<ReviewFinding> m_findings;

    int countBySeverity(const QString& severity) const;
};

#endif // REVIEWFINDINGMODEL_H
