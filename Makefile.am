

INCLUDES = $(all_includes)

LDFLAGS	= $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIB_KDEUI)

PICDIR = $(kde_datadir)/kshisen/pics

bin_PROGRAMS = kshisen

METASOURCES = board.moc app.moc

noinst_HEADERS = app.h board.h debug.h version.h

kshisen_SOURCES = main.cpp board.cpp app.cpp

appsdir = $(kde_appsdir)/Games
apps_DATA = kshisen.desktop

KDE_ICON = kshisen

SUBDIRS = pics 

EXTRA_DIST = $(apps_DATA) 

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kshisen_SOURCES) && mv messages.po ../po/kshisen.pot



