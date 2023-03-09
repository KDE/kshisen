ecm_setup_version(${KSHISEN_VERSION}
    VARIABLE_PREFIX KSHISEN
    VERSION_HEADER kshisen_version.h
)

add_executable(kshisen)

target_sources(kshisen PRIVATE
    app.cpp
    app.h
    board.cpp
    board.h
    main.cpp
    move.cpp
    move.h
    possiblemove.cpp
    possiblemove.h
    types.h

    kshisen.qrc
)

ecm_qt_declare_logging_category(kshisen
    HEADER debug.h
    IDENTIFIER KSHISEN_General
    CATEGORY_NAME kshisen
    DESCRIPTION "KShisen"
    EXPORT KSHISEN
)

ki18n_wrap_ui(kshisen settings.ui)

kconfig_add_kcfg_files(kshisen prefs.kcfgc)

file(GLOB ICON_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*-apps-kshisen.png")
ecm_add_app_icon(kshisen ICONS ${ICON_SRCS})

target_link_libraries(kshisen
    KF5KDEGames
    KF${KF_MAJOR_VERSION}::CoreAddons
    KF${KF_MAJOR_VERSION}::Crash
    KF${KF_MAJOR_VERSION}::DBusAddons
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::XmlGui
    Qt${QT_MAJOR_VERSION}::Gui
)
if (QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(kshisen KMahjongglib6)
else()
    target_link_libraries(kshisen KF5KMahjongglib)
endif()

install(TARGETS kshisen ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES kshisen.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})

ecm_qt_install_logging_categories(
    EXPORT KSHISEN
    FILE kshisen.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
