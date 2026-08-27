#ifndef UI_UTILS_H
#define UI_UTILS_H

#include <QListView>
#include <QString>

// 创建具有标准配置的 QListView（交替行颜色、无编辑触发、自定义上下文菜单）。
// 提取自 changespage.cpp 匿名命名空间中的 createChangeListView()。
inline QListView* createListView(QWidget* parent, const QString& objectName)
{
    auto* listView = new QListView(parent);
    listView->setObjectName(objectName);
    listView->setAlternatingRowColors(true);
    listView->setEditTriggers(QListView::NoEditTriggers);
    listView->setContextMenuPolicy(Qt::CustomContextMenu);
    return listView;
}

#endif // UI_UTILS_H
