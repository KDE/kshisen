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
#include "commit.h"
#include "version.h"

#include <KAboutData>
#include <Kdelibs4ConfigMigrator>

#include <QCommandLineParser>
#include <QApplication>
#include <KLocalizedString>
#include <KDBusService>
#include <QCommandLineParser>

static const char description[] = I18N_NOOP("A KDE game similar to Mahjongg");

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    KAboutData aboutData(QLatin1Literal("kshisen"), i18n("Shisen-Sho"),
                         QLatin1String(KSHISEN_VERSION " #" KSHISEN_COMMIT), i18n(description), KAboutLicense::GPL,
                         i18n("(c) 1997, Mario Weilguni")); 
    aboutData.setHomepage(QLatin1Literal("http://games.kde.org/kshisen"));
    aboutData.addAuthor(i18n("Frederik Schwarzer"), i18n("Current Maintainer"), QLatin1Literal("schwarzer@kde.org"));
    aboutData.addAuthor(i18n("Dave Corrie"), i18n("Former Maintainer"), QLatin1Literal("kde@davecorrie.com"));
    aboutData.addAuthor(i18n("Mario Weilguni"), i18n("Original Author"), QLatin1Literal("mweilguni@sime.com"));
    aboutData.addCredit(i18n("Mauricio Piacentini"), i18n("KMahjonggLib integration for KDE4"), QLatin1Literal("mauricio@tabuleiro.com"));
    aboutData.addCredit(i18n("Jason Lane"), i18n("Added 'tiles removed' counter<br/>Tile smooth-scaling and window resizing"), QLatin1Literal("jglane@btopenworld.com"));
    aboutData.addCredit(i18n("Thanks also to everyone who should be listed here but is not!"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(a);
    aboutData.processCommandLine(&parser);

    // Migrate pre-existing (4.x) configuration
    QStringList configFiles;
    configFiles.append(QLatin1String("kshisenrc"));
    configFiles.append(QLatin1String("kshisen.notifyrc"));

    Kdelibs4ConfigMigrator migrate(QLatin1String("kshisen"));
    migrate.setConfigFiles(configFiles);
    migrate.setUiFiles(QStringList() << QLatin1String("kshisenui.rc"));
    migrate.migrate();

    KDBusService service;

    App *app = new App();
    app->show();
    return a.exec();
}

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
