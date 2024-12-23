/*
    KShisen - A japanese game similar to Mahjongg
    SPDX-FileCopyrightText: 1997 Mario Weilguni <mweilguni@sime.com>
    SPDX-FileCopyrightText: 2002-2004 Dave Corrie <kde@davecorrie.com>
    SPDX-FileCopyrightText: 2009-2016 Frederik Schwarzer <schwarzer@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// STL
#include <memory>

// Qt
#include <QApplication>
#include <QCommandLineParser>

// KF
#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedString>
#define HAVE_KICONTHEME __has_include(<KIconTheme>)
#if HAVE_KICONTHEME
#include <KIconTheme>
#endif

#define HAVE_STYLE_MANAGER __has_include(<KStyleManager>)
#if HAVE_STYLE_MANAGER
#include <KStyleManager>
#endif
// KShisen
#include "app.h"
#include "debug.h"
#include "kshisen_version.h"

int main(int argc, char ** argv)
{
#if HAVE_KICONTHEME
    KIconTheme::initTheme();
#endif
    QApplication a(argc, argv);
#if HAVE_STYLE_MANAGER
    KStyleManager::initStyle();
#else // !HAVE_STYLE_MANAGER
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
    QApplication::setStyle(QStringLiteral("breeze"));
#endif // defined(Q_OS_MACOS) || defined(Q_OS_WIN)
#endif // HAVE_STYLE_MANAGER

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("kshisen"));

    KAboutData aboutData(QStringLiteral("kshisen"), i18n("Shisen-Sho"),
                         QStringLiteral(KSHISEN_VERSION_STRING),
                         i18n("A game similar to Mahjongg"),
                         KAboutLicense::GPL,
                         i18n("Copyright 1997 Mario Weilguni"),
                         QString(),
                         QStringLiteral("https://apps.kde.org/kshisen"));
    aboutData.addAuthor(i18n("Frederik Schwarzer"), i18n("Current Maintainer"), QStringLiteral("schwarzer@kde.org"));
    aboutData.addAuthor(i18n("Dave Corrie"), i18n("Former Maintainer"), QStringLiteral("kde@davecorrie.com"));
    aboutData.addAuthor(i18n("Mario Weilguni"), i18n("Original Author"), QStringLiteral("mweilguni@sime.com"));
    aboutData.addCredit(i18n("Mauricio Piacentini"), i18n("KMahjonggLib integration for KDE4"), QStringLiteral("mauricio@tabuleiro.com"));
    aboutData.addCredit(i18n("Jason Lane"), i18n("Added 'tiles removed' counter<br/>Tile smooth-scaling and window resizing"), QStringLiteral("jglane@btopenworld.com"));
    aboutData.addCredit(i18n("Thanks also to everyone who should be listed here but is not!"));
    KAboutData::setApplicationData(aboutData);
    KCrash::initialize();

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.process(a);
    aboutData.processCommandLine(&parser);

    a.setWindowIcon(QIcon::fromTheme(QStringLiteral("kshisen")));

    KDBusService service;

    auto app = new KShisen::App();
    app->show();
    return a.exec();
}

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
