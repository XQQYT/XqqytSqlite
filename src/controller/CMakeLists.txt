add_library(controller_lib STATIC
    connection_controller.h
    connection_controller.cpp
    database_controller.h
    database_controller.cpp
    sftp_controller.h
    sftp_controller.cpp
    sync_controller.h
    sync_controller.cpp
    table_designer_controller.h
    table_designer_controller.cpp
    config_manager.h
    config_manager.cpp
    driver_factory.h
    driver_factory.cpp
)

target_include_directories(controller_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(controller_lib PUBLIC
    driver_interface_lib
    eventbus_lib
    driver_lib
    util_lib
    spdlog
    nlohmann_json
)
