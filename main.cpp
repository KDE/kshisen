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
 * created 1997 by Mario Weilguni <mweilguni@sime.com>
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

#include "version.h"
#include "app.h"

static const char description[] = I18N_NOOP("A KDE game similiar to Mahjongg");

// A hack to circumvent tricky i18n issue, not used later on in the code.
// Both context and contents must be exactly the same as for the entry in
// kdelibs/kdeui/ui_standards.rc
static const char dummy[] = I18N_NOOP2("Menu title", "&Move");

int main(int argc, char **argv)
{
	KAboutData aboutData( "kshisen", I18N_NOOP("Shisen-Sho"),
		KSHISEN_VERSION, description, KAboutData::License_GPL,
		"(c) 1997, Mario Weilguni");
	aboutData.addAuthor("Dave Corrie", I18N_NOOP("Current Maintainer"), "kde@davecorrie.com");
	aboutData.addAuthor("Mario Weilguni", I18N_NOOP("Original Author"), "mweilguni@sime.com");
	aboutData.addCredit("Jason Lane", I18N_NOOP("Added 'tiles removed' counter\nTile smooth-scaling and window resizing"), "jglane@btopenworld.com");
	aboutData.addCredit(0, I18N_NOOP("Thanks also to everyone who should be listed here but isn't!"), 0);
	KCmdLineArgs::init( argc, argv, &aboutData );

	KApplication a;
	KGlobal::locale()->insertCatalogue("libkdegames");

	App *app = new App();
	app->show();
	a.setMainWidget(app);
	a.config()->sync();
  return a.exec();
}

