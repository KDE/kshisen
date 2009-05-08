project( kshisen )

add_subdirectory( src )
# not used as yet
#add_subdirectory( sounds )

kde4_add_app_icon( kshisen_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../hi*-app-kshisen.png" )

install( FILES kshisen.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR} )
