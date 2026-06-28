#include "sql_history_model.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace remote_sqlite_qt {

SqlHistoryModel::SqlHistoryModel(QObject* parent)
    : QAbstractListModel(parent) {}

int SqlHistoryModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(entries_.size());
}

QVariant SqlHistoryModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(entries_.size()))
        return {};

    const auto& entry = entries_[static_cast<size_t>(index.row())];
    switch (role) {
    case SqlRole:
        return QString::fromStdString(entry.sql);
    case ExecutedAtRole:
        return QString::fromStdString(entry.executedAt);
    case ElapsedRole:
        return entry.elapsedMs;
    case Qt::DisplayRole:
        return QString::fromStdString(entry.sql);
    }
    return {};
}

QHash<int, QByteArray> SqlHistoryModel::roleNames() const {
    return {
        {SqlRole, "sql"},
        {ExecutedAtRole, "executedAt"},
        {ElapsedRole, "elapsedMs"},
    };
}

void SqlHistoryModel::addEntry(const std::string& sql, double elapsedMs) {
    // 获取当前时间字符串
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%H:%M:%S");

    beginInsertRows(QModelIndex(), 0, 0);
    entries_.push_front({sql, oss.str(), elapsedMs});

    // 保持上限
    if (entries_.size() > kMaxHistoryEntries) {
        entries_.pop_back();
    }
    endInsertRows();
}

void SqlHistoryModel::clear() {
    beginResetModel();
    entries_.clear();
    endResetModel();
}

}  // namespace remote_sqlite_qt
