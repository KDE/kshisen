project(kshisen)

include_directories( ${CMAKE_SOURCE_DIR}/libkmahjongg ${CMAKE_SOURCE_DIR}/libkdegames/highscore )

########### next target ###############

set(kshisen_SRCS 
   main.cpp 
   board.cpp 
   app.cpp )

kde4_add_ui_files(kshisen_SRCS settings.ui )

kde4_add_kcfg_files(kshisen_SRCS prefs.kcfgc )

kde4_add_executable(kshisen ${kshisen_SRCS})

target_link_libraries(kshisen  ${KDE4_KDEUI_LIBS} kdegames kmahjongglib )

install(TARGETS kshisen  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kshisen.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kshisen.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES kshisenui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kshisen )

kde4_install_icons( ${ICON_INSTALL_DIR}   )




