#include "sync_status_model.h"

#include <algorithm>

namespace remote_sqlite_qt {

SyncStatusModel::SyncStatusModel(QObject* parent)
    : QAbstractListModel(parent) {}

int SyncStatusModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(entries_.size());
}

QVariant SyncStatusModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(entries_.size()))
        return {};
    const auto& e = entries_[static_cast<size_t>(index.row())];
    switch (role) {
    case RemotePathRole: return QString::fromStdString(e.remotePath);
    case LocalPathRole: return QString::fromStdString(e.localPath);
    case DbNameRole:   return QString::fromStdString(e.dbName);
    case StateRole:     return static_cast<int>(e.state);
    case RemoteSizeRole: return QVariant::fromValue(e.remoteSize);
    case LocalSizeRole:  return QVariant::fromValue(e.localSize);
    case ProgressRole:   return e.transferProgress;
    case Qt::DisplayRole: return QString::fromStdString(e.dbName);
    }
    return {};
}

QHash<int, QByteArray> SyncStatusModel::roleNames() const {
    return {
        {RemotePathRole, "remotePath"},
        {LocalPathRole, "localPath"},
        {DbNameRole, "dbName"},
        {StateRole, "syncState"},
        {RemoteSizeRole, "remoteSize"},
        {LocalSizeRole, "localSize"},
        {ProgressRole, "transferProgress"},
    };
}

int SyncStatusModel::activeCount() const {
    return static_cast<int>(std::count_if(entries_.begin(), entries_.end(),
        [](const SyncEntry& e) {
            return e.state == SyncState::Syncing ||
                   e.state == SyncState::Uploading ||
                   e.state == SyncState::LocalEditing ||
                   e.state == SyncState::LocalDirty;
        }));
}

void SyncStatusModel::startSync(const std::string& remotePath,
                                 const std::string& localPath) {
    auto idx = indexOf(remotePath);
    if (idx >= 0) {
        auto& e = entries_[static_cast<size_t>(idx)];
        e.state = SyncState::Syncing;
        e.transferProgress = 0;
        e.localPath = localPath;
        emit dataChanged(createIndex(idx, 0), createIndex(idx, 0));
    } else {
        beginInsertRows(QModelIndex(), 0, 0);
        SyncEntry e;
        e.remotePath = remotePath;
        e.localPath = localPath;
        e.dbName = remotePath.substr(remotePath.find_last_of('/') + 1);
        e.state = SyncState::Syncing;
        entries_.insert(entries_.begin(), std::move(e));
        endInsertRows();
    }
    emit modelChanged();
}

void SyncStatusModel::updateProgress(const std::string& remotePath, int progress) {
    auto idx = indexOf(remotePath);
    if (idx < 0) return;
    entries_[static_cast<size_t>(idx)].transferProgress = progress;
    emit dataChanged(createIndex(idx, 0), createIndex(idx, 0),
                     {ProgressRole});
}

void SyncStatusModel::markLocalEditing(const std::string& remotePath) {
    auto idx = indexOf(remotePath);
    if (idx < 0) return;
    entries_[static_cast<size_t>(idx)].state = SyncState::LocalEditing;
    emit dataChanged(createIndex(idx, 0), createIndex(idx, 0));
    emit modelChanged();
}

void SyncStatusModel::markDirty(const std::string& remotePath) {
    auto idx = indexOf(remotePath);
    if (idx < 0) return;
    entries_[static_cast<size_t>(idx)].state = SyncState::LocalDirty;
    emit dataChanged(createIndex(idx, 0), createIndex(idx, 0));
    emit modelChanged();
}

void SyncStatusModel::markConflict(const std::string& remotePath) {
    auto idx = indexOf(remotePath);
    if (idx < 0) return;
    entries_[static_cast<size_t>(idx)].state = SyncState::Conflict;
    emit dataChanged(createIndex(idx, 0), createIndex(idx, 0));
    emit modelChanged();
}

void SyncStatusModel::markInSync(const std::string& remotePath) {
    auto idx = indexOf(remotePath);
    if (idx < 0) return;
    entries_[static_cast<size_t>(idx)].state = SyncState::InSync;
    emit dataChanged(createIndex(idx, 0), createIndex(idx, 0));
    emit modelChanged();
}

void SyncStatusModel::removeEntry(const std::string& remotePath) {
    auto idx = indexOf(remotePath);
    if (idx < 0) return;
    beginRemoveRows(QModelIndex(), idx, idx);
    entries_.erase(entries_.begin() + idx);
    endRemoveRows();
    emit modelChanged();
}

void SyncStatusModel::clear() {
    beginResetModel();
    entries_.clear();
    endResetModel();
    emit modelChanged();
}

int SyncStatusModel::findByRemotePath(const std::string& remotePath) const {
    return indexOf(remotePath);
}

SyncState SyncStatusModel::stateFor(const std::string& remotePath) const {
    auto idx = indexOf(remotePath);
    return (idx >= 0) ? entries_[static_cast<size_t>(idx)].state : SyncState::RemoteOnly;
}

int SyncStatusModel::indexOf(const std::string& remotePath) const {
    for (size_t i = 0; i < entries_.size(); ++i) {
        if (entries_[i].remotePath == remotePath)
            return static_cast<int>(i);
    }
    return -1;
}

}  // namespace remote_sqlite_qt
