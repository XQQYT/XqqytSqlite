#pragma once

#include <memory>
#include <string>
#include <vector>

#include "driver_interface/i_remote_sqlite_driver.h"
#include "driver_interface/i_local_sqlite_driver.h"
#include "driver_interface/types.h"

namespace remote_sqlite_qt {

class EventBusManager;

// 数据库控制器——统一编排远程/本地两种模式的 CRUD 操作
class DatabaseController {
public:
    DatabaseController(std::unique_ptr<IRemoteSqliteDriver> remoteDriver,
                       std::unique_ptr<ILocalSqliteDriver> localDriver,
                       EventBusManager& busMgr);
    ~DatabaseController() = default;

    DatabaseController(const DatabaseController&) = delete;
    DatabaseController& operator=(const DatabaseController&) = delete;

    // 设置当前数据库
    void setDatabase(const std::string& connectionId, const std::string& dbPath);
    void setLocalMode(bool local);

    // ===== 查询操作 =====
    // 异步返回结果通过 EventBus 发布 query:result / query:error
    void queryTableData(const std::string& tableName, int offset, int limit);
    void executeSql(const std::string& sql);
    void fullTextSearch(const std::string& tableName, const std::string& searchTerm,
                        int offset, int limit);
    void filterByColumn(const std::string& tableName, const std::string& column,
                        const std::string& value, int offset, int limit);

    // ===== 数据变更 =====
    void insertRow(const std::string& tableName,
                   const std::vector<std::string>& columns,
                   const std::vector<std::string>& values);
    void updateRow(const std::string& tableName,
                   const std::string& whereClause,
                   const std::vector<std::string>& columns,
                   const std::vector<std::string>& values);
    void deleteRows(const std::string& tableName, const std::string& whereClause);

    // ===== 数据库浏览 =====
    void loadTables();
    void loadTableInfo(const std::string& tableName);
    void loadTableIndexes(const std::string& tableName);
    void loadTableDdl(const std::string& tableName);

    // ===== DDL 操作 =====
    void renameTable(const std::string& oldName, const std::string& newName);
    void deleteTable(const std::string& tableName);
    void dropIndex(const std::string& indexName);

    // 获取驱动
    IRemoteSqliteDriver* remoteDriver() { return remoteDriver_.get(); }
    ILocalSqliteDriver* localDriver() { return localDriver_.get(); }

private:
    std::unique_ptr<IRemoteSqliteDriver> remoteDriver_;
    std::unique_ptr<ILocalSqliteDriver> localDriver_;
    EventBusManager& busMgr_;
    std::string connectionId_;
    std::string dbPath_;
    bool localMode_{false};
    int requestCounter_{0};

    std::string nextRequestId();

    // 根据当前模式选择驱动执行查询
    QueryResult executeQuery(const std::string& sql);
    bool executeNonQuery(const std::string& sql);

    // 构建 SQL 语句的工具方法
    std::string buildSelectSql(const std::string& tableName, int offset, int limit) const;
    std::string buildSearchSql(const std::string& tableName, const std::string& term,
                               int offset, int limit) const;
    std::string buildFilterSql(const std::string& tableName, const std::string& column,
                               const std::string& value, int offset, int limit) const;
};

}  // namespace remote_sqlite_qt
