cmake_minimum_required(VERSION 3.27)
project(XqqytSqlite VERSION 0.1.0 LANGUAGES CXX C)

# C++17 / C11
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "Require C++17")
set(CMAKE_CXX_EXTENSIONS OFF CACHE BOOL "No compiler extensions")
set(CMAKE_C_STANDARD 11 CACHE STRING "C standard")
set(CMAKE_C_STANDARD_REQUIRED ON CACHE BOOL "Require C11")

# MSVC: UTF-8 + C++17 + constexpr 兼容
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /std:c++17 /Zc:__cplusplus" CACHE STRING "" FORCE)
endif()

# Qt 需要 AUTOMOC（QML 模块元类型生成依赖）
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Qt 6.8 兼容性：避免 MSVC staticMetaObject 错误
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:externConstexpr-")

# 导出 compile_commands.json (供 clang-tidy 等工具使用)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# vcpkg toolchain
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain file")
endif()

# 编译器警告
include(cmake/CompilerWarnings.cmake)

# Qt 6
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Quick QuickControls2 Qml)

# 第三方头文件
add_library(spdlog INTERFACE)
target_include_directories(spdlog INTERFACE third_party/spdlog/include)

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE third_party/nlohmann)

add_library(sqlite3_amalgamation STATIC third_party/sqlite3/sqlite3.c)
target_include_directories(sqlite3_amalgamation PUBLIC third_party/sqlite3)
target_compile_definitions(sqlite3_amalgamation PRIVATE
    SQLITE_THREADSAFE=1
    SQLITE_ENABLE_JSON1
)

# libssh2 (vcpkg 提供)
find_package(libssh2 CONFIG REQUIRED)

# 分层子目录
add_subdirectory(src/util)
add_subdirectory(src/driver_interface)
add_subdirectory(src/eventbus)
add_subdirectory(src/driver)
add_subdirectory(src/controller)
add_subdirectory(src/model)
add_subdirectory(src/qml)

# 主可执行文件
qt_add_executable(XqqytSqlite
    src/main.cpp
    src/qml/qrc_qml.cpp
)
set_target_properties(XqqytSqlite PROPERTIES WIN32_EXECUTABLE TRUE)

target_link_libraries(XqqytSqlite PRIVATE
    model_lib
    controller_lib
    eventbus_lib
    driver_lib
    driver_interface_lib
    util_lib
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Qml
)

# 测试
enable_testing()
add_subdirectory(tests)

# 安装规则
include(cmake/InstallRules.cmake)
