

INCLUDES = -I$(top_srcdir)/libkdegames -I$(top_srcdir)/libkdegames/highscore $(all_includes)

PICDIR = $(kde_datadir)/kshisen/pics

bin_PROGRAMS = kshisen

METASOURCES = board.moc app.moc

noinst_HEADERS = app.h board.h tileset.h debug.h version.h

kshisen_SOURCES = main.cpp board.cpp app.cpp tileset.cpp settings.ui prefs.kcfgc
kshisen_LDADD = $(LIB_KDEGAMES) $(LIB_KDEUI)
kshisen_DEPENDENCIES = $(LIB_KDEGAMES_DEP)
kshisen_LDFLAGS = $(all_libraries) $(KDE_RPATH)

xdg_apps_DATA = kshisen.desktop
kde_kcfg_DATA = kshisen.kcfg
KDE_ICON  = kshisen

SUBDIRS = pics 

rcdir = $(kde_datadir)/kshisen
rc_DATA = kshisenui.rc

messages: rc.cpp
	$(XGETTEXT) rc.cpp $(kshisen_SOURCES) -o $(podir)/kshisen.pot
