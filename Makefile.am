

INCLUDES = $(all_includes)

LDFLAGS	= $(all_libraries) $(KDE_RPATH)
LDADD = -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET)

KDELNKDIR= $(kde_appsdir)/Games
PICDIR = $(kde_datadir)/kshisen/pics
DOCDIR = $(kde_htmldir)/en/kshisen

bin_PROGRAMS = kshisen

SRCMETA = board.moc app.moc
DISTCLEANFILES = $(SRCMETA)

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
	$(INSTALL_DATA) mini-kshisen.xpm $(kde_minidir)/kshisen.xpm

uninstall-local:
	-rm -f $(kde_minidir)/kshisen.xpm

messages:
	@XGETTEXT@ -C -ktranslate -ki18n -x ../../kdelibs/po/kde.pot *.cpp && mv messages.po ../po/kshisen.pot

board.cpp:board.moc
board.moc: board.h
	$(MOC) board.h -o board.moc

app.cpp:app.moc
app.moc: app.h
	$(MOC) app.h -o app.moc



