#ifndef BASETABLEMODEL_H
#define BASETABLEMODEL_H

#include <QAbstractTableModel>
#include <QHash>
#include <QModelIndex>
#include <QVariant>
#include <array>
#include <vector>

// 通用表格模型基类，消除 rowCount/columnCount/headerData/flags/isValidRow/clear 样板。
// 子类需实现 roleNames() 和 dataForRole()。
//
// ColCount：编译期列数常量
// 列标题通过构造函数以 std::array<QString, ColCount> 传入

template <typename T, int ColCount>
class BaseTableModel : public QAbstractTableModel
{
public:
    using ColumnHeaders = std::array<QString, ColCount>;

    explicit BaseTableModel(const ColumnHeaders& headers, QObject* parent = nullptr)
        : QAbstractTableModel(parent), m_columnHeaders(headers)
    {
    }

    // ---- QAbstractTableModel 接口 ----

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid()) {
            return 0;
        }
        return static_cast<int>(m_items.size());
    }

    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid()) {
            return 0;
        }
        return ColCount;
    }

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override
    {
        if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
            return {};
        }
        if (section < 0 || section >= ColCount) {
            return {};
        }
        return m_columnHeaders[section];
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        if (!index.isValid() || !isValidRow(index.row())) {
            return Qt::NoItemFlags;
        }
        return baseFlags();
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid() || !isValidRow(index.row())) {
            return {};
        }
        return dataForRole(m_items.at(index.row()), role, index.column());
    }

    // ---------- 子类必须实现 ----------

    virtual QHash<int, QByteArray> roleNames() const override = 0;

    // index.column() 可用于区分不同列的 DisplayRole
    virtual QVariant dataForRole(const T& item, int role, int column) const = 0;

    // ---------- 数据操作 ----------

    void setItems(const std::vector<T>& items)
    {
        beginResetModel();
        m_items = items;
        endResetModel();
        onItemsChanged();
    }

    T itemAt(int row) const
    {
        if (!isValidRow(row)) {
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

    // ---------- 受保护工具 ----------

protected:
    std::vector<T> m_items;
    ColumnHeaders m_columnHeaders;

    bool isValidRow(int row) const
    {
        return row >= 0 && row < static_cast<int>(m_items.size());
    }

    // 发出 dataChanged 信号（含自定义信号转发）；子类覆盖以追加自定义信号
    virtual void emitItemChanged(int row)
    {
        if (!isValidRow(row)) {
            return;
        }
        const QModelIndex topLeft = index(row, 0);
        const QModelIndex bottomRight = index(row, ColCount - 1);
        emit dataChanged(topLeft, bottomRight);
    }

    // 子类可覆盖以添加额外标志（如 ItemIsUserCheckable）
    virtual Qt::ItemFlags baseFlags() const
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }

    // 子类可覆盖以在数据变更时发出自定义信号
    virtual void onItemsChanged() {}
};

#endif // BASETABLEMODEL_H
