add_library(driver_lib STATIC
    # Mock 驱动（开发期使用）
    mock/mock_ssh_driver.h
    mock/mock_ssh_driver.cpp
    mock/mock_sftp_driver.h
    mock/mock_sftp_driver.cpp
    mock/mock_sqlite_driver.h
    mock/mock_sqlite_driver.cpp

    # LibSSH2 SSH 驱动
    ssh/ssh_session_guard.h
    ssh/ssh_session_guard.cpp
    ssh/known_hosts.h
    ssh/known_hosts.cpp
    ssh/libssh2_ssh_driver.h
    ssh/libssh2_ssh_driver.cpp

    # LibSSH2 SFTP 驱动
    sftp/libssh2_sftp_driver.h
    sftp/libssh2_sftp_driver.cpp

    # 本地 SQLite 驱动（基于 sqlite3 amalgamation）
    sqlite/local_sqlite_driver.h
    sqlite/local_sqlite_driver.cpp

    # 远程 SQLite 驱动（通过 SSH exec）
    sqlite/remote_sqlite_driver.h
    sqlite/remote_sqlite_driver.cpp
    sqlite/sqlite_command_builder.h
    sqlite/sqlite_command_builder.cpp
)

target_include_directories(driver_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party/nlohmann
    ${CMAKE_SOURCE_DIR}/third_party/spdlog/include
)
target_link_libraries(driver_lib PUBLIC
    driver_interface_lib
    util_lib
    sqlite3_amalgamation
    nlohmann_json
    spdlog
    libssh2::libssh2
)

# Windows 需要链接 ws2_32
if(WIN32)
    target_link_libraries(driver_lib PUBLIC ws2_32)
endif()
