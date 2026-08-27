#ifndef TERMINALOUTPUTMODEL_H
#define TERMINALOUTPUTMODEL_H

#include "BaseListModel.h"
#include "domain/CommandLogEntry.h"

#include <QColor>
#include <QHash>
#include <vector>

// 终端输出行模型，显示命令执行日志（时间戳 + 命令行 + 输出）。
// 自定义角色支持前景色区分：用户输入(蓝色)、GUI操作(灰色)、失败输出(红色)。
class TerminalOutputModel : public BaseListModel<CommandLogEntry>
{
    Q_OBJECT

public:
    enum Role {
        DisplayLineRole = Qt::UserRole + 1,   // 带时间戳的命令行
        OutputRole,                            // stdout/stderr 文本
        ExitCodeRole,                          // 退出码 (int)
        IsUserInputRole,                       // 是否为手动输入 (bool)
        TextColorRole,                         // 前景色 (QColor)
    };

    explicit TerminalOutputModel(QObject* parent = nullptr);

    // BaseListModel 接口
    QHash<int, QByteArray> roleNames() const override;
    QVariant dataForRole(const CommandLogEntry& entry, int role) const override;

    // 增量追加（触发 beginInsertRows，单行渲染）
    void appendEntry(const CommandLogEntry& entry);

    // 获取指定行
    CommandLogEntry entryAt(int row) const;

signals:
    void entryAppended();
};

#endif // TERMINALOUTPUTMODEL_H
