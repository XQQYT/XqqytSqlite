#pragma once

#include <QAbstractListModel>
#include <QString>
#include <deque>
#include <string>

namespace remote_sqlite_qt {

constexpr int kMaxHistoryEntries = 100;

// SQL 执行历史 Model
class SqlHistoryModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        SqlRole = Qt::UserRole + 1,
        ExecutedAtRole,
        ElapsedRole,
    };

    explicit SqlHistoryModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void addEntry(const std::string& sql, double elapsedMs);
    void clear();

private:
    struct HistoryEntry {
        std::string sql;
        std::string executedAt;
        double elapsedMs{0.0};
    };
    std::deque<HistoryEntry> entries_;
};

}  // namespace remote_sqlite_qt
