# util_lib — 最小化头文件库，绕过 MSVC 内部问题
add_library(util_lib INTERFACE)
target_include_directories(util_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src)
