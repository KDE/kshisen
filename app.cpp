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
#include <qtimer.h>
#include <qaccel.h>
#include <kiconloader.h>
#include <kseparator.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <debug.h>
#include <math.h>
#include <qlineedit.h>
#include <stdio.h>
#include <kglobal.h>
#include <qpushbutton.h>
#include <kconfig.h>

#define ID_FQUIT	101

#define ID_GFIRST       201
#define ID_GUNDO	201
#define ID_GREDO	202
#define ID_GHOF		203
#define ID_GRESTART	204
#define ID_GNEW		205
#define ID_GHINT	206
#define ID_GISSOLVE	207
#define ID_GPAUSE       208
#define ID_GLAST        208

#ifdef DEBUGGING
#define ID_GFINISH      220
#undef  ID_GLAST
#define ID_GLAST        208
#endif

#define ID_OFIRST       300
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
#define ID_OGRAVITY     315
#define ID_OLAST        315

#define ID_HTUTORIAL	901
#define ID_HHELP	900	

static int size_x[5] = {14, 18, 24, 26, 30};
static int size_y[5] = { 6,  8, 12, 14, 16};
static int DELAY[5] = {125, 250, 500, 750, 1000};

extern int MAX(int, int);

App::App() : KTopLevelWidget() {
  setCaption(i18n("Shisen-Sho"));
  readHighscore();

  cheat = FALSE;

  // create menu
  mb = new KMenuBar(this);
  QPopupMenu *fm = new QPopupMenu;
  fm->insertItem(i18n("&Quit"), ID_FQUIT);

  QPopupMenu *gm = new QPopupMenu;
  gm->insertItem(i18n("&Undo"), ID_GUNDO);
  gm->insertItem(i18n("&Redo"), ID_GREDO);
  gm->insertSeparator();
  gm->insertItem(i18n("Get &hint"), ID_GHINT);
  gm->insertSeparator();
  gm->insertItem(i18n("&New game"), ID_GNEW);
  gm->insertItem(i18n("Res&tart game"), ID_GRESTART);
  gm->insertItem(i18n("&Pause game"), ID_GPAUSE);
  gm->insertSeparator();
  gm->insertItem(i18n("Is game solvable?"), ID_GISSOLVE);
  gm->insertSeparator();
  gm->insertItem(i18n("Hall of &Fame"), ID_GHOF);
#ifdef DEBUGGING
  gm->insertSeparator();
  gm->insertItem("&Finish", ID_GFINISH);
#endif

  QPopupMenu *om = new QPopupMenu;
  om->setCheckable(TRUE);
  QPopupMenu *om_s = new QPopupMenu;
  om_s->setCheckable(TRUE);
  om_s->insertItem(i18n("14x6"), ID_OSIZE1);
  om_s->insertItem(i18n("18x8"), ID_OSIZE2);
  om_s->insertItem(i18n("24x12"), ID_OSIZE3);
  om_s->insertItem(i18n("26x14"), ID_OSIZE4);
  om_s->insertItem(i18n("30x16"), ID_OSIZE5);
  //om_s->insertItem(i18n("Custom size..."), ID_OSIZECUSTOM);
  QPopupMenu *om_sp = new QPopupMenu;
  om_sp->setCheckable(TRUE);
  om_sp->insertItem(i18n("Very fast"), ID_OSPEED1);
  om_sp->insertItem(i18n("Fast"), ID_OSPEED2);
  om_sp->insertItem(i18n("Medium"), ID_OSPEED3);
  om_sp->insertItem(i18n("Slow"), ID_OSPEED4);
  om_sp->insertItem(i18n("Very slow"), ID_OSPEED5);
  QPopupMenu *om_l = new QPopupMenu;
  om_l->insertItem(i18n("Easy"), ID_OLVL1);
  om_l->insertItem(i18n("Medium"), ID_OLVL2);
  om_l->insertItem(i18n("Hard"), ID_OLVL3);

  QPopupMenu *help = kapp->getHelpMenu(true, i18n("Shisen-Sho")
                                         + " " + KSHISEN_VERSION
                                         + i18n("\n\nby Mario Weilguni")
                                         + " (mweilguni@sime.com)");

  mb->insertItem(i18n("&File"), fm);
  mb->insertItem(i18n("&Game"), gm);
  om->insertItem(i18n("Si&ze"), om_s);
  om->insertItem(i18n("S&peed"), om_sp);
  om->insertItem(i18n("&Level"), om_l);
  om->insertItem(i18n("G&ravity"), ID_OGRAVITY);
  om->insertItem(i18n("Disallow unsolvable games"), ID_OSOLVABLE);
  mb->insertItem(i18n("&Options"), om);
  mb->insertSeparator();
  mb->insertItem(i18n("&Help"), help);

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
  sb->insertItem(i18n("Your time: XX:XX:XX (XXXXXXXXXXXXXXX)"), 1);
  sb->insertItem(i18n("Cheat mode"), 2);
  sb->show();
  setStatusBar(sb);
  sb->changeItem("", 2);

  tb = new KToolBar(this);
  connect(tb, SIGNAL(clicked(int)),
	  this, SLOT(menuCallback(int)));

  tb->insertButton(BarIcon("exit"), 
		   ID_FQUIT, TRUE, i18n("Quit"));
  tb->insertButton(BarIcon("back"), 
		   ID_GUNDO, TRUE, i18n("Undo"));
  tb->insertButton(BarIcon("forward"), 
		   ID_GREDO, TRUE, i18n("Redo"));
  tb->insertButton(BarIcon("help"), 
		   ID_HHELP, TRUE, i18n("Help"));
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

  _b = conf->readNumEntry("Gravity", 1) > 0;
  b->setGravityFlag(_b);
  mb->setItemChecked(ID_OGRAVITY, _b);

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

  case ID_GPAUSE: 
    {
      bool paused = b->pause();      
      lockMenus(paused);
      if(paused)
	mb->changeItem(i18n("Resume game"), ID_GPAUSE);
      else
	mb->changeItem(i18n("Pause game"), ID_GPAUSE);
    }
    break;

  case ID_GISSOLVE:
    if(b->solvable())
	QMessageBox::information(this, i18n("Information"),
				i18n("This game is solveable"), i18n("OK"));
    else
	QMessageBox::information(this, i18n("Information"),
				 i18n("This game is NOT solveable"), i18n("OK"));
    break;

  case ID_GHINT:
#ifdef DEBUGGING
    b->makeHintMove();
#else
    b->getHint();
    cheat = TRUE;
    sb->changeItem(i18n("Cheat mode"), 2);
#endif
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
      sb->changeItem(i18n("Cheat mode"), 2);
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

  case ID_OGRAVITY:
    if(!b->canUndo()) {
      b->setGravityFlag(!b->gravityFlag());
      kapp->getConfig()->writeEntry("Gravity", (int)b->gravityFlag());
    }
    break;

  default:
    printf("kshisen: unimplemented command %d\n", id);
  }

  enableItems();
}

void App::lockMenus(bool lock) {
  int i;
  
  for(i = ID_GFIRST; i <= ID_GLAST; i++)
    mb->setItemEnabled(i, !lock | i==ID_GPAUSE);
  
  for(i = ID_OFIRST; i <= ID_OLAST; i++)
    mb->setItemEnabled(i, !lock);
  enableItems();
}

void App::enableItems() {
  if(!b->isPaused()) {
    mb->setItemEnabled(ID_GUNDO, b->canUndo());
    mb->setItemEnabled(ID_GREDO, b->canRedo());
    mb->setItemEnabled(ID_GRESTART, b->canUndo());
    tb->setItemEnabled(ID_GUNDO, b->canUndo());
    tb->setItemEnabled(ID_GREDO, b->canRedo());
    tb->setItemEnabled(ID_GRESTART, b->canUndo());
    mb->setItemEnabled(ID_OGRAVITY, !b->canUndo());
    mb->setItemChecked(ID_OGRAVITY, b->gravityFlag());
  }
}

void App::sizeChanged() {
  b->setFixedSize(b->sizeHint());
  updateRects();
}

void App::slotEndOfGame() {
  if(b->tilesLeft() > 0)
      QMessageBox::information(this, i18n("End of game"),
			       i18n("No more moves possible!"),
			       i18n("OK"));
  else {
    // create highscore entry
    HighScore hs;
    hs.seconds = b->getTimeForGame();
    hs.x = b->x_tiles();
    hs.y = b->y_tiles();
    hs.gravity = (int)b->gravityFlag();

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
		i18n("Congratulations! You made it in %02d:%02d:%02d"),
		b->getTimeForGame()/3600,
		(b->getTimeForGame() / 60)  % 60,
		b->getTimeForGame() % 60);
      
      QMessageBox::information(this, i18n("End of game"), s, i18n("OK"));
    }
  }

  cheat = FALSE;
  sb->changeItem("", 2);
  b->newGame();
}

void App::updateScore() {
  QString s = i18n("Your time: %1:%2:%3 %4")
		.arg(QString().sprintf("%02d", b->getTimeForGame()/3600))
		.arg(QString().sprintf("%02d", (b->getTimeForGame() / 60) % 60))
		.arg(QString().sprintf("%02d", b->getTimeForGame() % 60))
		.arg(b->isPaused()?i18n(" (Paused)"):QString::null);

  sb->changeItem(s, 1);
}

QString App::getPlayerName() {
  QDialog *dlg = new QDialog(this, "Hall Of Fame", TRUE);

  QLabel  *l1  = new QLabel(i18n("You've made in into the \"Hall Of Fame\".Type in\nyour name so mankind will always remember\nyour cool rating."), dlg);
  l1->setFixedSize(l1->sizeHint());

  QLabel *l2 = new QLabel(i18n("Your name:"), dlg);
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

  if(hs.gravity)
    return (int)(2.0 * points * sizebonus);
  else
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
      e = conf->readEntry(s);
      highscore.resize(i+1);

      HighScore hs;
      memset(hs.name, 0, sizeof(hs.name));
      
      int nelem;
      nelem = sscanf((const char *)e, "%d %d %d %ld %d %30c",
		     &hs.x, &hs.y, &hs.seconds, &hs.date, 
		     &hs.gravity, (char*)&hs.name);
    
      // old version <= 1.1
      if(nelem == 4) {
	nelem = sscanf((const char *)e, "%d %d %d %ld %30c",
		       &hs.x, &hs.y, &hs.seconds, &hs.date, 
		       (char*)&hs.name);
	hs.gravity=0;
      }      

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
  conf->setGroup(grp);
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
    e.sprintf("%d %d %d %ld %d %30s",
	      hs.x, hs.y, hs.seconds, hs.date, hs.gravity, hs.name);
    conf->writeEntry(s, e);
  }
  
  // restore old group
  conf->setGroup(grp);
}

void App::showHighscore(int focusitem)  {
  // this may look a little bit confusing...
  QDialog *dlg = new QDialog(0, i18n("Hall Of Fame"), TRUE);
  dlg->setCaption(i18n("Shisen-Sho: Hall Of Fame"));

  QVBoxLayout *tl = new QVBoxLayout(dlg, 10);
  
  QLabel *l = new QLabel(i18n("Hall Of Fame"), dlg);
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
  l = new QLabel(i18n("Rank"), dlg);
  l->setFont(f);
  l->setMinimumSize(l->sizeHint());
  table->addWidget(l, 0, 0);
  l = new QLabel(i18n("Name"), dlg);
  l->setFont(f);
  l->setMinimumSize(l->sizeHint());
  table->addWidget(l, 0, 1);
  l = new QLabel(i18n("Time"), dlg);
  l->setFont(f);
  l->setMinimumSize(l->sizeHint());
  table->addWidget(l, 0, 2);
  l = new QLabel(i18n("Size"), dlg);
  l->setFont(f);
  l->setMinimumSize(l->sizeHint());
  table->addWidget(l, 0, 3);
  l = new QLabel(i18n("Score"), dlg);
  l->setFont(f);
  l->setMinimumSize(l->sizeHint().width()*3, l->sizeHint().height());
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
    e[i][0] = new QLabel(s, dlg);

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
      e[i][2] = new QLabel(s, dlg);
    } else
      e[i][2] = new QLabel("", dlg);

    // insert size
    if(i < highscore.size()) 
      s.sprintf("%d x %d", hs.x, hs.y);
    else
      s = "";
    e[i][3] = new QLabel(s, dlg);

    // insert score
    if(i < highscore.size()) 
      s = QString("%1 %2")
	.arg(getScore(hs))
	.arg(hs.gravity ? i18n("(gravity)") : QString(""));
    else
      s = "";
    e[i][4] = new QLabel(s, dlg);
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
    
  QPushButton *b = new QPushButton(i18n("Close"), dlg);
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
