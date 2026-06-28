#include "sftp_file_model.h"

#include <QDateTime>

namespace remote_sqlite_qt {

SftpFileModel::SftpFileModel(QObject* parent)
    : QAbstractListModel(parent) {}

int SftpFileModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(entries_.size());
}

QVariant SftpFileModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(entries_.size()))
        return {};

    const auto& entry = entries_[static_cast<size_t>(index.row())];
    switch (role) {
    case NameRole:
        return QString::fromStdString(entry.name);
    case IsDirectoryRole:
        return entry.isDirectory;
    case SizeRole:
        return QVariant::fromValue(entry.size);
    case MtimeRole:
        return QString::fromStdString(entry.mtime);
    case PermissionsRole:
        return QString::fromStdString(entry.permissions);
    case Qt::DisplayRole:
        return QString::fromStdString(entry.name);
    }
    return {};
}

QHash<int, QByteArray> SftpFileModel::roleNames() const {
    return {
        {NameRole, "fileName"},
        {IsDirectoryRole, "isDirectory"},
        {SizeRole, "fileSize"},
        {MtimeRole, "mtime"},
        {PermissionsRole, "permissions"},
    };
}

void SftpFileModel::loadEntries(const std::string& path,
                                 const std::vector<FileEntry>& entries) {
    beginResetModel();
    currentPath_ = path;
    entries_ = entries;
    endResetModel();
}

void SftpFileModel::clear() {
    beginResetModel();
    currentPath_.clear();
    entries_.clear();
    endResetModel();
}

}  // namespace remote_sqlite_qt
