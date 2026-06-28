#pragma once
#include <string>

namespace remote_sqlite_qt::util {

inline std::string sqlQuote(const std::string& value) {
    std::string result = "'";
    for (char c : value) {
        if (c == '\'') result += "''";
        else result += c;
    }
    result += "'";
    return result;
}

inline std::string shellEscape(const std::string& value) {
    std::string result = "\"";
    for (char c : value) {
        if (c == '"' || c == '\\' || c == '$' || c == '`') result += '\\';
        result += c;
    }
    result += "\"";
    return result;
}

}  // namespace remote_sqlite_qt::util
