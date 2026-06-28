#pragma once

#include <memory>
#include <string>

#include "driver_interface/i_ssh_driver.h"
#include "driver_interface/i_remote_sqlite_driver.h"
#include "driver/sqlite/sqlite_command_builder.h"

namespace remote_sqlite_qt {

// 远程 SQLite 驱动——通过 SSH exec 在远端执行 sqlite3 CLI
class RemoteSqliteDriver : public IRemoteSqliteDriver {
public:
    explicit RemoteSqliteDriver(std::shared_ptr<ISshDriver> sshDriver);
    ~RemoteSqliteDriver() override = default;

    void setConnection(const std::string& connectionId,
                       const std::string& dbPath) override;

    bool isSqlite3Available() override;
    std::string getSqliteVersion() override;

    QueryResult query(const std::string& sql) override;
    bool execute(const std::string& sql) override;

    std::vector<TableInfo> getTables() override;
    std::vector<ColumnInfo> getTableInfo(const std::string& tableName) override;
    std::vector<IndexInfo> getIndexes(const std::string& tableName) override;
    int64_t getRowCount(const std::string& tableName) override;

    std::string lastError() const override;

private:
    ExecResult execRemote(const std::string& command);
    QueryResult parseJsonResult(const std::string& jsonStr);

    std::shared_ptr<ISshDriver> sshDriver_;
    std::string connectionId_;
    std::string dbPath_;
    std::string errorMessage_;

    // Uses nlohmann/json (included in .cpp)
    void* jsonLib_{nullptr};  // 占位，实际使用时包含 nlohmann/json.hpp
};

}  // namespace remote_sqlite_qt
