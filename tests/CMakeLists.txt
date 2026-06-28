# 测试公共 include 路径
set(TEST_INCLUDES
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/eventbus
    ${CMAKE_SOURCE_DIR}/src/controller
    ${CMAKE_SOURCE_DIR}/src/driver
    ${CMAKE_SOURCE_DIR}/src/driver_interface
    ${CMAKE_SOURCE_DIR}/src/util
    ${CMAKE_SOURCE_DIR}/src/model
    ${CMAKE_SOURCE_DIR}/libs/EventBus/include
)

add_executable(test_eventbus
    unit/test_eventbus.cpp
)
target_include_directories(test_eventbus PRIVATE ${TEST_INCLUDES})
target_link_libraries(test_eventbus PRIVATE eventbus_lib)
add_test(NAME EventBus COMMAND test_eventbus)

add_executable(test_drivers
    unit/test_drivers.cpp
)
target_include_directories(test_drivers PRIVATE ${TEST_INCLUDES})
target_link_libraries(test_drivers PRIVATE driver_lib driver_interface_lib util_lib controller_lib)
add_test(NAME Drivers COMMAND test_drivers)

add_executable(test_controller
    unit/test_controller.cpp
)
target_include_directories(test_controller PRIVATE ${TEST_INCLUDES})
target_link_libraries(test_controller PRIVATE controller_lib driver_lib eventbus_lib util_lib)
add_test(NAME Controller COMMAND test_controller)
