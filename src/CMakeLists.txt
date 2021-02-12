set(kshisen_SRCS
    app.cpp
    board.cpp
    main.cpp
    move.cpp
    possiblemove.cpp
    kshisen.qrc
)

ecm_setup_version(${KSHISEN_VERSION}
    VARIABLE_PREFIX KSHISEN
    VERSION_HEADER kshisen_version.h
)

ecm_qt_declare_logging_category(kshisen_SRCS
    HEADER debug.h
    IDENTIFIER KSHISEN_General
    CATEGORY_NAME kshisen
    DESCRIPTION "KShisen"
    EXPORT KSHISEN
)

ki18n_wrap_ui(kshisen_SRCS settings.ui)

kconfig_add_kcfg_files(kshisen_SRCS prefs.kcfgc)

file(GLOB ICON_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*-apps-kshisen.png")
ecm_add_app_icon(kshisen_SRCS ICONS ${ICON_SRCS})
add_executable(kshisen ${kshisen_SRCS})

target_link_libraries(kshisen
    Qt5::Gui
    KF5::Crash
    KF5::DBusAddons
    KF5::I18n
    KF5::XmlGui
    KF5KDEGames
    KF5KDEGamesPrivate
    KF5KMahjongglib
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
