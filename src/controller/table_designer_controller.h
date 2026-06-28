#pragma once

#include <memory>
#include <string>
#include <vector>

#include "driver_interface/types.h"

namespace remote_sqlite_qt {

class IRemoteSqliteDriver;
class ILocalSqliteDriver;
class EventBusManager;
class TableDesignerModel;

class TableDesignerController {
public:
    TableDesignerController(std::shared_ptr<IRemoteSqliteDriver> remote,
                            std::shared_ptr<ILocalSqliteDriver> local,
                            EventBusManager& busMgr);
    ~TableDesignerController() = default;

    TableDesignerController(const TableDesignerController&) = delete;
    TableDesignerController& operator=(const TableDesignerController&) = delete;

    void setModel(TableDesignerModel* model) { model_ = model; }

    // 生成 DDL
    std::string generateDDL() const;

    // 验证表定义
    bool validate(std::string& errorMsg) const;

    // 执行 CREATE TABLE
    bool createTable(const std::string& connectionId,
                     const std::string& dbPath,
                     bool localMode = false);

    // 执行建表（使用 model_ 的数据）
    bool executeCreate(const std::string& connectionId,
                       const std::string& dbPath);

    // 获取数据库中的所有表（供外键引用选择）
    std::vector<TableInfo> getAvailableTables(const std::string& connectionId,
                                               const std::string& dbPath);

private:
    std::shared_ptr<IRemoteSqliteDriver> remoteDriver_;
    std::shared_ptr<ILocalSqliteDriver> localDriver_;
    EventBusManager& busMgr_;
    TableDesignerModel* model_{nullptr};
};

}  // namespace remote_sqlite_qt
