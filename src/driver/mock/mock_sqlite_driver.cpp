#include "mock_sqlite_driver.h"

namespace remote_sqlite_qt {

bool MockLocalSqliteDriver::open(const std::string& dbPath) {
    if (shouldFail_) return false;
    dbPath_ = dbPath;
    open_ = true;
    return true;
}

void MockLocalSqliteDriver::close() {
    open_ = false;
}

bool MockLocalSqliteDriver::isOpen() const {
    return open_;
}

QueryResult MockLocalSqliteDriver::query(const std::string& /*sql*/) {
    if (shouldFail_) {
        errorMessage_ = "Mock query failed";
        return {};
    }
    return queryResult_;
}

bool MockLocalSqliteDriver::execute(const std::string& /*sql*/) {
    if (shouldFail_) {
        errorMessage_ = "Mock execute failed";
        return false;
    }
    return true;
}

std::vector<TableInfo> MockLocalSqliteDriver::getTables() {
    return tables_;
}

std::vector<ColumnInfo> MockLocalSqliteDriver::getTableInfo(
    const std::string& tableName) {
    auto it = columnInfo_.find(tableName);
    return (it != columnInfo_.end()) ? it->second : std::vector<ColumnInfo>{};
}

std::vector<IndexInfo> MockLocalSqliteDriver::getIndexes(
    const std::string& /*tableName*/) {
    return {};
}

int64_t MockLocalSqliteDriver::getRowCount(const std::string& /*tableName*/) {
    return static_cast<int64_t>(queryResult_.rows.size());
}

bool MockLocalSqliteDriver::beginTransaction() { return !shouldFail_; }
bool MockLocalSqliteDriver::commit() { return !shouldFail_; }
bool MockLocalSqliteDriver::rollback() { return !shouldFail_; }

int64_t MockLocalSqliteDriver::lastInsertRowId() { return lastRowId_; }

std::string MockLocalSqliteDriver::lastError() const {
    return errorMessage_;
}

void MockLocalSqliteDriver::setQueryResult(const QueryResult& result) {
    queryResult_ = result;
}

void MockLocalSqliteDriver::setTables(const std::vector<TableInfo>& tables) {
    tables_ = tables;
}

void MockLocalSqliteDriver::setColumnInfo(const std::string& tableName,
                                          const std::vector<ColumnInfo>& columns) {
    columnInfo_[tableName] = columns;
}

void MockLocalSqliteDriver::setShouldFail(bool fail) {
    shouldFail_ = fail;
}

void MockLocalSqliteDriver::setErrorMessage(const std::string& msg) {
    errorMessage_ = msg;
}

}  // namespace remote_sqlite_qt
