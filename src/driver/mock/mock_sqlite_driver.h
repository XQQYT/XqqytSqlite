#pragma once

#include <string>
#include <vector>

#include "driver_interface/i_local_sqlite_driver.h"
#include "driver_interface/types.h"

namespace remote_sqlite_qt {

// Mock 本地 SQLite 驱动，供单元测试使用
class MockLocalSqliteDriver : public ILocalSqliteDriver {
public:
    MockLocalSqliteDriver() = default;
    ~MockLocalSqliteDriver() override = default;

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

    // 测试辅助方法
    void setQueryResult(const QueryResult& result);
    void setTables(const std::vector<TableInfo>& tables);
    void setColumnInfo(const std::string& tableName, const std::vector<ColumnInfo>& columns);
    void setShouldFail(bool fail);
    void setErrorMessage(const std::string& msg);

private:
    bool open_{false};
    bool shouldFail_{false};
    std::string errorMessage_;
    std::string dbPath_;
    QueryResult queryResult_;
    std::vector<TableInfo> tables_;
    std::unordered_map<std::string, std::vector<ColumnInfo>> columnInfo_;
    int64_t lastRowId_{0};
};

}  // namespace remote_sqlite_qt
