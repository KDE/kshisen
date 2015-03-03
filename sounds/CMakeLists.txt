project( kshisen )

file( GLOB sounds_ogg "*.ogg" )

install( FILES ${sounds_ogg} DESTINATION ${KDE_INSTALL_SOUNDDIR}/kshisen )
