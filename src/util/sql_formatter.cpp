#include "sql_formatter.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_set>

namespace remote_sqlite_qt::util {

static const std::unordered_set<std::string> kKeywords = {
    "SELECT","FROM","WHERE","INSERT","INTO","VALUES","UPDATE","SET","DELETE",
    "CREATE","TABLE","ALTER","DROP","INDEX","VIEW","TRIGGER","JOIN","LEFT",
    "RIGHT","INNER","OUTER","CROSS","ON","AS","AND","OR","IN","BETWEEN",
    "LIKE","IS","ORDER","BY","ASC","DESC","GROUP","HAVING","LIMIT","OFFSET",
    "UNION","ALL","DISTINCT","CASE","WHEN","THEN","ELSE","END","BEGIN",
    "COMMIT","ROLLBACK","PRAGMA","EXPLAIN","EXISTS"
};

std::string SqlFormatter::format(const std::string& sql) {
    // Simple formatter: uppercase keywords, add newlines before main clauses
    std::string result = sql;

    for (auto& c : result) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

    // This is a simplified formatter. Full implementation in future milestone.
    return result;
}

}  // namespace remote_sqlite_qt::util
