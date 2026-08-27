#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include <QObject>
#include <QString>

// 所有控制器的公共基类。
// 提供共享信号（errorOccurred、operationFinished）和 clear() 虚方法，
// 消除每个控制器中独立声明相同信号的冗余。

class BaseController : public QObject
{
    Q_OBJECT

public:
    explicit BaseController(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    // 清除关联的 Model 数据；子类覆盖以调用各自 Model 的 clear()
    virtual void clear() {}

signals:
    // 通用错误信号 —— 所有控制器共用
    void errorOccurred(const QString& operation, const QString& errorMessage);

    // 通用操作完成信号 —— 多数控制器共用
    void operationFinished(const QString& operation, bool success,
                           const QString& errorMessage);
};

#endif // BASECONTROLLER_H
