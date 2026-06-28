#pragma once
#include <string>

namespace remote_sqlite_qt::util {

class SqlFormatter {
public:
    static std::string format(const std::string& sql);
};

}  // namespace remote_sqlite_qt::util
