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

#include "app.h"
#include <kapp.h>
#include <kmsgbox.h>
#include <qtimer.h>
#include <qaccel.h>
#include <kiconloader.h>

#define ID_FQUIT	101

#define ID_GUNDO	201
#define ID_GREDO	202
#define ID_GHOF		203
#define ID_GRESTART	204
#define ID_GNEW		205
#define ID_GHINT	206
#define ID_GISSOLVE	207

#define ID_OSIZE1	300
#define ID_OSIZE2	301
#define ID_OSIZE3	302
#define ID_OSIZE4	303
#define ID_OSIZE5	304
//#define ID_OSIZECUSTOM	305
#define ID_OSPEED1	306
#define ID_OSPEED2	307
#define ID_OSPEED3	308
#define ID_OSPEED4	309
#define ID_OSPEED5	310
#define ID_OLVL1	311
#define ID_OLVL2	312
#define ID_OLVL3	313
#define ID_OSOLVABLE	314

#define ID_HTUTORIAL	901
#define ID_HHELP	902
#define ID_HABOUT	903


static int size_x[5] = {14, 18, 22, 26, 30};
static int size_y[5] = { 6,  8, 10, 14, 16};
static int DELAY[5] = {125, 250, 500, 750, 1000};

App::App() : KFixedTopWidget() {
  locale = kapp->getLocale();
  setCaption(locale->translate("Shisen-Sho"));

  // create menu
  mb = new KMenuBar(this);
  QPopupMenu *fm = new QPopupMenu;
  fm->insertItem(locale->translate("&Quit"), ID_FQUIT);

  QPopupMenu *gm = new QPopupMenu;
  gm->insertItem(locale->translate("&Undo"), ID_GUNDO);
  gm->insertItem(locale->translate("&Redo"), ID_GREDO);
  gm->insertSeparator();
  gm->insertItem(locale->translate("Get &hint"), ID_GHINT);
  gm->insertSeparator();
  gm->insertItem(locale->translate("&New game"), ID_GNEW);
  gm->insertItem(locale->translate("Res&tart game"), ID_GRESTART);
  gm->insertSeparator();
  gm->insertItem(locale->translate("Is game solvable?"), ID_GISSOLVE);

  QPopupMenu *om = new QPopupMenu;
  om->setCheckable(TRUE);
  QPopupMenu *om_s = new QPopupMenu;
  om_s->setCheckable(TRUE);
  om_s->insertItem(locale->translate("14x6"), ID_OSIZE1);
  om_s->insertItem(locale->translate("18x8"), ID_OSIZE2);
  om_s->insertItem(locale->translate("24x12"), ID_OSIZE3);
  om_s->insertItem(locale->translate("28x16"), ID_OSIZE4);
  om_s->insertItem(locale->translate("32x20"), ID_OSIZE5);
  //om_s->insertItem(locale->translate("Custom size..."), ID_OSIZECUSTOM);
  QPopupMenu *om_sp = new QPopupMenu;
  om_sp->setCheckable(TRUE);
  om_sp->insertItem(locale->translate("Very fast"), ID_OSPEED1);
  om_sp->insertItem(locale->translate("Fast"), ID_OSPEED2);
  om_sp->insertItem(locale->translate("Medium"), ID_OSPEED3);
  om_sp->insertItem(locale->translate("Slow"), ID_OSPEED4);
  om_sp->insertItem(locale->translate("Very slow"), ID_OSPEED5);
  QPopupMenu *om_l = new QPopupMenu;
  om_l->insertItem(locale->translate("Easy"), ID_OLVL1);
  om_l->insertItem(locale->translate("Medium"), ID_OLVL2);
  om_l->insertItem(locale->translate("Hard"), ID_OLVL3);

  QPopupMenu *hm = new QPopupMenu;
  hm->insertItem(locale->translate("&Help"), ID_HHELP);
  //hm->insertItem(locale->translate("Start &Tutorial"), ID_HTUTORIAL);
  hm->insertSeparator();
  hm->insertItem(locale->translate("A&bout..."), ID_HABOUT);

  mb->insertItem(locale->translate("&File"), fm);
  mb->insertItem(locale->translate("&Game"), gm);
  om->insertItem(locale->translate("Si&ze"), om_s);
  om->insertItem(locale->translate("S&peed"), om_sp);
  om->insertItem(locale->translate("&Level"), om_l);
  om->insertItem(locale->translate("No unsolvable games"), ID_OSOLVABLE);
  mb->insertItem(locale->translate("&Options"), om);
  mb->insertSeparator();
  mb->insertItem(locale->translate("&Help"), hm);

  mb->setAccel(CTRL+Key_Q, ID_FQUIT);
  mb->setAccel(CTRL+Key_Z, ID_GUNDO);
  mb->setAccel(CTRL+Key_N, ID_GNEW);
  mb->setAccel(CTRL+Key_D, ID_GREDO);
  mb->setAccel(CTRL+Key_R, ID_GRESTART);
  mb->setAccel(CTRL+Key_H, ID_GHINT);
  mb->setAccel(Key_F1, ID_HHELP);

  mb->show();
  setMenu(mb);

  b = new Board(this);  
  setView(b);
  b->show();

  sb = new KStatusBar(this);
  sb->insertItem(locale->translate("Your time: XX:XX:XX"), 1);
  sb->show();
  setStatusBar(sb);

  tb = new KToolBar(this);
  connect(tb, SIGNAL(clicked(int)),
	  this, SLOT(menuCallback(int)));
  KIconLoader *il = kapp->getIconLoader();
  tb->insertButton(il->loadIcon("exit.xpm"), ID_FQUIT, TRUE, locale->translate("Quit"));
  tb->insertButton(il->loadIcon("back.xpm"), ID_GUNDO, TRUE, locale->translate("Undo"));
  tb->insertButton(il->loadIcon("forward.xpm"), ID_GREDO, TRUE, locale->translate("Redo"));
  tb->insertButton(il->loadIcon("help.xpm"), ID_HHELP, TRUE, locale->translate("Help"));
  tb->show();
  addToolBar(tb);
  updateRects();

  connect(mb, SIGNAL(activated(int)),
	  this, SLOT(menuCallback(int)));

  connect(b, SIGNAL(changed()),
	  this, SLOT(enableItems()));

  connect(b, SIGNAL(sizeChange()),
	  this, SLOT(sizeChanged()));

  // load default settings
  KConfig *conf = kapp->getConfig();
  int i;
  i = conf->readNumEntry("Speed", ID_OSPEED3);
  menuCallback(i); // what a hack

  i = conf->readNumEntry("Size", ID_OSIZE2);
  //if(i == ID_OSIZECUSTOM)
  //printf("CUSTOM SIZE, TODO\n");
  //  else
    menuCallback(i);

  QTimer *t = new QTimer(this);
  t->start(1000);
  connect(t, SIGNAL(timeout()),
	  this, SLOT(updateScore()));
  updateScore();

  connect(b, SIGNAL(endOfGame()),
 	  this, SLOT(slotEndOfGame()));

  bool _b;
  _b = conf->readNumEntry("Solvable", 1) > 0;
  b->setSolvableFlag(_b);
  mb->setItemChecked(ID_OSOLVABLE, b->getSolvableFlag());

  kapp->processEvents();
  i = conf->readNumEntry("Level", ID_OLVL2);
  menuCallback(i);

  sizeChanged();
  enableItems();
}

App::~App() {
  delete tb;
  delete mb;
  delete b;
  delete sb;
}

void App::menuCallback(int id) {
  int i;

  switch(id) {
  case ID_FQUIT:
    delete this;
    kapp->quit();
    return;
    break;

  case ID_GISSOLVE:
    if(b->solvable())
      KMsgBox::message(this, locale->translate("Information"),
		       locale->translate("This game is solveable"));
    else
      KMsgBox::message(this, locale->translate("Information"),
		       locale->translate("This game is NOT solveable"));
    break;

  case ID_GHINT:
    b->getHint();
    break;

  case ID_GNEW:
    b->newGame();
    break;

  case ID_GRESTART:
    b->setUpdatesEnabled(FALSE);
    while(b->canUndo())
      b->undo();
    b->setUpdatesEnabled(TRUE);
    b->update();
    break;

  case ID_GUNDO:
    if(b->canUndo())
      b->undo();
    break;

  case ID_GREDO:
    if(b->canRedo())
      b->redo();
    break;

  case ID_OSOLVABLE:
    b->setSolvableFlag(!b->getSolvableFlag());
    kapp->getConfig()->writeEntry("Solvable", (int)b->getSolvableFlag());
    mb->setItemChecked(id, b->getSolvableFlag());
    break;

  case ID_OLVL1:
  case ID_OLVL2:
  case ID_OLVL3:
    for(i = ID_OLVL1; i <= ID_OLVL3; i++)
      mb->setItemChecked(i, i == id);
    b->setShuffle((id - ID_OLVL1) * 4 + 1);
    b->newGame();
    kapp->getConfig()->writeEntry("Level", id);
    break;
   
  case ID_OSIZE1:
  case ID_OSIZE2:
  case ID_OSIZE3:
  case ID_OSIZE4:
  case ID_OSIZE5:
    {
      b->setSize(size_x[id-ID_OSIZE1], size_y[id-ID_OSIZE1]);
      b->newGame();
      for(i = ID_OSIZE1; i <= ID_OSIZE5; i++)
	mb->setItemChecked(i, FALSE);
      mb->setItemChecked(id, TRUE);
      kapp->getConfig()->writeEntry("Size", id);
    }
    break;

    //case ID_OSIZECUSTOM:
    //printf("CUSTOM SIZE\n");
    //break;

  case ID_OSPEED1:
  case ID_OSPEED2:
  case ID_OSPEED3:
  case ID_OSPEED4:
  case ID_OSPEED5:
    b->setDelay(DELAY[id - ID_OSPEED1]);
    for(i = ID_OSPEED1; i <= ID_OSPEED5; i++)
      mb->setItemChecked(i, i == id);
    break;

  case ID_HHELP:
    kapp->invokeHTMLHelp("", "");
    break;

  case ID_HTUTORIAL:
    printf("ENTER TUTORIAL\n");
    break;

  case ID_HABOUT:
    KMsgBox::message(this, locale->translate("About Shisen-Sho"),
    locale->translate("Shisen-Sho\n\n" \
		      "version 0.1\n" \
		      "(c) 1997 Mario Weilguni <mweilguni@sime.com>\n\n"\
		      "This program is free software published \nunder "\
		      "the GNU General Public License GPL.\n" \
		      "More about this in the help."),
		     KMsgBox::INFORMATION);
    break;

  default:
    printf("kshisen: unimplemented command %d\n", id);
  }

  enableItems();
}

void App::enableItems() {
  mb->setItemEnabled(ID_GUNDO, b->canUndo());
  mb->setItemEnabled(ID_GREDO, b->canRedo());
  mb->setItemEnabled(ID_GRESTART, b->canUndo());
  tb->setItemEnabled(ID_GUNDO, b->canUndo());
  tb->setItemEnabled(ID_GREDO, b->canRedo());
  tb->setItemEnabled(ID_GRESTART, b->canUndo());
}

void App::sizeChanged() {
  b->setFixedSize(b->sizeHint());
  updateRects();
}

void App::slotEndOfGame() {
  if(b->tilesLeft() > 0)
    KMsgBox::message(this, locale->translate("End of game"),
		     locale->translate("No more moves possible!"));
  else {
    QString s;
    s.sprintf(
      locale->translate("Congratulations! You made it in %02d:%02d:%02d"),
      b->getTimeForGame().hour(),
      b->getTimeForGame().minute(),
      b->getTimeForGame().second());
    
    KMsgBox::message(this, locale->translate("End of game"), s.data());
    b->newGame();
  }
}

void App::updateScore() {
  QString s;
  s.sprintf(
	    locale->translate("Your time: %02d:%02d:%02d"),
	    b->getTimeForGame().hour(),
	    b->getTimeForGame().minute(),
	    b->getTimeForGame().second());

  sb->changeItem(s.data(), 1);
}
    
#include "app.moc"
