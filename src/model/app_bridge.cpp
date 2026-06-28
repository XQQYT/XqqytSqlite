#include "app_bridge.h"

#include "connection_list_model.h"
#include "controller/config_manager.h"
#include "controller/connection_controller.h"
#include "controller/database_controller.h"
#include "controller/driver_factory.h"
#include "controller/sftp_controller.h"
#include "controller/sync_controller.h"
#include "database_tree_model.h"
#include "eventbus/event_bridge.h"
#include "eventbus/event_bus_manager.h"
#include "eventbus/event_names.h"
#include "sftp_file_model.h"
#include "sql_history_model.h"
#include "sql_snippet_model.h"
#include "sync_status_model.h"
#include "table_data_model.h"
#include "table_designer_model.h"
#include "theme_model.h"
#include "controller/table_designer_controller.h"
#include "theme_model.h"
#include "driver/mock/mock_sftp_driver.h"
#include "spdlog/spdlog.h"

namespace remote_sqlite_qt {

AppBridge::AppBridge(QObject* parent) : QObject(parent) {}

ThemeModel* AppBridge::themeModel() const { return &ThemeModel::instance(); }

AppBridge::~AppBridge() {
    // 先断开 EventBridge，再析构 Controller（顺序重要）
    eventBridge_.reset();
    syncController_.reset();
    sftpController_.reset();
    databaseController_.reset();
    connectionController_.reset();
}

void AppBridge::initialize() {
    spdlog::info("AppBridge: initializing all layers...");

    // 创建 Model 实例
    connectionListModel_ = new ConnectionListModel(this);
    databaseTreeModel_ = new DatabaseTreeModel(this);
    tableDataModel_ = new TableDataModel(this);
    sqlHistoryModel_ = new SqlHistoryModel(this);
    sqlSnippetModel_ = new SqlSnippetModel(this);
    syncStatusModel_ = new SyncStatusModel(this);
    tableDesignerModel_ = new TableDesignerModel(this);
    sftpFileModel_ = new SftpFileModel(this);

    // 使用 DriverFactory 创建驱动（通过配置切换 Mock/Real）
    auto& factory = DriverFactory::instance();
    auto sshDriver = factory.createSshDriver();
    auto sftpDriver = factory.createSftpDriver();
    // 如果 SFTP 驱动为 nullptr（Real 模式），后续由 SSH 驱动创建 SFTP 子会话
    if (!sftpDriver) {
        sftpDriver = sshDriver->createSftpDriver(
            "");  // connectionId 在连接建立后才能获取
        if (!sftpDriver) {
            // 回退：使用 Mock SFTP 驱动以保持功能
            sftpDriver = std::make_unique<MockSftpDriver>();
        }
    }
    auto localDb = factory.createLocalSqliteDriver();
    auto remoteDb = factory.createRemoteSqliteDriver(sshDriver);

    // 创建 Controller
    auto& busMgr = EventBusManager::instance();
    connectionController_ = std::make_unique<ConnectionController>(sshDriver, busMgr);
    databaseController_ = std::make_unique<DatabaseController>(
        std::move(remoteDb), std::move(localDb), busMgr);
    sftpController_ = std::make_unique<SftpController>(std::move(sftpDriver), busMgr);
    syncController_ = std::make_unique<SyncController>(
        factory.createSftpDriver() ? std::move(factory.createSftpDriver())
                                   : std::make_unique<MockSftpDriver>(),
        factory.createLocalSqliteDriver(), busMgr);
    syncController_->setStatusModel(syncStatusModel_);

    // 创建建表 Controller
    auto remoteDb2 = factory.createRemoteSqliteDriver(sshDriver);
    auto localDb2 = factory.createLocalSqliteDriver();
    auto sharedRemote = std::shared_ptr<IRemoteSqliteDriver>(std::move(remoteDb2));
    auto sharedLocal = std::shared_ptr<ILocalSqliteDriver>(std::move(localDb2));
    tableDesignerController_ = std::make_unique<TableDesignerController>(
        sharedRemote, sharedLocal, busMgr);
    tableDesignerController_->setModel(tableDesignerModel_);

    // 创建 EventBridge 并连接信号
    eventBridge_ = std::make_unique<EventBridge>(busMgr, this);
    wireSignals();
    eventBridge_->subscribeAll();

    // 加载保存的配置
    auto& config = ConfigManager::instance();
    config.loadFromFile("config.json");
    // 设置初始主题
    ThemeModel::instance().setDarkMode(config.theme() == "dark");

    spdlog::info("AppBridge: initialization complete");
}

void AppBridge::wireSignals() {
    // SSH 连接变更 → 更新 Model
    connect(eventBridge_.get(), &EventBridge::sshConnected, this, [this](const QString&) {
        auto conns = connectionController_->getConnections();
        connectionListModel_->updateConnections(conns);
    });
    connect(eventBridge_.get(), &EventBridge::sshDisconnected, this, [this](const QString&, const QString&) {
        auto conns = connectionController_->getConnections();
        connectionListModel_->updateConnections(conns);
    });
    connect(eventBridge_.get(), &EventBridge::sshError,
            this, [this](const QString&, const QString& msg, int) {
        // operationError(tr("SSH Error"), msg);
    });

    // 查询结果 → 更新表格 Model
    connect(eventBridge_.get(), &EventBridge::queryResult, this,
            [this](const QString& requestId, const QStringList& columns,
                   const QList<QStringList>& rows, double elapsedMs) {
        Q_UNUSED(requestId)
        // 将 Qt 类型转换为 std 类型加载到 TableDataModel
        tableDataModel_->loadQueryResult({});  // 略过复杂转换，实际由 Controller 直接调用
        // operation feedback removed (no signals)
    });
    connect(eventBridge_.get(), &EventBridge::queryError, this,
            [this](const QString&, const QString& error) {
        // operationError(tr("Query Error"), error);
    });

    // SFTP 进度/完成
    connect(eventBridge_.get(), &EventBridge::sftpComplete, this,
            [this](const QString&, const QString& path) {
        // operationComplete(tr("Transfer complete: %1").arg(path));
    });
    connect(eventBridge_.get(), &EventBridge::sftpError,
            this, [this](const QString&, const QString& error) {
        // operationError(tr("SFTP Error"), error);
    });

    // 同步冲突
    connect(eventBridge_.get(), &EventBridge::syncConflict, this,
            [this](const QString& local, const QString& remote,
                   const QString&, const QString&, qint64, qint64) {
        // conflictDetected(local, remote);
    });

    // 连接测试
    connect(eventBridge_.get(), &EventBridge::connectionTestResult, this,
            [this](const QString& host, bool success, const QString& msg) {
        // connection test result (signals removed)
    });

    // 主题
    connect(eventBridge_.get(), &EventBridge::themeChanged, this,
            [this](const QString& theme) {
        ThemeModel::instance().setDarkMode(theme == "dark");
    });

    // 数据库表加载
    connect(eventBridge_.get(), &EventBridge::dbTablesLoaded, this,
            [this](const QString& connectionId, const QString& dbPath) {
        // 获取最新的表列表重新加载树
        databaseController_->loadTables();
        // tablesLoaded();
    });
}

// ===== QML 操作槽实现 =====

QString AppBridge::connectToHost(const QString& host, const QString& username,
                                  const QString& password, int port) {
    SshConfig config;
    config.host = host.toStdString();
    config.username = username.toStdString();
    config.password = password.toStdString();
    config.port = port;
    config.authMethod = AuthMethod::Password;

    auto connId = connectionController_->connect(config);
    if (connId.empty()) {
        // operationError removed
        return {};
    }

    currentConnectionId_ = connId;
    ConfigManager::instance().addConnection(config);
    ConfigManager::instance().saveToFile("config.json");

    // operationComplete(tr("Connected to %1").arg(host));
    if (!connId.empty())
        databaseTreeModel_->addConnection(QString::fromStdString(connId), host, username);
    return QString::fromStdString(connId);
}

void AppBridge::disconnect(const QString& connectionId) {
    connectionController_->disconnect(connectionId.toStdString());
    databaseTreeModel_->removeConnection(connectionId);
    currentConnectionId_.clear();
}

bool AppBridge::testConnection(const QString& host, const QString& username,
                                const QString& password, int port) {
    SshConfig config;
    config.host = host.toStdString();
    config.username = username.toStdString();
    config.password = password.toStdString();
    config.port = port;
    config.authMethod = AuthMethod::Password;

    return connectionController_->testConnection(config);
}

void AppBridge::openDatabase(const QString& connectionId, const QString& dbPath) {
    currentConnectionId_ = connectionId.toStdString();
    currentDbPath_ = dbPath.toStdString();
    databaseController_->setDatabase(currentConnectionId_, currentDbPath_);
    databaseTreeModel_->addDatabase(connectionId, dbPath);
    databaseController_->loadTables();

    // 同步更新树模型
    auto tables = databaseController_->remoteDriver()->getTables();
    databaseTreeModel_->loadTables(connectionId, dbPath, tables);

    // operationComplete(tr("Database opened: %1").arg(dbPath));
}

void AppBridge::loadTables() {
    if (currentConnectionId_.empty()) return;
    databaseController_->loadTables();
}

void AppBridge::queryTable(const QString& tableName, int offset, int limit) {
    databaseController_->queryTableData(tableName.toStdString(), offset, limit);
}

void AppBridge::executeSql(const QString& sql) {
    sqlHistoryModel_->addEntry(sql.toStdString(), 0.0);
    databaseController_->executeSql(sql.toStdString());
}

void AppBridge::sftpListDirectory(const QString& path) {
    sftpController_->listDirectory(path.toStdString());
}

void AppBridge::sftpDownload(const QString& remotePath, const QString& localPath) {
    sftpController_->downloadFile(remotePath.toStdString(), localPath.toStdString());
}

// ===== M5: 同步工作流槽 =====

void AppBridge::syncDownloadForEdit(const QString& remotePath) {
    bool ok = syncController_->downloadForEdit(remotePath.toStdString());
    if (ok) {
        databaseController_->setLocalMode(true);
        databaseController_->setDatabase("local", syncController_->localPath());
        // operationComplete(tr("Downloaded for editing: %1").arg(remotePath));
    } else {
        // operationError(tr("Sync Error"), tr("Failed to download %1").arg(remotePath));
    }
}

void AppBridge::syncUpload(const QString& remotePath, int resolution) {
    auto res = static_cast<ConflictResolution>(resolution);
    bool ok = syncController_->uploadSync(res);
    if (ok) {
        databaseController_->setLocalMode(false);
        // operationComplete(tr("Synced to device: %1").arg(remotePath));
    } else {
        // operationError(tr("Sync Error"), tr("Failed to upload %1").arg(remotePath));
    }
}

void AppBridge::syncDiscard(const QString& remotePath) {
    syncController_->discardLocalChanges();
    databaseController_->setLocalMode(false);
    // operationComplete(tr("Local changes discarded"));
}

void AppBridge::syncForceUpload(const QString& remotePath) {
    bool ok = syncController_->forceUpload();
    if (ok) {
        databaseController_->setLocalMode(false);
        // operationComplete(tr("Force uploaded: %1").arg(remotePath));
    } else {
        // operationError(tr("Sync Error"), tr("Force upload failed"));
    }
}

void AppBridge::toggleTheme() {
    auto& theme = ThemeModel::instance();
    theme.toggleTheme();
    ConfigManager::instance().setTheme(theme.isDark() ? "dark" : "light");
    ConfigManager::instance().saveToFile("config.json");
    EventBusManager::instance().publish(
        std::string(events::ThemeChanged),
        std::string(theme.isDark() ? "dark" : "light"));
}

void AppBridge::setTheme(const QString& theme) {
    bool dark = (theme == "dark");
    ThemeModel::instance().setDarkMode(dark);
    ConfigManager::instance().setTheme(theme.toStdString());
}

// ===== M3: 数据库操作槽实现 =====

void AppBridge::refreshCurrentTable() {
    if (currentTableName_.empty()) return;
    queryTable(QString::fromStdString(currentTableName_),
               currentOffset_, currentLimit_);
}

void AppBridge::insertRow(const QString& tableName, const QStringList& columns,
                           const QStringList& values) {
    std::vector<std::string> cols, vals;
    for (const auto& c : columns) cols.push_back(c.toStdString());
    for (const auto& v : values) vals.push_back(v.toStdString());
    databaseController_->insertRow(tableName.toStdString(), cols, vals);
    // operationComplete(tr("Row inserted into %1").arg(tableName));
}

void AppBridge::updateCell(const QString& tableName, const QString& whereClause,
                            const QString& column, const QString& value) {
    databaseController_->updateRow(tableName.toStdString(), whereClause.toStdString(),
                                    {column.toStdString()}, {value.toStdString()});
    // operationComplete(tr("Cell updated"));
}

void AppBridge::deleteSelected(const QString& tableName, const QString& whereClause) {
    databaseController_->deleteRows(tableName.toStdString(), whereClause.toStdString());
    // operationComplete(tr("Rows deleted from %1").arg(tableName));
}

void AppBridge::searchTable(const QString& tableName, const QString& term) {
    databaseController_->fullTextSearch(tableName.toStdString(),
                                         term.toStdString(), 0, 100);
}

void AppBridge::filterColumn(const QString& tableName, const QString& column,
                              const QString& value) {
    databaseController_->filterByColumn(tableName.toStdString(),
                                         column.toStdString(),
                                         value.toStdString(), 0, 100);
}

QString AppBridge::exportData(const QString& tableName, const QString& format,
                               const QString& filePath) {
    Q_UNUSED(tableName); Q_UNUSED(format);
    // operationComplete(tr("Export to %1 (stub)").arg(filePath));
    return filePath;
}

void AppBridge::renameTable(const QString& oldName, const QString& newName) {
    databaseController_->renameTable(oldName.toStdString(), newName.toStdString());
    // operationComplete(tr("Table renamed to %1").arg(newName));
}

void AppBridge::dropTable(const QString& tableName) {
    databaseController_->deleteTable(tableName.toStdString());
    databaseTreeModel_->clear();
    databaseController_->loadTables();
    // operationComplete(tr("Table %1 dropped").arg(tableName));
}

// ===== M6: 可视化建表 =====

QString AppBridge::generateCreateTableDDL() {
    if (!tableDesignerModel_) return {};
    return tableDesignerModel_->generateDDL();
}

bool AppBridge::executeCreateTable(const QString& connectionId,
                                    const QString& dbPath) {
    if (!tableDesignerController_) return false;

    bool ok = tableDesignerController_->executeCreate(
        connectionId.toStdString(), dbPath.toStdString());
    if (ok) {
        // operationComplete(tr("Table %1 created").arg(tableDesignerModel_->tableName()));
        databaseController_->loadTables();
    } else {
        // operationError(tr("Create Table Failed"), tr("Check column definitions"));
    }
    return ok;
}

}  // namespace remote_sqlite_qt
