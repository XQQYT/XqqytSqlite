#include "sqlite_command_builder.h"

#include "util/string_utils.h"

namespace remote_sqlite_qt {

std::string SqliteCommandBuilder::buildQueryCommand(const std::string& dbPath,
                                                     const std::string& sql) {
    return "sqlite3 -json " + util::shellEscape(dbPath) + " " + util::shellEscape(sql);
}

std::string SqliteCommandBuilder::buildTableListCommand(const std::string& dbPath) {
    return "sqlite3 -json " + util::shellEscape(dbPath) +
           " \"SELECT name, type, sql FROM sqlite_master "
           "WHERE type IN ('table','view','index','trigger') "
           "AND name NOT LIKE 'sqlite_%' ORDER BY type, name;\"";
}

std::string SqliteCommandBuilder::buildTableInfoCommand(const std::string& dbPath,
                                                         const std::string& tableName) {
    return "sqlite3 -json " + util::shellEscape(dbPath) +
           " \"PRAGMA table_info('" + tableName + "');\"";
}

std::string SqliteCommandBuilder::buildIndexListCommand(const std::string& dbPath,
                                                         const std::string& tableName) {
    return "sqlite3 -json " + util::shellEscape(dbPath) +
           " \"PRAGMA index_list('" + tableName + "');\"";
}

std::string SqliteCommandBuilder::buildRowCountCommand(const std::string& dbPath,
                                                        const std::string& tableName) {
    return "sqlite3 -json " + util::shellEscape(dbPath) +
           " \"SELECT COUNT(*) FROM \\\"" + tableName + "\\\";\"";
}

std::string SqliteCommandBuilder::buildVersionCommand() {
    return "sqlite3 --version";
}

std::string SqliteCommandBuilder::escapeSingleQuotes(const std::string& input) {
    std::string result;
    for (char c : input) {
        if (c == '\'') result += "'\\''";
        else result += c;
    }
    return result;
}

}  // namespace remote_sqlite_qt
