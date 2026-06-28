#pragma once

#include <QAbstractListModel>
#include <QString>
#include <vector>

#include "driver_interface/types.h"

namespace remote_sqlite_qt {

// SFTP 远端文件列表 Model
class SftpFileModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        IsDirectoryRole,
        SizeRole,
        MtimeRole,
        PermissionsRole,
    };

    explicit SftpFileModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void loadEntries(const std::string& path,
                     const std::vector<FileEntry>& entries);
    void clear();

private:
    std::string currentPath_;
    std::vector<FileEntry> entries_;
};

}  // namespace remote_sqlite_qt
