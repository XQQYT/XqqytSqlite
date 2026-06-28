#include "data_export.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace remote_sqlite_qt::util {

std::string DataExport::exportToString(const QueryResult& result,
                                        const std::string& tableName,
                                        Format format) {
    switch (format) {
    case Format::Csv: return toCsv(result);
    case Format::Json: return toJson(result);
    case Format::SqlInsert: return toSqlInsert(result, tableName);
    }
    return "";
}

bool DataExport::exportToFile(const QueryResult& result,
                               const std::string& tableName,
                               Format format,
                               const std::string& filePath) {
    auto content = exportToString(result, tableName, format);
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) return false;
    file << content;
    return file.good();
}

DataExport::Format DataExport::formatFromExtension(const std::string& path) {
    auto dot = path.find_last_of('.');
    if (dot == std::string::npos) return Format::Csv;

    std::string ext = path.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == "json") return Format::Json;
    if (ext == "sql") return Format::SqlInsert;
    return Format::Csv;
}

std::string DataExport::formatToExtension(Format format) {
    switch (format) {
    case Format::Csv: return ".csv";
    case Format::Json: return ".json";
    case Format::SqlInsert: return ".sql";
    }
    return ".csv";
}

// ===== CSV =====

std::string DataExport::toCsv(const QueryResult& result) {
    std::ostringstream oss;

    // Header
    for (size_t i = 0; i < result.columns.size(); ++i) {
        if (i > 0) oss << ",";
        oss << csvEscape(result.columns[i]);
    }
    oss << "\n";

    // Rows
    for (const auto& row : result.rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            if (i > 0) oss << ",";
            oss << csvEscape(row[i]);
        }
        oss << "\n";
    }

    return oss.str();
}

std::string DataExport::csvEscape(const std::string& field) {
    // 如果包含逗号、引号或换行，需要用引号包裹
    bool needsQuotes = (field.find(',') != std::string::npos ||
                        field.find('"') != std::string::npos ||
                        field.find('\n') != std::string::npos);

    if (!needsQuotes) return field;

    std::string result = "\"";
    for (char c : field) {
        if (c == '"') result += "\"\"";  // CSV 引号转义
        else result += c;
    }
    result += "\"";
    return result;
}

// ===== JSON =====

std::string DataExport::toJson(const QueryResult& result) {
    std::ostringstream oss;
    oss << "[\n";

    for (size_t r = 0; r < result.rows.size(); ++r) {
        oss << "  {";
        for (size_t c = 0; c < result.columns.size(); ++c) {
            if (c > 0) oss << ", ";
            oss << "\"" << jsonEscape(result.columns[c]) << "\": ";
            oss << "\"" << jsonEscape(
                c < result.rows[r].size() ? result.rows[r][c] : "NULL")
                << "\"";
        }
        oss << "}";
        if (r < result.rows.size() - 1) oss << ",";
        oss << "\n";
    }

    oss << "]\n";
    return oss.str();
}

std::string DataExport::jsonEscape(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
        case '"': result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result += c;
        }
    }
    return result;
}

// ===== SQL INSERT =====

std::string DataExport::toSqlInsert(const QueryResult& result,
                                     const std::string& tableName) {
    std::ostringstream oss;

    for (const auto& row : result.rows) {
        oss << "INSERT INTO \"" << tableName << "\" (";
        for (size_t i = 0; i < result.columns.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << "\"" << result.columns[i] << "\"";
        }
        oss << ") VALUES (";
        for (size_t i = 0; i < row.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << sqlValueEscape(row[i]);
        }
        oss << ");\n";
    }

    return oss.str();
}

std::string DataExport::sqlValueEscape(const std::string& val) {
    if (val == "NULL") return "NULL";

    std::string result = "'";
    for (char c : val) {
        if (c == '\'') result += "''";
        else result += c;
    }
    result += "'";
    return result;
}

}  // namespace remote_sqlite_qt::util
