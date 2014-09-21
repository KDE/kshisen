project( kshisen )

set( kshisen_SRCS
     main.cpp
     board.cpp
     app.cpp
     )

ki18n_wrap_ui( kshisen_SRCS
    settings.ui
    )

kconfig_add_kcfg_files( kshisen_SRCS prefs.kcfgc )

add_executable( kshisen ${kshisen_SRCS} )

include_directories(${PHONON_INCLUDES})

target_link_libraries(kshisen Qt5::Gui KF5::CoreAddons KF5::KIOWidgets KF5::DNSSD Phonon::phonon4qt5 
                              KF5::XmlGui KF5KDEGames KF5KDEGamesPrivate KF5KMahjongglib )

install( TARGETS kshisen ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES kshisen.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES kshisenui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/kshisen )

