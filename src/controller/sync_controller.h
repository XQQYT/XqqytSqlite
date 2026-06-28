#pragma once

#include <memory>
#include <string>
#include <functional>

#include "driver_interface/i_sftp_driver.h"
#include "driver_interface/i_local_sqlite_driver.h"
#include "driver_interface/types.h"

namespace remote_sqlite_qt {

class EventBusManager;
class SyncStatusModel;

// 同步冲突解决方式
enum class ConflictResolution { Overwrite, KeepLocal, KeepRemote, Merge };

// 同步控制器——完整工作流：下载→本地编辑→冲突检测→上传
class SyncController {
public:
    SyncController(std::unique_ptr<ISftpDriver> sftp,
                   std::unique_ptr<ILocalSqliteDriver> localDb,
                   EventBusManager& busMgr);
    ~SyncController() = default;

    SyncController(const SyncController&) = delete;
    SyncController& operator=(const SyncController&) = delete;

    // 设置同步状态模型（由 AppBridge 设置，用于 UI 更新）
    void setStatusModel(SyncStatusModel* model) { statusModel_ = model; }

    // ===== 工作流步骤 =====

    // 步骤 1: 下载数据库到本地
    // 返回 true 表示下载成功，进入本地编辑模式
    bool downloadForEdit(const std::string& remotePath);

    // 步骤 2: 上传本地修改回设备（含冲突检测）
    // resolution: 冲突时的处理方式
    bool uploadSync(ConflictResolution resolution = ConflictResolution::Overwrite);

    // 步骤 3: 放弃本地修改，删除临时文件
    void discardLocalChanges();

    // 步骤 4: 强制覆盖上传（跳过冲突检测）
    bool forceUpload();

    // ===== 状态查询 =====
    bool isLocalMode() const { return localMode_; }
    std::string remotePath() const { return remotePath_; }
    std::string localPath() const { return localPath_; }

    // 获取本地 SQLite 驱动（供 QML 层直接操作）
    ILocalSqliteDriver* localDriver() { return localDb_.get(); }

    // 冲突检测
    bool hasConflict();
    SyncConflict getConflictInfo() const;

    // 清理所有临时文件
    void cleanupTempFiles();

private:
    // 检查远程是否有新修改
    bool checkRemoteChanged();

    // 验证下载文件的完整性
    bool verifyDownload(const std::string& localPath, int64_t expectedSize);

    std::unique_ptr<ISftpDriver> sftpDriver_;
    std::unique_ptr<ILocalSqliteDriver> localDb_;
    EventBusManager& busMgr_;
    SyncStatusModel* statusModel_{nullptr};

    std::string remotePath_;
    std::string localPath_;
    std::string remoteMtime_;
    int64_t remoteSize_{0};
    bool localMode_{false};
    bool dirty_{false};
};

}  // namespace remote_sqlite_qt
