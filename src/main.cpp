/***************************************************************************
 *   KShisen - A japanese game similar to mahjongg                         *
 *   Copyright 1997  Mario Weilguni <mweilguni@sime.com>                   *
 *   Copyright 2002-2004  Dave Corrie <kde@davecorrie.com>                 *
 *   Copyright 2009  Frederik Schwarzer <schwarzerf@gmail.com>             *
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

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kglobal.h>

// Version numbers are of the form: MAJOR.MINOR.MICRO
// The MAJOR number should only be incremented when really major changes occur.
// The MINOR number is increased for every branch of KDE from trunk (i.e. KDE 4.1, KDE 4.2 etc).
// The MICRO version is increased for every bugfix release if there were changes.
// MICRO numbers >= 50 are used for development snapshots in trunk.
#define KSHISEN_VERSION "1.7.51"

static const char description[] = I18N_NOOP("A KDE game similar to Mahjongg");

int main(int argc, char **argv)
{
    KAboutData aboutData("kshisen", 0, ki18n("Shisen-Sho"),
                         KSHISEN_VERSION, ki18n(description), KAboutData::License_GPL,
                         ki18n("(c) 1997, Mario Weilguni"), KLocalizedString(), "http://games.kde.org/kshisen");
    aboutData.addAuthor(ki18n("Frederik Schwarzer"), ki18n("Current Maintainer"), "schwarzerf@gmail.com");
    aboutData.addAuthor(ki18n("Dave Corrie"), ki18n("Former Maintainer"), "kde@davecorrie.com");
    aboutData.addAuthor(ki18n("Mario Weilguni"), ki18n("Original Author"), "mweilguni@sime.com");
    aboutData.addCredit(ki18n("Mauricio Piacentini"), ki18n("KMahjonggLib integration for KDE4"), "mauricio@tabuleiro.com");
    aboutData.addCredit(ki18n("Jason Lane"), ki18n("Added 'tiles removed' counter<br/>Tile smooth-scaling and window resizing"), "jglane@btopenworld.com");
    aboutData.addCredit(ki18n("Thanks also to everyone who should be listed here but is not!"));
    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication a;
    KGlobal::locale()->insertCatalog("libkdegames");
    KGlobal::locale()->insertCatalog("libkmahjongg");

    App *app = new App();
    app->show();
    return a.exec();
}

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
