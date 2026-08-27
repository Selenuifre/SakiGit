#ifndef BASEKEYEDTABLEMODEL_H
#define BASEKEYEDTABLEMODEL_H

#include "BaseTableModel.h"

#include <algorithm>
#include <functional>

// 在 BaseTableModel 之上叠加键控操作（add/update/upsert/remove/removeAt/indexOf/contains）。
// 子类需额外实现 keyForItem(const T&)，用于去重和查找。
//
// KeyType：键类型（通常为 QString）

template <typename T, typename KeyType, int ColCount>
class BaseKeyedTableModel : public BaseTableModel<T, ColCount>
{
public:
    using Base = BaseTableModel<T, ColCount>;
    using Base::m_items;
    using Base::isValidRow;
    using Base::emitItemChanged;
    using Base::onItemsChanged;

    explicit BaseKeyedTableModel(const typename Base::ColumnHeaders& headers,
                                 QObject* parent = nullptr)
        : Base(headers, parent)
    {
    }

    // ---------- 子类必须实现 ----------

    virtual KeyType keyForItem(const T& item) const = 0;

    // ---------- 去重插入 ----------

    void setItems(const std::vector<T>& items)
    {
        Base::beginResetModel();
        m_items.clear();
        for (const auto& item : items) {
            if (!item.isValid()) {
                continue;
            }
            const int existingRow = indexOf(keyForItem(item));
            if (existingRow >= 0) {
                m_items[existingRow] = item;
            } else {
                m_items.push_back(item);
            }
        }
        Base::endResetModel();
        onItemsChanged();
    }

    void addItem(const T& item)
    {
        if (!item.isValid()) {
            return;
        }

        const int existingRow = indexOf(keyForItem(item));
        if (existingRow >= 0) {
            m_items[existingRow] = item;
            emitItemChanged(existingRow);
            return;
        }

        const int row = static_cast<int>(m_items.size());
        Base::beginInsertRows(QModelIndex(), row, row);
        m_items.push_back(item);
        Base::endInsertRows();

        onItemsChanged();
    }

    bool updateItem(const T& item)
    {
        const int row = indexOf(keyForItem(item));
        if (row < 0) {
            return false;
        }

        m_items[row] = item;
        emitItemChanged(row);
        return true;
    }

    // 存在则更新，不存在则新增；返回 true 表示更新了已有项
    bool upsertItem(const T& item)
    {
        const int row = indexOf(keyForItem(item));
        if (row >= 0) {
            m_items[row] = item;
            emitItemChanged(row);
            return true;
        }

        addItem(item);
        return false;
    }

    bool removeItemAt(int row)
    {
        if (!isValidRow(row)) {
            return false;
        }

        Base::beginRemoveRows(QModelIndex(), row, row);
        m_items.erase(m_items.begin() + row);
        Base::endRemoveRows();

        onItemsChanged();
        return true;
    }

    bool removeItem(const KeyType& key)
    {
        const int row = indexOf(key);
        if (row < 0) {
            return false;
        }
        return removeItemAt(row);
    }

    int indexOf(const KeyType& key) const
    {
        for (int row = 0; row < static_cast<int>(m_items.size()); ++row) {
            if (keyForItem(m_items.at(row)) == key) {
                return row;
            }
        }
        return -1;
    }

    bool contains(const KeyType& key) const
    {
        return indexOf(key) >= 0;
    }
};

#endif // BASEKEYEDTABLEMODEL_H
