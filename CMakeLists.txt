kde4_header()
project(kshisen)

add_subdirectory( pics ) 

include_directories( ${CMAKE_SOURCE_DIR}/libkdegames ${CMAKE_SOURCE_DIR}/libkdegames/highscore ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kshisen_SRCS 
   main.cpp 
   board.cpp 
   app.cpp 
   tileset.cpp )

kde4_automoc(${kshisen_SRCS})

kde4_add_ui3_files(kshisen_SRCS settings.ui )

kde4_add_kcfg_files(kshisen_SRCS prefs.kcfgc )

kde4_add_executable(kshisen ${kshisen_SRCS})

target_link_libraries(kshisen  ${KDE4_KDEUI_LIBS} kdegames )

install_targets(/bin kshisen )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kshisen.desktop )
install_files( ${KCFG_INSTALL_DIR} FILES kshisen.kcfg )
install_files( ${DATA_INSTALL_DIR}/kshisen FILES kshisenui.rc )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#
#
#INCLUDES = -I$(top_srcdir)/libkdegames -I$(top_srcdir)/libkdegames/highscore $(all_includes)
#
#PICDIR = $(kde_datadir)/kshisen/pics
#
#bin_PROGRAMS = kshisen
#
#METASOURCES = board.moc app.moc
#
#noinst_HEADERS = app.h board.h tileset.h debug.h version.h
#
#kshisen_SOURCES = main.cpp board.cpp app.cpp tileset.cpp settings.ui prefs.kcfgc
#kshisen_LDADD = $(LIB_KDEGAMES) $(LIB_KDEUI)
#kshisen_DEPENDENCIES = $(LIB_KDEGAMES_DEP)
#kshisen_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#xdg_apps_DATA = kshisen.desktop
#kde_kcfg_DATA = kshisen.kcfg
#KDE_ICON  = kshisen
#
#SUBDIRS = pics 
#
#rcdir = $(kde_datadir)/kshisen
#rc_DATA = kshisenui.rc
#
#messages: rc.cpp
#	$(XGETTEXT) rc.cpp $(kshisen_SOURCES) -o $(podir)/kshisen.pot
