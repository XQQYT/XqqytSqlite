#include "table_data_model.h"

namespace remote_sqlite_qt {

TableDataModel::TableDataModel(QObject* parent)
    : QAbstractTableModel(parent) {}

int TableDataModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(rows_.size());
}

int TableDataModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(columns_.size());
}

QVariant TableDataModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() ||
        index.row() >= static_cast<int>(rows_.size()) ||
        index.column() >= static_cast<int>(columns_.size()))
        return {};

    if (role == Qt::DisplayRole) {
        const auto& row = rows_[static_cast<size_t>(index.row())];
        return QString::fromStdString(
            row[static_cast<size_t>(index.column())]);
    }
    return {};
}

QVariant TableDataModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const {
    if (role != Qt::DisplayRole) return {};
    if (orientation == Qt::Horizontal) {
        if (section < static_cast<int>(columns_.size()))
            return QString::fromStdString(columns_[static_cast<size_t>(section)]);
    } else {
        return section + 1;  // 行号从 1 开始
    }
    return {};
}

void TableDataModel::loadQueryResult(const QueryResult& result) {
    beginResetModel();
    columns_ = result.columns;
    rows_ = result.rows;
    totalRows_ = result.totalRows;
    currentPage_ = 0;
    endResetModel();
    emit dataChanged();
}

void TableDataModel::clear() {
    beginResetModel();
    columns_.clear();
    rows_.clear();
    totalRows_ = 0;
    currentPage_ = 0;
    endResetModel();
    emit dataChanged();
}

}  // namespace remote_sqlite_qt
