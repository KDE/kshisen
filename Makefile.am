

INCLUDES = -I$(top_srcdir)/libkdegames $(all_includes)

LDFLAGS	= -L$(top_builddir)/libkdegames $(all_libraries) $(KDE_RPATH)
LDADD = -lkdegames $(LIB_KDEUI)

PICDIR = $(kde_datadir)/kshisen/pics

bin_PROGRAMS = kshisen

METASOURCES = board.moc app.moc

noinst_HEADERS = app.h board.h debug.h version.h

kshisen_SOURCES = main.cpp board.cpp app.cpp

appsdir   = $(kde_appsdir)/Games
apps_DATA = kshisen.desktop
KDE_ICON  = kshisen

SUBDIRS = pics 

EXTRA_DIST = $(apps_DATA) 

rcdir = $(kde_datadir)/kshisen
rc_DATA = kshisenui.rc

messages: rc.cpp
	$(XGETTEXT) rc.cpp $(kshisen_SOURCES) -o $(podir)/kshisen.pot



