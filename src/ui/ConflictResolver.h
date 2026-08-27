#ifndef CONFLICTRESOLVER_H
#define CONFLICTRESOLVER_H

#include <QWidget>

class QLabel;
class QPlainTextEdit;
class QPushButton;
class QToolBar;

// 冲突文件查看与编辑器。
// 读取包含 <<<<<<< / ======= / >>>>>>> 标记的冲突文件，
// 以颜色高亮显示冲突区域，支持直接编辑解决冲突。
class ConflictResolver : public QWidget
{
    Q_OBJECT

public:
    explicit ConflictResolver(QWidget* parent = nullptr);

    // 加载冲突文件内容
    void loadFile(const QString& repoPath, const QString& filePath);

    // 清空当前显示
    void clear();

    // 返回当前文件路径
    QString currentFilePath() const;

    // 设置空数据提示文本
    void setPlaceholderText(const QString& text);

signals:
    // 用户保存了冲突解决结果
    void conflictResolved(const QString& repoPath, const QString& filePath);

    // 用户请求标记文件为已解决 (git add)
    void markResolvedRequested(const QString& repoPath, const QString& filePath);

private slots:
    void onAcceptOurs();
    void onAcceptTheirs();
    void onSave();
    void onMarkResolved();

private:
    void setupUi();

    // 解析并高亮冲突标记
    void applyConflictHighlighting();

    // 获取当前编辑器的纯文本内容
    QString editorContent() const;

    // 替换所有 Ours 冲突块为 Ours 内容
    void replaceAllWithOurs();

    // 替换所有 Theirs 冲突块为 Theirs 内容
    void replaceAllWithTheirs();

    QLabel* m_filePathLabel;
    QLabel* m_statusLabel;
    QPlainTextEdit* m_editor;
    QToolBar* m_toolBar;
    QPushButton* m_acceptOursButton;
    QPushButton* m_acceptTheirsButton;
    QPushButton* m_saveButton;
    QPushButton* m_markResolvedButton;
    QWidget* m_placeholderWidget;

    QString m_repoPath;
    QString m_filePath;
};

#endif // CONFLICTRESOLVER_H
