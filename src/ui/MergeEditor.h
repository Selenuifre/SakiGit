#ifndef MERGEEDITOR_H
#define MERGEEDITOR_H

#include <QWidget>

class ConflictResolver;

// 合并编辑器——包装 ConflictResolver，提供统一的三方合并编辑视图
// 命名遵循二五计划 Phase 4 规范
class MergeEditor : public QWidget
{
    Q_OBJECT

public:
    explicit MergeEditor(QWidget* parent = nullptr);

    // 加载冲突文件
    void loadFile(const QString& repoPath, const QString& filePath);

    // 清空
    void clear();

    // 访问内部 ConflictResolver
    ConflictResolver* conflictResolver() const;

signals:
    void markResolvedRequested(const QString& repoPath, const QString& filePath);

private:
    void setupUi();

    ConflictResolver* m_conflictResolver;
};

#endif // MERGEEDITOR_H
