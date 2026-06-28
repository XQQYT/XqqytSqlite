#pragma once
#include <string>
#include <cstdio>
#include <algorithm>

namespace remote_sqlite_qt::util {

inline std::string toPosixPath(const std::string& path) {
    std::string r = path;
    for (auto& c : r) if (c == '\\') c = '/';
    return r;
}
inline std::string toNativePath(const std::string& path) {
#ifdef _WIN32
    std::string r = path;
    for (auto& c : r) if (c == '/') c = '\\';
    return r;
#else
    return path;
#endif
}
inline std::string joinPath(const std::string& base, const std::string& child) {
    if (base.empty()) return child;
    if (child.empty()) return base;
    if (base.back() == '/' || base.back() == '\\') return base + child;
    return base + "/" + child;
}
inline std::string fileName(const std::string& path) {
    auto p = path.find_last_of("/\\");
    return (p == std::string::npos) ? path : path.substr(p + 1);
}
inline std::string tempDir() { return "C:/Temp"; }
inline std::string tempFilePath(const std::string& prefix) { return tempDir() + "/" + prefix + "_tmp.db"; }
inline bool fileExists(const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb");
    if (f) { fclose(f); return true; }
    return false;
}
inline int64_t fileSize(const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    auto sz = ftell(f);
    fclose(f);
    return static_cast<int64_t>(sz);
}
inline bool removeFile(const std::string& path) { return std::remove(path.c_str()) == 0; }

}  // namespace remote_sqlite_qt::util
