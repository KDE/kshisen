/***************************************************************************
 *   KShisen - A japanese game similar to mahjongg                         *
 *   Copyright 1997  Mario Weilguni <mweilguni@sime.com>                   *
 *   Copyright 2002-2004  Dave Corrie <kde@davecorrie.com>                 *
 *   Copyright 2009,2010  Frederik Schwarzer <schwarzer@kde.org>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "app.h"

#include <KAboutData>
#include <Kdelibs4ConfigMigrator>

#include <QCommandLineParser>
#include <QApplication>
#include <KLocalizedString>
#include <KDBusService>

static const char description[] = I18N_NOOP("A KDE game similar to Mahjongg");

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    // Migrate pre-existing (4.x) configuration
    Kdelibs4ConfigMigrator migrate(QStringLiteral("kshisen"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("kshisenrc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("kshisenui.rc"));
    migrate.migrate();

    KLocalizedString::setApplicationDomain("kshisen");
    KAboutData aboutData(QStringLiteral("kshisen"), i18n("Shisen-Sho"), QStringLiteral("1.9+ #17"),
                         i18n(description), KAboutLicense::GPL, i18n("(c) 1997, Mario Weilguni"));
    aboutData.setHomepage(QStringLiteral("http://games.kde.org/kshisen"));
    aboutData.addAuthor(i18n("Frederik Schwarzer"), i18n("Current Maintainer"), QStringLiteral("schwarzer@kde.org"));
    aboutData.addAuthor(i18n("Dave Corrie"), i18n("Former Maintainer"), QStringLiteral("kde@davecorrie.com"));
    aboutData.addAuthor(i18n("Mario Weilguni"), i18n("Original Author"), QStringLiteral("mweilguni@sime.com"));
    aboutData.addCredit(i18n("Mauricio Piacentini"), i18n("KMahjonggLib integration for KDE4"), QStringLiteral("mauricio@tabuleiro.com"));
    aboutData.addCredit(i18n("Jason Lane"), i18n("Added 'tiles removed' counter<br/>Tile smooth-scaling and window resizing"), QStringLiteral("jglane@btopenworld.com"));
    aboutData.addCredit(i18n("Thanks also to everyone who should be listed here but is not!"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(a);
    aboutData.processCommandLine(&parser);

    a.setWindowIcon(QIcon::fromTheme(QStringLiteral("kshisen")));

    KDBusService service;

    App *app = new App();
    app->show();
    return a.exec();
}

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
