/* Yo Emacs, this is -*- C++ -*-
 *******************************************************************
 *******************************************************************
 *
 *
 * KSHISEN
 *
 *
 *******************************************************************
 *
 * A japanese game similar to mahjongg
 *
 *******************************************************************
 *
 * Copyright (C) 1997 Mario Weilguni <mweilguni@sime.com>
 * Copyright (C) 2002-2004 Dave Corrie  <kde@davecorrie.com>
 *
 *******************************************************************
 *
 * This file is part of the KDE project "KSHISEN"
 *
 * KSHISEN is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * KSHISEN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with KSHISEN; see the file COPYING.  If not, write to
 * the Free Software Foundation, 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 *******************************************************************
 */

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>

#include "version.h"
#include "app.h"

static const char description[] = I18N_NOOP("A KDE game similar to Mahjongg");

// A hack to circumvent tricky i18n issue, not used later on in the code.
// Both context and contents must be exactly the same as for the entry in
// kdelibs/kdeui/ui_standards.rc
static const char dummy[] = I18N_NOOP2("Menu title", "&Move");

int main(int argc, char **argv)
{
	KAboutData aboutData( "kshisen", 0, ki18n("Shisen-Sho"),
			KSHISEN_VERSION, ki18n(description), KAboutData::License_GPL,
			ki18n("(c) 1997, Mario Weilguni"), KLocalizedString(), "http://games.kde.org/kshisen" );
	aboutData.addAuthor(ki18n("Dave Corrie"), ki18n("Current Maintainer"), "kde@davecorrie.com");
	aboutData.addAuthor(ki18n("Mario Weilguni"), ki18n("Original Author"), "mweilguni@sime.com");
	aboutData.addCredit(ki18n("Mauricio Piacentini"), ki18n("KMahjonggLib integration for KDE4"), "mauricio@tabuleiro.com");
	aboutData.addCredit(ki18n("Jason Lane"), ki18n("Added 'tiles removed' counter\nTile smooth-scaling and window resizing"), "jglane@btopenworld.com");
	aboutData.addCredit(ki18n("Thanks also to everyone who should be listed here but is not!"));
	KCmdLineArgs::init( argc, argv, &aboutData );

	KApplication a;
	KGlobal::locale()->insertCatalog("libkdegames");
	KGlobal::locale()->insertCatalog("libkmahjongg");

	App *app = new App();
	app->show();
	return a.exec();
}

