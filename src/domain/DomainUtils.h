#ifndef DOMAINUTILS_H
#define DOMAINUTILS_H

#include <QString>
#include <QStringList>

// 域层通用内联工具函数。
// 消除所有域对象 setter 和 display*() 方法中的重复模式。

// 修剪后赋值 —— 替代所有域 setter 中的 m_x = x.trimmed()
inline void assignTrimmed(QString& member, const QString& value)
{
    member = value.trimmed();
}

// 值或默认值回退 —— 替代 display*() 方法中重复的 if-empty-return-default 模式
inline QString valueOrDefault(const QString& value, const QString& defaultValue)
{
    return value.isEmpty() ? defaultValue : value;
}

// 去重修剪后添加 —— 仅当条目非空且不存在于列表中时才添加
// 替代 Commit::addParentHash() 和 Commit::addChangedFile() 中的重复逻辑
inline void addUniqueTrimmed(QStringList& list, const QString& item)
{
    const QString clean = item.trimmed();
    if (!clean.isEmpty() && !list.contains(clean)) {
        list.append(clean);
    }
}

#endif // DOMAINUTILS_H
