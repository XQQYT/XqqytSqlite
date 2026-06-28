# 安装与打包规则

include(GNUInstallDirs)

# ===== 安装规则 =====
# 主程序
install(TARGETS XqqytSqlite
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# windeployqt 输出：所有 Qt DLL、QML 模块、插件目录
# 安装整个 Release 目录内容（递归，排除 EXE 自身避免重复）
install(
    DIRECTORY "${CMAKE_BINARY_DIR}/Release/"
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    USE_SOURCE_PERMISSIONS
    PATTERN "XqqytSqlite.exe" EXCLUDE
    PATTERN "XqqytSqlite.pdb" EXCLUDE
    PATTERN "XqqytSqlite.ilk" EXCLUDE
    PATTERN "vc_red*" EXCLUDE
)

# ===== CPack 配置 =====
set(CPACK_PACKAGE_NAME "XqqytSqlite")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Remote SQLite management tool based on Qt 6.8")
set(CPACK_PACKAGE_VENDOR "XqqytSqlite")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

if(WIN32)
    # Windows 提供两种打包方式：便携版 ZIP 和 NSIS 安装包
    set(CPACK_GENERATOR "NSIS;ZIP")

    # ---- NSIS 安装包配置 ----
    set(CPACK_NSIS_DISPLAY_NAME "XqqytSqlite")
    set(CPACK_NSIS_PACKAGE_NAME "XqqytSqlite")
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    set(CPACK_NSIS_INSTALL_DIRECTORY "XqqytSqlite")

    # Icon: 暂时禁用（需要真实 .ico 文件）
    # set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/resources/icon.ico")
    # set(CPACK_NSIS_MUI_UNICON "${CMAKE_SOURCE_DIR}/resources/icon.ico")

    # 创建开始菜单快捷方式
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\XqqytSqlite.lnk' '$INSTDIR\\\\bin\\\\XqqytSqlite.exe'"
    )
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\XqqytSqlite.lnk'"
    )

    # 创建桌面快捷方式
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "${CPACK_NSIS_CREATE_ICONS_EXTRA}\nCreateShortCut '$DESKTOP\\\\XqqytSqlite.lnk' '$INSTDIR\\\\bin\\\\XqqytSqlite.exe'"
    )
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "${CPACK_NSIS_DELETE_ICONS_EXTRA}\nDelete '$DESKTOP\\\\XqqytSqlite.lnk'"
    )

    # 添加/删除程序信息
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\XqqytSqlite.exe")
    set(CPACK_NSIS_HELP_LINK "https://github.com/XQQYT/XqqytSqlite")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/XQQYT/XqqytSqlite")
    set(CPACK_NSIS_CONTACT "xqqyt0502@163.com")

    # 安装引导
    set(CPACK_NSIS_MUI_FINISHPAGE_RUN "XqqytSqlite.exe")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

    # 要求管理员权限（安装到 Program Files）
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY "${CMAKE_INSTALL_BINDIR}")

    # ---- ZIP 便携包配置 ----
    # ZIP 无需额外配置，CPack 自动生成
    # 便携包产物: XqqytSqlite-0.1.0-win64.zip
    # 解压即用，包含所有 Qt 运行时依赖

elseif(UNIX AND NOT APPLE)
    # Linux 提供 DEB 和 TGZ
    set(CPACK_GENERATOR "DEB;TGZ")

    set(CPACK_DEBIAN_PACKAGE_NAME "remote-sqlite-qt")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "XqqytSqlite Team")
    set(CPACK_DEBIAN_PACKAGE_SECTION "database")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS
        "libc6 (>= 2.34), libgcc-s1 (>= 3.0), libstdc++6 (>= 11), libssl3 (>= 3.0)")
endif()

include(CPack)
