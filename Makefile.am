

INCLUDES = $(all_includes)

LDFLAGS	= $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIB_KDEUI)

KDELNKDIR= $(kde_appsdir)/Games
PICDIR = $(kde_datadir)/kshisen/pics
DOCDIR = $(kde_htmldir)/en/kshisen

bin_PROGRAMS = kshisen

METASOURCES = board.moc app.moc

noinst_HEADERS = app.h board.h debug.h version.h

kshisen_SOURCES = main.cpp board.cpp app.cpp

appsdir = $(kde_appsdir)/Games
apps_DATA = kshisen.kdelnk

icondir = $(kde_icondir)
icon_DATA = kshisen.xpm

SUBDIRS = html pics 

EXTRA_DIST = $(apps_DATA) $(icon_DATA) mini-kshisen.xpm

install-data-local:
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-kshisen.xpm $(kde_minidir)/kshisen.xpm
	$(mkinstalldirs) $(kde_icondir)/large
	$(INSTALL_DATA) $(srcdir)/large-kshisen.xpm $(kde_icondir)/large/kshisen.xpm

uninstall-local:
	-rm -f $(kde_minidir)/kshisen.xpm
	-rm -f $(kde_icondir)/large/kshisen.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kshisen_SOURCES) && mv messages.po ../po/kshisen.pot



