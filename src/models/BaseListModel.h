#ifndef BASELISTMODEL_H
#define BASELISTMODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <QModelIndex>
#include <QVariant>
#include <vector>

// 通用列表模型基类，消除所有简单列表模型中的 rowCount/clear/itemAt/setItems 样板代码。
// 子类只需实现 roleNames() 和 dataForRole() 即可获得完整的 QAbstractListModel 实现。
//
// 用法示例：
//   class MyModel : public BaseListModel<MyItem> {
//       QHash<int, QByteArray> roleNames() const override { ... }
//       QVariant dataForRole(const MyItem& item, int role) const override { ... }
//   };

template <typename T, typename Container = std::vector<T>>
class BaseListModel : public QAbstractListModel
{
public:
    explicit BaseListModel(QObject* parent = nullptr)
        : QAbstractListModel(parent)
    {
    }

    // ---- QAbstractListModel 接口 ----

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid()) {
            return 0;
        }
        return static_cast<int>(m_items.size());
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid() || index.row() < 0
            || index.row() >= static_cast<int>(m_items.size())) {
            return {};
        }
        return dataForRole(m_items.at(index.row()), role);
    }

    // ---------- 子类必须实现 ----------

    virtual QHash<int, QByteArray> roleNames() const override = 0;

    // 将单个数据项转换为指定角色的 QVariant；DisplayRole 同样走此路径
    virtual QVariant dataForRole(const T& item, int role) const = 0;

    // ---------- 数据操作（基类实现） ----------

    void setItems(const Container& items)
    {
        beginResetModel();
        m_items = items;
        endResetModel();
        onItemsChanged();
    }

    T itemAt(int row) const
    {
        if (row < 0 || row >= static_cast<int>(m_items.size())) {
            return T();
        }
        return m_items.at(row);
    }

    void clear()
    {
        if (m_items.empty()) {
            return;
        }
        beginResetModel();
        m_items.clear();
        endResetModel();
        onItemsChanged();
    }

    int count() const { return static_cast<int>(m_items.size()); }
    bool isEmpty() const { return m_items.empty(); }

    // ---------- 保护接口 ----------

protected:
    Container m_items;

    // 子类可覆盖此方法以在数据变更时发出自定义信号
    virtual void onItemsChanged() {}
};

#endif // BASELISTMODEL_H
