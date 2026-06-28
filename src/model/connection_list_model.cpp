#include "connection_list_model.h"

namespace remote_sqlite_qt {

ConnectionListModel::ConnectionListModel(QObject* parent)
    : QAbstractListModel(parent) {}

int ConnectionListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(connections_.size());
}

QVariant ConnectionListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(connections_.size()))
        return {};

    const auto& conn = connections_[static_cast<size_t>(index.row())];
    switch (role) {
    case HostRole:
        return QString::fromStdString(conn.host);
    case UsernameRole:
        return QString::fromStdString(conn.username);
    case PortRole:
        return 22;  // conn.port
    case StateRole:
        return static_cast<int>(conn.state);
    case Qt::DisplayRole:
        return QString::fromStdString(conn.host + " (" + conn.username + ")");
    }
    return {};
}

QHash<int, QByteArray> ConnectionListModel::roleNames() const {
    return {
        {HostRole, "host"},
        {UsernameRole, "username"},
        {PortRole, "port"},
        {StateRole, "connectionState"},
    };
}

void ConnectionListModel::updateConnections(
    const std::vector<ConnectionInfo>& connections) {
    beginResetModel();
    connections_ = connections;
    endResetModel();
}

}  // namespace remote_sqlite_qt
