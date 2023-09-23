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
    KMahjongglib6
    KDEGames6
    KF6::CoreAddons
    KF6::Crash
    KF6::DBusAddons
    KF6::I18n
    KF6::XmlGui
    Qt6::Gui
)

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
