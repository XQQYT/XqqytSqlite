#pragma once

#include <QAbstractListModel>
#include <QString>
#include <vector>
#include <string>

namespace remote_sqlite_qt {

// 同步状态枚举
enum class SyncState {
    RemoteOnly,          // 仅远程（未下载）
    Syncing,             // 正在同步（下载中）
    LocalEditing,        // 本地编辑中
    LocalDirty,          // 本地有未上传的修改
    Uploading,           // 正在上传
    InSync,              // 已同步（本地=远程）
    Conflict,            // 冲突——远程和本地都有修改
};

struct SyncEntry {
    std::string remotePath;
    std::string localPath;
    std::string dbName;
    SyncState state{SyncState::RemoteOnly};
    int64_t remoteSize{0};
    int64_t localSize{0};
    std::string remoteMtime;
    std::string localMtime;
    int transferProgress{0};  // 0-100
};

// 同步状态模型——追踪所有 SFTP 同步数据库的状态
class SyncStatusModel : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(int activeCount READ activeCount NOTIFY modelChanged)

public:
    enum Roles {
        RemotePathRole = Qt::UserRole + 1,
        LocalPathRole,
        DbNameRole,
        StateRole,
        RemoteSizeRole,
        LocalSizeRole,
        ProgressRole,
    };

    explicit SyncStatusModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int activeCount() const;

    // 状态管理
    void startSync(const std::string& remotePath, const std::string& localPath);
    void updateProgress(const std::string& remotePath, int progress);
    void markLocalEditing(const std::string& remotePath);
    void markDirty(const std::string& remotePath);
    void markConflict(const std::string& remotePath);
    void markInSync(const std::string& remotePath);
    void removeEntry(const std::string& remotePath);
    void clear();

    // 查找
    int findByRemotePath(const std::string& remotePath) const;
    SyncState stateFor(const std::string& remotePath) const;

signals:
    void modelChanged();

private:
    std::vector<SyncEntry> entries_;
    int indexOf(const std::string& remotePath) const;
};

}  // namespace remote_sqlite_qt
