project( kshisen )

file( GLOB sounds_ogg "*.ogg" )

install( FILES ${sounds_ogg} DESTINATION ${SOUND_INSTALL_DIR}/kshisen )
