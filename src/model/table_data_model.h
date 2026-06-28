#pragma once

#include <QAbstractTableModel>
#include <QString>
#include <vector>
#include <string>

#include "driver_interface/types.h"

namespace remote_sqlite_qt {

// 表数据 Model——支持分页显示
class TableDataModel : public QAbstractTableModel {
    Q_OBJECT

    Q_PROPERTY(int totalRows READ totalRows NOTIFY dataChanged)
    Q_PROPERTY(int currentPage READ currentPage NOTIFY dataChanged)

public:
    explicit TableDataModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    int totalRows() const { return totalRows_; }
    int currentPage() const { return currentPage_; }

public slots:
    void loadQueryResult(const QueryResult& result);
    void clear();

signals:
    void dataChanged();

private:
    std::vector<std::string> columns_;
    std::vector<std::vector<std::string>> rows_;
    int totalRows_{0};
    int currentPage_{0};
};

}  // namespace remote_sqlite_qt
