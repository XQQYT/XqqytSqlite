#pragma once

#include <QAbstractListModel>
#include <QString>
#include <vector>

#include "driver_interface/types.h"

namespace remote_sqlite_qt {

// SSH 连接列表 Model
class ConnectionListModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        HostRole = Qt::UserRole + 1,
        UsernameRole,
        PortRole,
        StateRole,
    };

    explicit ConnectionListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void updateConnections(const std::vector<ConnectionInfo>& connections);

private:
    std::vector<ConnectionInfo> connections_;
};

}  // namespace remote_sqlite_qt
