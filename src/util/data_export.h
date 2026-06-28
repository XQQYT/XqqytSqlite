#pragma once

#include <string>
#include <vector>

#include "driver_interface/types.h"

namespace remote_sqlite_qt::util {

// 数据导出工具——支持 CSV / JSON / SQL INSERT 三种格式
class DataExport {
public:
    enum class Format { Csv, Json, SqlInsert };

    // 导出查询结果到字符串
    static std::string exportToString(const QueryResult& result,
                                       const std::string& tableName,
                                       Format format);

    // 导出查询结果到文件
    static bool exportToFile(const QueryResult& result,
                             const std::string& tableName,
                             Format format,
                             const std::string& filePath);

    // 根据扩展名判断格式
    static Format formatFromExtension(const std::string& path);
    static std::string formatToExtension(Format format);

private:
    static std::string toCsv(const QueryResult& result);
    static std::string toJson(const QueryResult& result);
    static std::string toSqlInsert(const QueryResult& result,
                                    const std::string& tableName);

    // CSV 字段转义
    static std::string csvEscape(const std::string& field);
    // JSON 字符串转义
    static std::string jsonEscape(const std::string& str);
    // SQL 值转义
    static std::string sqlValueEscape(const std::string& val);
};

}  // namespace remote_sqlite_qt::util
