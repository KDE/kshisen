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
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *******************************************************************
 */

#include <kapp.h>
#include <kconfig.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "version.h"
#include "app.h"


static const char *description = I18N_NOOP("KDE Game");

int main(int argc, char **argv)
{
  KAboutData aboutData( "kshisen", I18N_NOOP("KShisen"), 
    KSHISEN_VERSION, description, KAboutData::GPL, 
    "(c) 1997, Mario Weilguni");
  aboutData.addAuthor("Mario Weilguni",0, "mweilguni@sime.com");
  KCmdLineArgs::init( argc, argv, &aboutData );

  KApplication a;

  App *app = new App();
  app->resize(800, 500);
  app->show();
  a.setMainWidget(app);
  a.config()->sync();
  return a.exec();
}
