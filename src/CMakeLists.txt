set(kshisen_SRCS
    app.cpp
    board.cpp
    main.cpp
    move.cpp
    possiblemove.cpp
)

ki18n_wrap_ui(kshisen_SRCS settings.ui)

kconfig_add_kcfg_files(kshisen_SRCS prefs.kcfgc)

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
install(FILES kshisenui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kshisen)
