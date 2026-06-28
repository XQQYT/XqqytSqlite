#pragma once

#include <QObject>
#include <QMetaObject>
#include <functional>
#include <memory>
#include <string>

namespace remote_sqlite_qt {

class EventBusManager;

// EventBus 回调线程 → Qt 主线程的桥梁
// EventBus publish 的回调在 worker 线程执行，但 QAbstractItemModel 只能在主线程更新。
// EventBridge 订阅 EventBus 事件，在回调中用 QMetaObject::invokeMethod 将数据转发到主线程。
class EventBridge : public QObject {
    Q_OBJECT

public:
    explicit EventBridge(EventBusManager& busMgr, QObject* parent = nullptr);
    ~EventBridge() override;

    EventBridge(const EventBridge&) = delete;
    EventBridge& operator=(const EventBridge&) = delete;

    // 开始订阅所有事件（在 init 之后调用）
    void subscribeAll();

    // 取消所有订阅
    void unsubscribeAll();

signals:
    // ===== SSH 连接信号 =====
    void sshConnected(const QString& connectionId);
    void sshDisconnected(const QString& connectionId, const QString& reason);
    void sshError(const QString& connectionId, const QString& message, int code);

    // ===== 查询信号 =====
    void queryResult(const QString& requestId,
                     const QStringList& columns,
                     const QList<QStringList>& rows,
                     double elapsedMs);
    void queryError(const QString& requestId, const QString& error);

    // ===== SFTP 信号 =====
    void sftpProgress(const QString& taskId, qint64 transferred, qint64 total);
    void sftpComplete(const QString& taskId, const QString& path);
    void sftpError(const QString& taskId, const QString& error);

    // ===== 同步信号 =====
    void syncConflict(const QString& localPath, const QString& remotePath,
                      const QString& localMtime, const QString& remoteMtime,
                      qint64 localSize, qint64 remoteSize);
    void syncComplete(const QString& remotePath);
    void syncError(const QString& message);

    // ===== 数据库浏览信号 =====
    void dbTablesLoaded(const QString& connectionId, const QString& dbPath);

    // ===== 连接管理信号 =====
    void connectionTestResult(const QString& host, bool success, const QString& message);
    void connectionListChanged();

    // ===== 主题信号 =====
    void themeChanged(const QString& themeName);
    void languageChanged(const QString& langCode);

private:
    EventBusManager& busMgr_;
    std::vector<std::pair<std::string, size_t>> subscriptions_;
};

}  // namespace remote_sqlite_qt
