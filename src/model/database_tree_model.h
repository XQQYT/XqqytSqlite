#pragma once

#include <QAbstractItemModel>
#include <QString>
#include <vector>
#include <memory>
#include <string>

#include "driver_interface/types.h"

namespace remote_sqlite_qt {

struct DbTreeNode {
    std::string name;
    std::string type;       // "root","device","database","folder","table","view","index","trigger"
    std::string connectionId;
    std::string dbPath;
    std::string sql;
    std::string host;       // 设备名
    std::string username;
    DbTreeNode* parent{nullptr};
    std::vector<std::unique_ptr<DbTreeNode>> children;
};

class DatabaseTreeModel : public QAbstractItemModel {
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        TypeRole,
        ConnectionIdRole,
        DbPathRole,
        SqlRole,
        IconRole,
        HostRole,
        UsernameRole,
    };

    explicit DatabaseTreeModel(QObject* parent = nullptr);

    QModelIndex index(int row, int col, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& idx) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    // 设备管理
    void addConnection(const QString& connId, const QString& host, const QString& user);
    void removeConnection(const QString& connId);

    // 数据库管理
    void addDatabase(const QString& connId, const QString& dbPath);
    void removeDatabase(const QString& connId, const QString& dbPath);

    // 加载表列表
    void loadTables(const QString& connId, const QString& dbPath,
                    const std::vector<TableInfo>& tables);

    void clear();

private:
    DbTreeNode* nodeFromIndex(const QModelIndex& idx) const;
    DbTreeNode* findDevice(const std::string& connId) const;
    DbTreeNode* findDatabase(DbTreeNode* device, const std::string& dbPath) const;
    static std::string iconForType(const std::string& type);

    std::unique_ptr<DbTreeNode> root_;
};

}  // namespace remote_sqlite_qt
