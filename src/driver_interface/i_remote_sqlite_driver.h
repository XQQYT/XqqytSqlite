#pragma once

#include <string>
#include <vector>

#include "types.h"

namespace remote_sqlite_qt {

class IRemoteSqliteDriver {
public:
    virtual ~IRemoteSqliteDriver() = default;

    // 设置 SSH 连接参数
    virtual void setConnection(const std::string& connectionId,
                               const std::string& dbPath) = 0;

    // 检查远程 sqlite3 CLI 是否可用
    virtual bool isSqlite3Available() = 0;

    // 获取远程 sqlite3 版本字符串
    virtual std::string getSqliteVersion() = 0;

    // 执行查询
    virtual QueryResult query(const std::string& sql) = 0;

    // 执行非查询语句
    virtual bool execute(const std::string& sql) = 0;

    // 获取所有表/视图
    virtual std::vector<TableInfo> getTables() = 0;

    // 获取表列信息
    virtual std::vector<ColumnInfo> getTableInfo(const std::string& tableName) = 0;

    // 获取索引列表
    virtual std::vector<IndexInfo> getIndexes(const std::string& tableName) = 0;

    // 获取总行数
    virtual int64_t getRowCount(const std::string& tableName) = 0;

    // 获取最后错误信息
    virtual std::string lastError() const = 0;
};

}  // namespace remote_sqlite_qt
