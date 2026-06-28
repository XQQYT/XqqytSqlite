# 编译器警告设置
# MSVC 和 GCC 分别使用各自的警告标志

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4)
    # /WX 仅在 CI 中启用，本地开发不阻塞
    if(DEFINED ENV{CI})
        add_compile_options(/WX)
    endif()
    # MSVC 特定的 C++17 一致性
    add_compile_options(/Zc:__cplusplus)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(DEFINED ENV{CI})
        add_compile_options(-Werror)
    endif()
    # GCC 警告补充
    add_compile_options(-Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align
                        -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion
                        -Wnull-dereference -Wformat=2 -Wimplicit-fallthrough)
endif()

# 可选的 clang-tidy 集成（需安装 clang-tidy）
find_program(CLANG_TIDY_EXE clang-tidy)
if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY
        "${CLANG_TIDY_EXE};-config-file=${CMAKE_SOURCE_DIR}/.clang-tidy;--header-filter=${CMAKE_SOURCE_DIR}/src/"
    )
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
else()
    message(WARNING "clang-tidy not found. Static analysis disabled.")
endif()
