project( kshisen )

add_subdirectory( src )
# not used as yet
#add_subdirectory( sounds )

install( FILES kshisen.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
