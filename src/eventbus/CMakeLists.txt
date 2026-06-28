add_library(eventbus_lib STATIC
    event_names.h
    event_bus_manager.h
    event_bus_manager.cpp
    event_bridge.h
    event_bridge.cpp
)

target_include_directories(eventbus_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libs/EventBus/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(eventbus_lib PUBLIC
    driver_interface_lib
    Qt6::Core
)
