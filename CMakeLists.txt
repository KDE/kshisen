project( kshisen )

add_subdirectory( src )
add_subdirectory( sounds )

install( PROGRAMS kshisen.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR} )
