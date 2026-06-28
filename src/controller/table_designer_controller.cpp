#include "table_designer_controller.h"
#include "model/table_designer_model.h"
#include "driver_interface/i_remote_sqlite_driver.h"
#include "driver_interface/i_local_sqlite_driver.h"
#include "eventbus/event_bus_manager.h"
#include "eventbus/event_names.h"
#include "model/table_designer_model.h"
#include "spdlog/spdlog.h"

namespace remote_sqlite_qt {

TableDesignerController::TableDesignerController(
    std::shared_ptr<IRemoteSqliteDriver> remote,
    std::shared_ptr<ILocalSqliteDriver> local,
    EventBusManager& busMgr)
    : remoteDriver_(std::move(remote)),
      localDriver_(std::move(local)),
      busMgr_(busMgr) {}

std::string TableDesignerController::generateDDL() const {
    if (!model_) return "";
    return model_->generateDDL().toStdString();
}

bool TableDesignerController::validate(std::string& errorMsg) const {
    if (!model_) {
        errorMsg = "No model set";
        return false;
    }

    auto cols = model_->columns();
    if (cols.empty()) {
        errorMsg = "Table must have at least one column";
        return false;
    }

    for (const auto& col : cols) {
        if (col.name.empty()) {
            errorMsg = "Column name cannot be empty";
            return false;
        }
        // 检查列名是否合法（字母或下划线开头）
        if (!std::isalpha(static_cast<unsigned char>(col.name[0])) && col.name[0] != '_') {
            errorMsg = "Column '" + col.name + "' must start with a letter or underscore";
            return false;
        }
    }

    // 检查表名
    auto name = model_->tableName().toStdString();
    if (name.empty()) {
        errorMsg = "Table name cannot be empty";
        return false;
    }

    return true;
}

bool TableDesignerController::createTable(const std::string& connectionId,
                                           const std::string& dbPath,
                                           bool localMode) {
    if (!model_) return false;

    std::string errorMsg;
    if (!validate(errorMsg)) {
        spdlog::error("Table validation failed: {}", errorMsg);
        busMgr_.publish(events::QueryError,
                        std::string("table_designer"), errorMsg);
        return false;
    }

    auto ddl = generateDDL();
    spdlog::info("Creating table via designer:\n{}", ddl);

    if (localMode) {
        return localDriver_->execute(ddl);
    } else {
        remoteDriver_->setConnection(connectionId, dbPath);
        return remoteDriver_->execute(ddl);
    }
}

bool TableDesignerController::executeCreate(const std::string& connectionId,
                                             const std::string& dbPath) {
    return createTable(connectionId, dbPath, false);
}

std::vector<TableInfo> TableDesignerController::getAvailableTables(
    const std::string& connectionId, const std::string& dbPath) {
    remoteDriver_->setConnection(connectionId, dbPath);
    return remoteDriver_->getTables();
}

}  // namespace remote_sqlite_qt
