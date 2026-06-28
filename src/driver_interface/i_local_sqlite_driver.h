#pragma once

#include <string>
#include <vector>

#include "types.h"

namespace remote_sqlite_qt {

class ILocalSqliteDriver {
public:
    virtual ~ILocalSqliteDriver() = default;

    // 打开数据库文件
    virtual bool open(const std::string& dbPath) = 0;

    // 关闭数据库
    virtual void close() = 0;

    // 是否已打开
    virtual bool isOpen() const = 0;

    // 执行查询
    virtual QueryResult query(const std::string& sql) = 0;

    // 执行非查询语句（INSERT/UPDATE/DELETE/DDL）
    virtual bool execute(const std::string& sql) = 0;

    // 获取所有表/视图
    virtual std::vector<TableInfo> getTables() = 0;

    // 获取表列信息
    virtual std::vector<ColumnInfo> getTableInfo(const std::string& tableName) = 0;

    // 获取索引列表
    virtual std::vector<IndexInfo> getIndexes(const std::string& tableName) = 0;

    // 获取总行数
    virtual int64_t getRowCount(const std::string& tableName) = 0;

    // 事务控制
    virtual bool beginTransaction() = 0;
    virtual bool commit() = 0;
    virtual bool rollback() = 0;

    // 获取最后插入的 rowid
    virtual int64_t lastInsertRowId() = 0;

    // 获取最后错误信息
    virtual std::string lastError() const = 0;
};

}  // namespace remote_sqlite_qt
