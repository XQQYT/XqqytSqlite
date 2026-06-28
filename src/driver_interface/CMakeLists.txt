add_library(driver_interface_lib INTERFACE)

target_include_directories(driver_interface_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(driver_interface_lib INTERFACE
    types.h
    i_ssh_driver.h
    i_sftp_driver.h
    i_local_sqlite_driver.h
    i_remote_sqlite_driver.h
)
