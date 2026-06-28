#include "database_controller.h"

#include "eventbus/event_bus_manager.h"
#include "eventbus/event_names.h"
#include "spdlog/spdlog.h"

namespace remote_sqlite_qt {

DatabaseController::DatabaseController(
    std::unique_ptr<IRemoteSqliteDriver> remoteDriver,
    std::unique_ptr<ILocalSqliteDriver> localDriver,
    EventBusManager& busMgr)
    : remoteDriver_(std::move(remoteDriver)),
      localDriver_(std::move(localDriver)),
      busMgr_(busMgr) {}

void DatabaseController::setDatabase(const std::string& connectionId,
                                      const std::string& dbPath) {
    connectionId_ = connectionId;
    dbPath_ = dbPath;
    remoteDriver_->setConnection(connectionId, dbPath);
}

void DatabaseController::setLocalMode(bool local) {
    localMode_ = local;
}

// ===== 查询 =====

void DatabaseController::queryTableData(const std::string& tableName,
                                         int offset, int limit) {
    auto requestId = nextRequestId();
    auto sql = buildSelectSql(tableName, offset, limit);

    spdlog::debug("[{}] Query: {}", requestId, sql);
    busMgr_.publish(events::QueryRequest, connectionId_, dbPath_, sql, requestId);

    auto result = executeQuery(sql);
    if (!result.columns.empty() || !result.rows.empty()) {
        busMgr_.publish(events::QueryResult, requestId, result);
    } else {
        auto err = localMode_ ? localDriver_->lastError()
                              : remoteDriver_->lastError();
        busMgr_.publish(events::QueryError, requestId,
                        err.empty() ? "Query returned no results" : err);
    }
}

void DatabaseController::executeSql(const std::string& sql) {
    auto requestId = nextRequestId();
    spdlog::debug("[{}] Execute: {}", requestId, sql);
    busMgr_.publish(events::QueryRequest, connectionId_, dbPath_, sql, requestId);

    auto result = executeQuery(sql);
    auto err = localMode_ ? localDriver_->lastError()
                          : remoteDriver_->lastError();
    if (!err.empty()) {
        busMgr_.publish(events::QueryError, requestId, err);
    } else {
        busMgr_.publish(events::QueryResult, requestId, result);
    }
}

void DatabaseController::fullTextSearch(const std::string& tableName,
                                         const std::string& searchTerm,
                                         int offset, int limit) {
    auto requestId = nextRequestId();
    auto sql = buildSearchSql(tableName, searchTerm, offset, limit);

    spdlog::debug("[{}] Search: {} in {}", requestId, searchTerm, tableName);
    busMgr_.publish(events::QueryRequest, connectionId_, dbPath_, sql, requestId);

    auto result = executeQuery(sql);
    busMgr_.publish(events::QueryResult, requestId, result);
}

void DatabaseController::filterByColumn(const std::string& tableName,
                                         const std::string& column,
                                         const std::string& value,
                                         int offset, int limit) {
    auto requestId = nextRequestId();
    auto sql = buildFilterSql(tableName, column, value, offset, limit);

    spdlog::debug("[{}] Filter: {} = {} in {}", requestId, column, value, tableName);
    auto result = executeQuery(sql);
    busMgr_.publish(events::QueryResult, requestId, result);
}

// ===== 数据变更 =====

void DatabaseController::insertRow(const std::string& tableName,
                                    const std::vector<std::string>& columns,
                                    const std::vector<std::string>& values) {
    std::string colList, valList;
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) { colList += ", "; valList += ", "; }
        colList += "\"" + columns[i] + "\"";
        valList += "'" + (i < values.size() ? values[i] : "") + "'";
    }
    auto sql = "INSERT INTO \"" + tableName + "\" (" + colList + ") VALUES (" + valList + ");";

    spdlog::info("Insert into {}: {} values", tableName, columns.size());
    executeNonQuery(sql);
    queryTableData(tableName, 0, 100);  // 刷新显示
}

void DatabaseController::updateRow(const std::string& tableName,
                                    const std::string& whereClause,
                                    const std::vector<std::string>& columns,
                                    const std::vector<std::string>& values) {
    std::string setClause;
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) setClause += ", ";
        setClause += "\"" + columns[i] + "\" = '"
                     + (i < values.size() ? values[i] : "") + "'";
    }
    auto sql = "UPDATE \"" + tableName + "\" SET " + setClause
             + " WHERE " + whereClause + ";";

    spdlog::info("Update {}: {}", tableName, whereClause);
    executeNonQuery(sql);
}

void DatabaseController::deleteRows(const std::string& tableName,
                                     const std::string& whereClause) {
    auto sql = "DELETE FROM \"" + tableName + "\" WHERE " + whereClause + ";";
    spdlog::warn("Delete from {}: {}", tableName, whereClause);
    executeNonQuery(sql);
    queryTableData(tableName, 0, 100);
}

// ===== 数据库浏览 =====

void DatabaseController::loadTables() {
    auto tables = localMode_ ? localDriver_->getTables()
                             : remoteDriver_->getTables();
    busMgr_.publish(events::DbTablesLoaded, connectionId_, dbPath_, tables);
}

void DatabaseController::loadTableInfo(const std::string& tableName) {
    auto columns = localMode_ ? localDriver_->getTableInfo(tableName)
                              : remoteDriver_->getTableInfo(tableName);

    QueryResult result;
    for (const auto& col : columns) {
        result.rows.push_back({
            std::to_string(col.cid), col.name, col.type,
            col.notNull ? "YES" : "NO",
            col.defaultValue,
            col.primaryKey ? "YES" : "NO"
        });
    }
    result.columns = {"cid", "name", "type", "notnull", "dflt_value", "pk"};
    result.totalRows = static_cast<int>(columns.size());
    busMgr_.publish(events::QueryResult,
                    std::string("table_info:") + tableName, result);
}

void DatabaseController::loadTableIndexes(const std::string& tableName) {
    auto indexes = localMode_ ? localDriver_->getIndexes(tableName)
                              : remoteDriver_->getIndexes(tableName);

    QueryResult result;
    for (const auto& idx : indexes) {
        std::string cols;
        for (size_t i = 0; i < idx.columns.size(); ++i) {
            if (i > 0) cols += ", ";
            cols += idx.columns[i];
        }
        result.rows.push_back({idx.name, idx.unique ? "YES" : "NO", cols, idx.sql});
    }
    result.columns = {"name", "unique", "columns", "sql"};
    result.totalRows = static_cast<int>(indexes.size());
    busMgr_.publish(events::QueryResult,
                    std::string("index_info:") + tableName, result);
}

void DatabaseController::loadTableDdl(const std::string& tableName) {
    auto sql = "SELECT sql FROM sqlite_master WHERE name='" + tableName + "' AND type='table';";
    auto result = executeQuery(sql);
    busMgr_.publish(events::QueryResult,
                    std::string("ddl:") + tableName, result);
}

// ===== DDL =====

void DatabaseController::renameTable(const std::string& oldName,
                                      const std::string& newName) {
    auto sql = "ALTER TABLE \"" + oldName + "\" RENAME TO \"" + newName + "\";";
    spdlog::info("Rename table: {} -> {}", oldName, newName);
    executeNonQuery(sql);
    loadTables();
}

void DatabaseController::deleteTable(const std::string& tableName) {
    auto sql = "DROP TABLE IF EXISTS \"" + tableName + "\";";
    spdlog::warn("Drop table: {}", tableName);
    executeNonQuery(sql);
    loadTables();
}

void DatabaseController::dropIndex(const std::string& indexName) {
    auto sql = "DROP INDEX IF EXISTS \"" + indexName + "\";";
    executeNonQuery(sql);
}

// ===== 私有方法 =====

QueryResult DatabaseController::executeQuery(const std::string& sql) {
    return localMode_ ? localDriver_->query(sql)
                      : remoteDriver_->query(sql);
}

bool DatabaseController::executeNonQuery(const std::string& sql) {
    if (localMode_) {
        return localDriver_->execute(sql);
    } else {
        return remoteDriver_->execute(sql);
    }
}

std::string DatabaseController::buildSelectSql(const std::string& tableName,
                                                int offset, int limit) const {
    return "SELECT * FROM \"" + tableName + "\" LIMIT "
           + std::to_string(limit) + " OFFSET " + std::to_string(offset) + ";";
}

std::string DatabaseController::buildSearchSql(const std::string& tableName,
                                                const std::string& term,
                                                int offset, int limit) const {
    // 先获取列信息以构建跨列搜索
    return "SELECT * FROM \"" + tableName + "\" WHERE "
           "CAST(rowid AS TEXT) LIKE '%" + term + "%' "
           "LIMIT " + std::to_string(limit) + " OFFSET "
           + std::to_string(offset) + ";";
}

std::string DatabaseController::buildFilterSql(const std::string& tableName,
                                                const std::string& column,
                                                const std::string& value,
                                                int offset, int limit) const {
    return "SELECT * FROM \"" + tableName + "\" WHERE \""
           + column + "\" = '" + value + "' LIMIT "
           + std::to_string(limit) + " OFFSET "
           + std::to_string(offset) + ";";
}

std::string DatabaseController::nextRequestId() {
    return "req-" + std::to_string(++requestCounter_);
}

}  // namespace remote_sqlite_qt
