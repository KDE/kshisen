project( kshisen )

if(NOT COMPILING_KDEGAMES)
	find_package(KDE4 REQUIRED)
	include(KDE4Defaults)
	include(MacroLibrary)
	find_package(KDEGames REQUIRED)

	set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)
	find_package(LibKMahjongg REQUIRED)

	add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
	add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
	include_directories(${LIBKMAHJONGG_INCLUDE_DIR} ${KDEGAMES_INCLUDE_DIRS} ${KDE4_INCLUDES} ${QDBUS_INCLUDE_DIRS})
else(NOT COMPILING_KDEGAMES)
	set(LIBKMAHJONGG_LIBRARY kmahjongglib)
endif(NOT COMPILING_KDEGAMES)

add_subdirectory( src )
add_subdirectory( sounds )
add_subdirectory( doc )

install( PROGRAMS kshisen.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR} )
