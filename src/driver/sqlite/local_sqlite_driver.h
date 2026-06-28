#pragma once

#include <memory>
#include <string>

#include "driver_interface/i_ssh_driver.h"
#include "driver_interface/i_local_sqlite_driver.h"
#include "driver_interface/i_sftp_driver.h"

struct sqlite3;

namespace remote_sqlite_qt {

// 本地 SQLite 驱动——直接封装 sqlite3 C API
class LocalSqliteDriver : public ILocalSqliteDriver {
public:
    LocalSqliteDriver();
    ~LocalSqliteDriver() override;

    bool open(const std::string& dbPath) override;
    void close() override;
    bool isOpen() const override;

    QueryResult query(const std::string& sql) override;
    bool execute(const std::string& sql) override;

    std::vector<TableInfo> getTables() override;
    std::vector<ColumnInfo> getTableInfo(const std::string& tableName) override;
    std::vector<IndexInfo> getIndexes(const std::string& tableName) override;
    int64_t getRowCount(const std::string& tableName) override;

    bool beginTransaction() override;
    bool commit() override;
    bool rollback() override;

    int64_t lastInsertRowId() override;
    std::string lastError() const override;

private:
    QueryResult execQuery(const std::string& sql, bool isDataQuery);
    sqlite3* db_{nullptr};
    std::string errorMessage_;
};

}  // namespace remote_sqlite_qt
