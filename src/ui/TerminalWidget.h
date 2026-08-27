#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QWidget>

class QAbstractItemModel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

// 终端窗口组件：上区命令输出日志 + 下区命令输入行。
// 输出区通过绑定 TerminalOutputModel 自动刷新；每行显示时间戳 + 命令行 + 执行结果。
class TerminalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalWidget(QWidget* parent = nullptr);

    // 绑定数据模型（TerminalOutputModel）
    void setModel(QAbstractItemModel* model);
    QAbstractItemModel* model() const;

    // 清空输出区
    void clearOutput();

    // 设置/获取输入区提示文本
    void setInputPlaceholder(const QString& text);
    QString inputText() const;

signals:
    void commandEntered(const QString& command);
    void clearRequested();

private slots:
    void handleExecute();
    void onRowsInserted(const QModelIndex& parent, int first, int last);

private:
    void setupUi();
    void appendOutput(const QString& line);

    QPlainTextEdit* m_outputView;
    QLineEdit* m_inputEdit;
    QPushButton* m_executeButton;
    QPushButton* m_clearButton;
    QAbstractItemModel* m_model = nullptr;
};

#endif // TERMINALWIDGET_H
