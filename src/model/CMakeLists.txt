add_library(model_lib STATIC
    theme_model.h theme_model.cpp
    connection_list_model.h connection_list_model.cpp
    table_data_model.h table_data_model.cpp
    sql_history_model.h sql_history_model.cpp
    sql_snippet_model.h sql_snippet_model.cpp
    sync_status_model.h sync_status_model.cpp
    table_designer_model.h table_designer_model.cpp
    database_tree_model.h database_tree_model.cpp
    sftp_file_model.h sftp_file_model.cpp
    app_bridge.h app_bridge.cpp
)

# 跳过 app_bridge.h 的 AUTOMOC（其 Q_OBJECT 与 MSVC 14.44 不兼容）
set_source_files_properties(app_bridge.h PROPERTIES SKIP_AUTOMOC ON)

target_include_directories(model_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(model_lib PUBLIC driver_interface_lib controller_lib eventbus_lib driver_lib util_lib Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick)
