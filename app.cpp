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
#include "version.h"

#include <kapp.h>
#include <kmsgbox.h>
#include <qtimer.h>
#include <qaccel.h>
#include <kiconloader.h>
#include <kseparator.h>
#include <qlayout.h>
#include <qmsgbox.h>
#include <debug.h>
#include <math.h>

#define ID_FQUIT	101

#define ID_GUNDO	201
#define ID_GREDO	202
#define ID_GHOF		203
#define ID_GRESTART	204
#define ID_GNEW		205
#define ID_GHINT	206
#define ID_GISSOLVE	207

#ifdef DEBUGGING
#define ID_GFINISH      220
#endif

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
#define ID_HHELP	900	

static int size_x[5] = {14, 18, 24, 26, 30};
static int size_y[5] = { 6,  8, 12, 14, 16};
static int DELAY[5] = {125, 250, 500, 750, 1000};

extern int MAX(int, int);

App::App() : KTopLevelWidget() {
  locale = kapp->getLocale();
  setCaption(locale->translate("Shisen-Sho"));
  readHighscore();

  cheat = FALSE;

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
  gm->insertSeparator();
  gm->insertItem(locale->translate("&Hall of Fame"), ID_GHOF);
#ifdef DEBUGGING
  gm->insertSeparator();
  gm->insertItem("&Finish", ID_GFINISH);
#endif

  QPopupMenu *om = new QPopupMenu;
  om->setCheckable(TRUE);
  QPopupMenu *om_s = new QPopupMenu;
  om_s->setCheckable(TRUE);
  om_s->insertItem(locale->translate("14x6"), ID_OSIZE1);
  om_s->insertItem(locale->translate("18x8"), ID_OSIZE2);
  om_s->insertItem(locale->translate("24x12"), ID_OSIZE3);
  om_s->insertItem(locale->translate("26x14"), ID_OSIZE4);
  om_s->insertItem(locale->translate("30x16"), ID_OSIZE5);
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

  QPopupMenu *help = kapp->getHelpMenu(true, QString(i18n("Shisen-Sho"))
                                         + " " + KSHISEN_VERSION
                                         + i18n("\n\nby Mario Weilguni")
                                         + " (mweilguni@sime.com)");    


  mb->insertItem(locale->translate("&File"), fm);
  mb->insertItem(locale->translate("&Game"), gm);
  om->insertItem(locale->translate("Si&ze"), om_s);
  om->insertItem(locale->translate("S&peed"), om_sp);
  om->insertItem(locale->translate("&Level"), om_l);
  om->insertItem(locale->translate("Disallow unsolvable games"), ID_OSOLVABLE);
  mb->insertItem(locale->translate("&Options"), om);
  mb->insertSeparator();
  mb->insertItem(locale->translate("&Help"), help);

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
  sb->insertItem(locale->translate("Cheat mode"), 2);
  sb->show();
  setStatusBar(sb);
  sb->changeItem("", 2);

  tb = new KToolBar(this);
  connect(tb, SIGNAL(clicked(int)),
	  this, SLOT(menuCallback(int)));
  KIconLoader *il = kapp->getIconLoader();
  tb->insertButton(il->loadIcon("exit.xpm"), 
		   ID_FQUIT, TRUE, locale->translate("Quit"));
  tb->insertButton(il->loadIcon("back.xpm"), 
		   ID_GUNDO, TRUE, locale->translate("Undo"));
  tb->insertButton(il->loadIcon("forward.xpm"), 
		   ID_GREDO, TRUE, locale->translate("Redo"));
  tb->insertButton(il->loadIcon("help.xpm"), 
		   ID_HHELP, TRUE, locale->translate("Help"));
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
  mb->setItemChecked(ID_OSOLVABLE, 
		     b->getSolvableFlag());

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

  if ( id < 10 )		// Use default help menu
    return;

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
    cheat = TRUE;
    sb->changeItem(locale->translate("Cheat mode"), 2);
    break;

  case ID_GHOF:
    showHighscore();
    break;

  case ID_GNEW:
    b->newGame();
    cheat = FALSE;
  sb->changeItem("", 2);
    break;

  case ID_GRESTART:
    b->setUpdatesEnabled(FALSE);
    while(b->canUndo())
      b->undo();
    b->setUpdatesEnabled(TRUE);
    b->update();
    break;

  case ID_GUNDO:
    if(b->canUndo()) {
      b->undo();
      cheat = TRUE;
      sb->changeItem(locale->translate("Cheat mode"), 2);
    }
    break;

  case ID_GREDO:
    if(b->canRedo()) 
      b->redo();
    break;

#ifdef DEBUGGING
  case ID_GFINISH:
    b->finish();
    break;
#endif

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

    //  case ID_OSIZECUSTOM:
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

  case ID_HTUTORIAL:
    printf("ENTER TUTORIAL\n");
    break;

  case ID_HHELP:
    KApplication::getKApplication()->invokeHTMLHelp("", ""); 
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
    // create highscore entry
    HighScore hs;
    hs.seconds = b->getTimeForGame();
    hs.x = b->x_tiles();
    hs.y = b->y_tiles();

    // check if we made it into Top10
    bool isHighscore = FALSE;
    if(highscore.size() < HIGHSCORE_MAX)
      isHighscore = TRUE;
    else if(isBetter(hs, highscore[HIGHSCORE_MAX-1]))
      isHighscore = TRUE;

    if(isHighscore) {
      QString name = getPlayerName();
      strncpy(hs.name, (const char *)name, sizeof(hs.name) - 1);
      hs.date = time((time_t*)0);
      hs.x = b->x_tiles();
      hs.y = b->y_tiles();
      int rank = insertHighscore(hs);
      showHighscore(rank);
    } else {
      QString s;
      s.sprintf(
		locale->translate("Congratulations! You made it in %02d:%02d:%02d"),
		b->getTimeForGame()/3600,
		(b->getTimeForGame() / 60)  % 60,
		b->getTimeForGame() % 60);
      
      KMsgBox::message(this, locale->translate("End of game"), s.data());
    }
  }

  cheat = FALSE;
  sb->changeItem("", 2);
  b->newGame();
}

void App::updateScore() {
  QString s;
  s.sprintf(
	    locale->translate("Your time: %02d:%02d:%02d"),
	    b->getTimeForGame()/3600,
	    (b->getTimeForGame() / 60)  % 60,
	    b->getTimeForGame() % 60);

  sb->changeItem(s.data(), 1);
}

QString App::getPlayerName() {
  QDialog *dlg = new QDialog(this, "Hall Of Fame", TRUE);

  QLabel  *l1  = new QLabel(locale->translate("You've made in into the \"Hall Of Fame\".Type in\nyour name so mankind will always remember\nyour cool rating."), dlg);
  l1->setFixedSize(l1->sizeHint());

  QLabel *l2 = new QLabel(locale->translate("Your name:"), dlg);
  l2->setFixedSize(l2->sizeHint());

  QLineEdit *e = new QLineEdit(dlg);
  e->setText("XXXXXXXXXXXXXXXX");
  e->setMinimumWidth(e->sizeHint().width());
  e->setFixedHeight(e->sizeHint().height());
  e->setText("");
  e->setFocus();

  QPushButton *b = new QPushButton(i18n("OK"), dlg);
  b->setDefault(TRUE);
  if(style() == MotifStyle)
    b->setFixedSize(b->sizeHint().width() + 10,
		    b->sizeHint().height() +10);
  else
    b->setFixedSize(b->sizeHint());
  connect(b, SIGNAL(released()), dlg, SLOT(accept()));
  connect(e, SIGNAL(returnPressed()), 
	  dlg, SLOT(accept()));

  // create layout
  QVBoxLayout *tl = new QVBoxLayout(dlg, 10);
  QHBoxLayout *tl1 = new QHBoxLayout();
  tl->addWidget(l1);
  tl->addSpacing(5);
  tl->addLayout(tl1);
  tl1->addWidget(l2);
  tl1->addWidget(e);
  tl->addSpacing(5);
  tl->addWidget(b);
  tl->activate();
  tl->freeze();

  dlg->exec();

  QString s = e->text();
  delete dlg;

  if(s.length() == 0)
    s = " ";
  return s;
}

int App::getScore(HighScore &hs) {
  double ntiles = hs.x*hs.y;
  double tilespersec = ntiles/(double)hs.seconds;
  
  double sizebonus = sqrt(ntiles/(double)(14.0 * 6.0));
  double points = tilespersec / 0.14 * 100.0;
  return (int)(points * sizebonus);
}

bool App::isBetter(HighScore &hs, HighScore &than) {
  if(getScore(hs) > getScore(than))
    return TRUE;
  else
    return FALSE;
}


int App::insertHighscore(HighScore &hs) {
  int i;

  if(highscore.size() == 0) {
    highscore.resize(1);
    highscore[0] = hs;
    writeHighscore();
    return 0;
  } else {
    HighScore last = highscore[highscore.size() - 1];
    if(isBetter(hs, last) || (highscore.size() < HIGHSCORE_MAX)) {
      if(highscore.size() == HIGHSCORE_MAX)
	highscore[HIGHSCORE_MAX - 1] = hs;
      else {
	highscore.resize(highscore.size()+1);
	highscore[highscore.size() - 1] = hs;
      }

      // sort in new entry
      int bestsofar = highscore.size() - 1;
      for(i = highscore.size() - 1; i > 0; i--)
	if(isBetter(highscore[i], highscore[i-1])) {
	  // swap entries
	  HighScore temp = highscore[i-1];
	  highscore[i-1] = highscore[i];
	  highscore[i] = temp;
	  bestsofar = i - 1;
	}

      writeHighscore();
      return bestsofar;
    }
  }
  return -1;
}


void App::readHighscore() {
  int i;
  QString s, e, grp;
  KConfig *conf = kapp->getConfig();

  highscore.resize(0);
  i = 0;
  bool eol = FALSE;
  grp = conf->group();
  conf->setGroup("Hall of Fame");
  while ((i < (int)HIGHSCORE_MAX) && !eol) {
    s.sprintf("Highscore_%d", i);
    if(conf->hasKey(s)) {
      e = conf->readEntry(s.data());
      highscore.resize(i+1);

      HighScore hs;
      memset(hs.name, 0, sizeof(hs.name));
      sscanf((const char *)e, "%d %d %d %ld %30c",
	     &hs.x, &hs.y, &hs.seconds, &hs.date, (char*)&hs.name);
      highscore[i] = hs;
    } else
      eol = TRUE;
    i++;
  }

//   // freshly installed, add my own highscore
//   if(highscore.size() == 0) {
//     HighScore hs;
//     hs.x = 28;
//     hs.y = 16;
//     hs.seconds = 367;
//     strcpy(hs.name, "Mario");
//     highscore.resize(1);
//     highscore[0] = hs;
//   }

  // restore old group
  conf->setGroup(grp.data());
}


void App::writeHighscore() {
  int i;
  QString s, e, grp;
  KConfig *conf = kapp->getConfig();

  grp = conf->group();
  conf->setGroup("Hall of Fame");
  for(i = 0; i < (int)highscore.size(); i++) {
    s.sprintf("Highscore_%d", i);
    HighScore hs = highscore[i];
    e.sprintf("%d %d %d %ld %30s",
	      hs.x, hs.y, hs.seconds, hs.date, hs.name);
    conf->writeEntry(s, e);
  }
  
  // restore old group
  conf->setGroup(grp.data());
}

void App::showHighscore(int focusitem)  {
  // this may look a little bit confusing...
  QDialog *dlg = new QDialog(0, locale->translate("Hall Of Fame"), TRUE);
  dlg->setCaption(locale->translate("Shisen-Sho: Hall Of Fame"));

  QVBoxLayout *tl = new QVBoxLayout(dlg, 10);
  
  QLabel *l = new QLabel(locale->translate("Hall Of Fame"), dlg);
  QFont f = font();
  f.setPointSize(24);
  f.setBold(TRUE);
  l->setFont(f);
  l->setFixedSize(l->sizeHint());
  l->setFixedWidth(l->width() + 32);
  l->setAlignment(AlignCenter);
  tl->addWidget(l);

  // insert highscores in a gridlayout
  QGridLayout *table = new QGridLayout(12, 5, 5);
  tl->addLayout(table, 1);

  // add a separator line
  KSeparator *sep = new KSeparator(dlg);
  table->addMultiCellWidget(sep, 1, 1, 0, 4);

  // add titles
  f = font();
  f.setBold(TRUE);
  l = new QLabel(locale->translate("Rank"), dlg);
  l->setFont(f);
  l->setMinimumSize(l->sizeHint());
  table->addWidget(l, 0, 0);
  l = new QLabel(locale->translate("Name"), dlg);
  l->setFont(f);
  l->setMinimumSize(l->sizeHint());
  table->addWidget(l, 0, 1);
  l = new QLabel(locale->translate("Time"), dlg);
  l->setFont(f);
  l->setMinimumSize(l->sizeHint());
  table->addWidget(l, 0, 2);
  l = new QLabel(locale->translate("Size"), dlg);
  l->setFont(f);
  l->setMinimumSize(l->sizeHint());
  table->addWidget(l, 0, 3);
  l = new QLabel(locale->translate("Score"), dlg);
  l->setFont(f);
  l->setMinimumSize(l->sizeHint());
  table->addWidget(l, 0, 4);
  
  QString s;
  QLabel *e[10][5];
  unsigned i, j;

  for(i = 0; i < 10; i++) {
    HighScore hs;
    if(i < highscore.size())
      hs = highscore[i];   
    
    // insert rank    
    s.sprintf("%d", i+1);
    e[i][0] = new QLabel(s.data(), dlg);

    // insert name
    if(i < highscore.size())
      e[i][1] = new QLabel(hs.name, dlg);
    else
      e[i][1] = new QLabel("", dlg);

    // insert time
    QTime ti(0,0,0);
    if(i < highscore.size()) {
      ti = ti.addSecs(hs.seconds);
      s.sprintf("%02d:%02d:%02d", ti.hour(), ti.minute(), ti.second());
      e[i][2] = new QLabel(s.data(), dlg);
    } else
      e[i][2] = new QLabel("", dlg);

    // insert size
    if(i < highscore.size()) 
      s.sprintf("%d x %d", hs.x, hs.y);
    else
      s = "";
    e[i][3] = new QLabel(s.data(), dlg);

    // insert score
    if(i < highscore.size()) 
      s.sprintf("%d", getScore(hs));
    else
      s = "";
    e[i][4] = new QLabel(s.data(), dlg);
    e[i][4]->setAlignment(AlignRight);
  }

  f = font();
  f.setBold(TRUE);
  f.setItalic(TRUE);
  for(i = 0; i < 10; i++)
    for(j = 0; j < 5; j++) {
      e[i][j]->setMinimumHeight(e[i][j]->sizeHint().height());
      if(j == 1)
	e[i][j]->setMinimumWidth(MAX(e[i][j]->sizeHint().width(), 100));
      else
	e[i][j]->setMinimumWidth(MAX(e[i][j]->sizeHint().width(), 60));
      if((int)i == focusitem)
	e[i][j]->setFont(f);
      table->addWidget(e[i][j], i+2, j, AlignCenter);	
    }
    
  QPushButton *b = new QPushButton(locale->translate("Close"), dlg);
  if(style() == MotifStyle)
    b->setFixedSize(b->sizeHint().width() + 10,
		    b->sizeHint().height() + 10);
  else
    b->setFixedSize(b->sizeHint());

  // connect the "Close"-button to done
  connect(b, SIGNAL(clicked()),
	  dlg, SLOT(accept()));
  b->setDefault(TRUE);
  b->setFocus();

  // make layout
  tl->addSpacing(10);
  tl->addWidget(b);
  tl->activate();
  tl->freeze();

  dlg->exec();
  delete dlg;
}

#include "app.moc"
