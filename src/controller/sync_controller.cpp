#include "sync_controller.h"
#include "model/sync_status_model.h"

#include "eventbus/event_bus_manager.h"
#include "eventbus/event_names.h"
#include "util/file_utils.h"
#include "spdlog/spdlog.h"

namespace remote_sqlite_qt {

SyncController::SyncController(std::unique_ptr<ISftpDriver> sftp,
                               std::unique_ptr<ILocalSqliteDriver> localDb,
                               EventBusManager& busMgr)
    : sftpDriver_(std::move(sftp)),
      localDb_(std::move(localDb)),
      busMgr_(busMgr) {}

bool SyncController::downloadForEdit(const std::string& remotePath) {
    spdlog::info("Sync: downloading {}", remotePath);

    // 如果已有本地编辑中的文件，先清理
    if (localMode_) discardLocalChanges();

    remotePath_ = remotePath;
    localPath_ = util::tempFilePath("sync_" + util::fileName(remotePath));

    try {
        // 记录远程文件元数据（用于后续冲突检测）
        remoteMtime_ = sftpDriver_->getFileMtime(remotePath);
        remoteSize_ = sftpDriver_->getFileSize(remotePath);

        if (statusModel_) statusModel_->startSync(remotePath, localPath_);

        // 下载文件
        sftpDriver_->download(remotePath, localPath_,
            [this](int64_t transferred, int64_t total) {
                int progress = (total > 0) ? static_cast<int>(transferred * 100 / total) : 0;
                if (statusModel_) statusModel_->updateProgress(remotePath_, progress);

                busMgr_.publish(events::SftpProgress,
                                std::string("sync-download"), transferred, total);
            });

        spdlog::info("Sync: downloaded {} -> {} ({} bytes)", remotePath, localPath_, remoteSize_);

        // 校验下载完整性
        if (!verifyDownload(localPath_, remoteSize_)) {
            spdlog::error("Sync: download verification failed");
            if (statusModel_) statusModel_->removeEntry(remotePath);
            busMgr_.publish(events::SyncError, std::string("Download verification failed"));
            return false;
        }

        // 用本地 SQLite 驱动打开
        if (!localDb_->open(localPath_)) {
            spdlog::error("Sync: failed to open local database {}", localPath_);
            if (statusModel_) statusModel_->removeEntry(remotePath);
            busMgr_.publish(events::SyncError,
                            std::string("Cannot open local database: ") + localDb_->lastError());
            return false;
        }

        localMode_ = true;
        dirty_ = false;

        if (statusModel_) statusModel_->markLocalEditing(remotePath);
        spdlog::info("Sync: local mode ready at {}", localPath_);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Sync: download failed: {}", e.what());
        if (statusModel_) statusModel_->removeEntry(remotePath);
        busMgr_.publish(events::SyncError, std::string(e.what()));
        return false;
    }
}

bool SyncController::uploadSync(ConflictResolution resolution) {
    if (!localMode_) {
        spdlog::warn("Sync: not in local mode, cannot upload");
        return false;
    }

    spdlog::info("Sync: uploading {}", remotePath_);

    // 冲突检测
    if (checkRemoteChanged()) {
        spdlog::warn("Sync: conflict detected on {}", remotePath_);

        SyncConflict conflict;
        conflict.localPath = localPath_;
        conflict.remotePath = remotePath_;
        conflict.localMtime = "now";
        conflict.remoteMtime = remoteMtime_;
        conflict.localSize = util::fileSize(localPath_);
        conflict.remoteSize = sftpDriver_->getFileSize(remotePath_);

        if (statusModel_) statusModel_->markConflict(remotePath_);
        busMgr_.publish(events::SyncConflict, conflict);

        switch (resolution) {
        case ConflictResolution::Overwrite:
            spdlog::info("Sync: user chose to overwrite remote");
            break;
        case ConflictResolution::KeepLocal:
            spdlog::info("Sync: user chose to keep local, cancelling upload");
            return false;
        case ConflictResolution::KeepRemote:
            spdlog::info("Sync: user chose to keep remote, discarding local");
            discardLocalChanges();
            return false;
        case ConflictResolution::Merge:
            spdlog::warn("Sync: merge not supported, falling back to overwrite");
            break;
        }
    }

    try {
        localDb_->close();

        if (statusModel_) {
            statusModel_->startSync(remotePath_, localPath_);
            dirty_ = false;  // 先清除 dirty 标志（startSync 会重置状态）
        }

        sftpDriver_->upload(localPath_, remotePath_,
            [this](int64_t transferred, int64_t total) {
                int progress = (total > 0) ? static_cast<int>(transferred * 100 / total) : 0;
                if (statusModel_) statusModel_->updateProgress(remotePath_, progress);

                busMgr_.publish(events::SftpProgress,
                                std::string("sync-upload"), transferred, total);
            });

        spdlog::info("Sync: upload complete: {}", remotePath_);

        if (statusModel_) statusModel_->markInSync(remotePath_);
        busMgr_.publish(events::SyncComplete, remotePath_);

        // 上传成功后清理本地临时文件
        discardLocalChanges();
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Sync: upload failed: {}", e.what());
        // 上传失败时重新打开本地数据库
        localDb_->open(localPath_);
        if (statusModel_) statusModel_->markDirty(remotePath_);
        busMgr_.publish(events::SyncError, std::string(e.what()));
        return false;
    }
}

void SyncController::discardLocalChanges() {
    if (localMode_) {
        localDb_->close();
        util::removeFile(localPath_);
        if (statusModel_) statusModel_->removeEntry(remotePath_);
        localMode_ = false;
        dirty_ = false;
        spdlog::info("Sync: local changes discarded for {}", remotePath_);
    }
}

bool SyncController::forceUpload() {
    return uploadSync(ConflictResolution::Overwrite);
}

bool SyncController::checkRemoteChanged() {
    try {
        auto currentMtime = sftpDriver_->getFileMtime(remotePath_);
        if (currentMtime.empty()) return false;

        // 比较修改时间
        if (currentMtime != remoteMtime_) {
            spdlog::info("Sync: remote file changed: was {} now {}", remoteMtime_, currentMtime);
            // 更新记录的 mtime
            remoteMtime_ = currentMtime;
            return true;
        }
        return false;
    } catch (...) {
        spdlog::warn("Sync: cannot check remote mtime, assuming no conflict");
        return false;
    }
}

bool SyncController::verifyDownload(const std::string& localPath,
                                     int64_t expectedSize) {
    auto localSize = util::fileSize(localPath);
    if (localSize < 0) return false;
    if (expectedSize > 0 && localSize != expectedSize) {
        spdlog::error("Sync: size mismatch! Expected: {}, got: {}", expectedSize, localSize);
        return false;
    }
    return localSize > 0;
}

bool SyncController::hasConflict() {
    return checkRemoteChanged();
}

SyncConflict SyncController::getConflictInfo() const {
    SyncConflict conflict;
    conflict.localPath = localPath_;
    conflict.remotePath = remotePath_;
    conflict.localMtime = "now";
    conflict.remoteMtime = remoteMtime_;
    conflict.localSize = util::fileSize(localPath_);
    conflict.remoteSize = remoteSize_;
    return conflict;
}

void SyncController::cleanupTempFiles() {
    // 清理所有遗留的临时同步文件
    if (localMode_) discardLocalChanges();
}

}  // namespace remote_sqlite_qt
