#pragma once

#include <QObject>
#include <QPointer>
#include <memory>
#include <string>

#include "driver_interface/types.h"

namespace remote_sqlite_qt {

class EventBusManager;
class EventBridge;
class ConnectionController;
class DatabaseController;
class SftpController;
class SyncController;
class ConnectionListModel;
class DatabaseTreeModel;
class TableDataModel;
class SqlHistoryModel;
class SftpFileModel;
class SqlSnippetModel;
class SyncStatusModel;
class TableDesignerModel;
class TableDesignerController;
class ThemeModel;

// AppBridge——应用程序中枢，连接所有层
// QML 通过此类间接访问下层功能，不直接接触 EventBus/Controller
class AppBridge : public QObject {
public:
    explicit AppBridge(QObject* parent = nullptr);
    ~AppBridge() override;

    AppBridge(const AppBridge&) = delete;
    AppBridge& operator=(const AppBridge&) = delete;

    void initialize();

    // Model 访问器
    ConnectionListModel* connectionListModel() const { return connectionListModel_; }
    DatabaseTreeModel* databaseTreeModel() const { return databaseTreeModel_; }
    TableDataModel* tableDataModel() const { return tableDataModel_; }
    SqlHistoryModel* sqlHistoryModel() const { return sqlHistoryModel_; }
    SftpFileModel* sftpFileModel() const { return sftpFileModel_; }
    SqlSnippetModel* sqlSnippetModel() const { return sqlSnippetModel_; }
    SyncStatusModel* syncStatusModel() const { return syncStatusModel_; }
    TableDesignerModel* tableDesignerModel() const { return tableDesignerModel_; }
    ThemeModel* themeModel() const;

    // QML 可调用方法
    QString connectToHost(const QString& host, const QString& username,
                          const QString& password, int port);
    void disconnect(const QString& connectionId);
    bool testConnection(const QString& host, const QString& username,
                        const QString& password, int port);
    void openDatabase(const QString& connectionId, const QString& dbPath);
    void loadTables();
    void queryTable(const QString& tableName, int offset, int limit);
    void refreshCurrentTable();
    void insertRow(const QString& tableName, const QStringList& columns, const QStringList& values);
    void updateCell(const QString& tableName, const QString& whereClause, const QString& column, const QString& value);
    void deleteSelected(const QString& tableName, const QString& whereClause);
    void searchTable(const QString& tableName, const QString& term);
    void filterColumn(const QString& tableName, const QString& column, const QString& value);
    QString exportData(const QString& tableName, const QString& format, const QString& filePath);
    void renameTable(const QString& oldName, const QString& newName);
    void dropTable(const QString& tableName);
    QString generateCreateTableDDL();
    bool executeCreateTable(const QString& connectionId, const QString& dbPath);
    void executeSql(const QString& sql);
    void sftpListDirectory(const QString& path);
    void sftpDownload(const QString& remotePath, const QString& localPath);
    void syncDownloadForEdit(const QString& remotePath);
    void syncUpload(const QString& remotePath, int resolution);
    void syncDiscard(const QString& remotePath);
    void syncForceUpload(const QString& remotePath);
    void toggleTheme();
    void setTheme(const QString& theme);

private:
    // 连接 EventBridge 信号到各 Model 的槽
    void wireSignals();

    // 各层组件
    std::unique_ptr<EventBridge> eventBridge_;
    std::unique_ptr<ConnectionController> connectionController_;
    std::unique_ptr<DatabaseController> databaseController_;
    std::unique_ptr<SftpController> sftpController_;
    std::unique_ptr<SyncController> syncController_;
    std::unique_ptr<TableDesignerController> tableDesignerController_;

    // Model 实例（QPointer 确保 QML 侧安全）
    ConnectionListModel* connectionListModel_{nullptr};
    DatabaseTreeModel* databaseTreeModel_{nullptr};
    TableDataModel* tableDataModel_{nullptr};
    SqlHistoryModel* sqlHistoryModel_{nullptr};
    SqlSnippetModel* sqlSnippetModel_{nullptr};
    SyncStatusModel* syncStatusModel_{nullptr};
    TableDesignerModel* tableDesignerModel_{nullptr};
    SftpFileModel* sftpFileModel_{nullptr};

    // 当前状态
    std::string currentConnectionId_;
    std::string currentDbPath_;
    std::string currentTableName_;
    int currentOffset_{0};
    int currentLimit_{100};
};

}  // namespace remote_sqlite_qt
