#include "file_utils.h"
#include <string>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#endif

namespace remote_sqlite_qt::util {

std::string toPosixPath(const std::string& path) {
    std::string r = path;
    for (auto& c : r) if (c == '\\') c = '/';
    return r;
}

std::string toNativePath(const std::string& path) {
#ifdef _WIN32
    std::string r = path;
    for (auto& c : r) if (c == '/') c = '\\';
    return r;
#else
    return path;
#endif
}

std::string joinPath(const std::string& base, const std::string& child) {
    if (base.empty()) return child;
    if (child.empty()) return base;
    if (base.back() == '/' || base.back() == '\\') return base + child;
    return base + "/" + child;
}

std::string fileName(const std::string& path) {
    auto p = path.find_last_of("/\\");
    if (p == std::string::npos) return path;
    return path.substr(p + 1);
}

std::string parentPath(const std::string& path) {
    auto p = path.find_last_of("/\\");
    if (p == std::string::npos) return "";
    return path.substr(0, p);
}

std::string tempDir() {
#ifdef _WIN32
    char buf[MAX_PATH];
    GetTempPathA(MAX_PATH, buf);
    return std::string(buf);
#else
    return "/tmp";
#endif
}

std::string tempFilePath(const std::string& prefix) {
    return tempDir() + prefix + "_tmp.db";
}

bool fileExists(const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb");
    if (f) { fclose(f); return true; }
    return false;
}

int64_t fileSize(const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    auto sz = ftell(f);
    fclose(f);
    return static_cast<int64_t>(sz);
}

bool removeFile(const std::string& path) {
    return std::remove(path.c_str()) == 0;
}

}  // namespace remote_sqlite_qt::util
