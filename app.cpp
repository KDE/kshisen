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

#include <math.h>
#include <stdio.h>

#include <qlayout.h>
#include <qtimer.h>
#include <qaccel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qstyle.h>

#include <kapplication.h>
#include <kiconloader.h>
#include <kseparator.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kstdgameaction.h>
#include <khighscore.h>
#include <kdebug.h>

static int size_x[5] = {14, 18, 24, 26, 30};
static int size_y[5] = { 6,  8, 12, 14, 16};
static int DELAY[5] = {125, 250, 500, 750, 1000};

extern int MAX(int, int);

App::App() : KMainWindow(0) {
  setCaption(i18n("Shisen-Sho"));
  highscoreTable = new KHighscore();
  if (highscoreTable->hasTable()) {
    readHighscore();
  } else {
    readOldHighscore();
  }

  cheat = FALSE;
  

  initKAction();

  b = new Board(this);  
  setCentralWidget(b);

  sb = new KStatusBar(this);
  sb->insertItem(i18n("Your time: XX:XX:XX (XXXXXXXXXXXXXXX)"), 1);
  sb->insertItem(i18n("Cheat mode"), 2);
  sb->changeItem("", 2);

  connect(b, SIGNAL(changed()),
	  this, SLOT(enableItems()));

  connect(b, SIGNAL(sizeChange()),
	  this, SLOT(sizeChanged()));

  // load default settings
  KConfig *conf = kapp->config();
  int i;
  i = conf->readNumEntry("Speed", 2);
  ((KSelectAction*)actionCollection()->action("options_speed"))->setCurrentItem(i);
  changeSpeed();

  i = conf->readNumEntry("Size", 300 + 2);
  //if(i == ID_OSIZECUSTOM)
  //kdDebug() << "CUSTOM SIZE, TODO" << endl;
  //  else
  ((KSelectAction*)actionCollection()->action("options_size"))->setCurrentItem(i - 300);
  changeSize();

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
  ((KToggleAction*)actionCollection()->action("options_disallow"))->setChecked(b->getSolvableFlag());

  _b = conf->readNumEntry("Gravity", 1) > 0;
  b->setGravityFlag(_b);
  ((KToggleAction*)actionCollection()->action("options_gravity"))->setChecked(_b);

  kapp->processEvents();
  i = conf->readNumEntry("Level", 311 + 1);
  ((KSelectAction*)actionCollection()->action("options_level"))->setCurrentItem(i - 311);
  changeLevel();

  sizeChanged();
  enableItems();
}

App::~App() {
  delete b;
  delete sb;
  delete highscoreTable;

}

void App::initKAction() {
//Game
  KStdGameAction::gameNew(this, SLOT(newGame()), actionCollection());
  KStdGameAction::quit(this, SLOT(quitGame()), actionCollection());
  KStdGameAction::highscores(this, SLOT(hallOfFame()), actionCollection());
  KStdGameAction::pause(this, SLOT(pause()), actionCollection());

  (void)new KAction(i18n("Is Game Solvable?"), 0, this, SLOT(isSolvable()), actionCollection(), "game_solvable");
  (void)new KAction(i18n("Res&tart Game"), Qt::CTRL+Qt::Key_R, this, SLOT(restartGame()), actionCollection(), "game_restart");
  (void)new KAction(i18n("Get &Hint"), Key_H, this, SLOT(hint()), actionCollection(), "game_hint");

#ifdef DEBUGGING
  (void)new KAction("&Finish", 0, b, SLOT(finish()), actionCollection(), "game_finish");
#endif

//Edit
  KStdAction::undo(this, SLOT(undo()), actionCollection());
  KStdAction::redo(this, SLOT(redo()), actionCollection());

//Settings
  QStringList list;
  KSelectAction* size = new KSelectAction(i18n("Si&ze"), 0, this, SLOT(changeSize()), actionCollection(), "options_size");
  list.append(i18n("14x6"));
  list.append(i18n("18x8"));
  list.append(i18n("24x12"));
  list.append(i18n("26x14"));
  list.append(i18n("30x16"));
  size->setItems(list);

  list.clear();
  KSelectAction* speed = new KSelectAction(i18n("S&peed"), 0, this, SLOT(changeSpeed()), actionCollection(), "options_speed");
  list.append(i18n("Very fast"));
  list.append(i18n("Fast"));
  list.append(i18n("Medium"));
  list.append(i18n("Slow"));
  list.append(i18n("Very slow"));
  speed->setItems(list);

  list.clear();
  KSelectAction* level = new KSelectAction(i18n("&Level"), 0, this, SLOT(changeLevel()), actionCollection(), "options_level");
  list.append(i18n("Easy"));
  list.append(i18n("Medium"));
  list.append(i18n("Hard"));
  level->setItems(list);

  (void)new KToggleAction(i18n("G&ravity"), 0, this, SLOT(toggleGravity()), actionCollection(), "options_gravity");
  (void)new KToggleAction(i18n("Disallow Unsolvable Games"), 0, this, SLOT(toggleDisallowUnsolvable()), actionCollection(), "options_disallow");
  
  createGUI("kshisenui.rc");
}

void App::hallOfFame() {
  showHighscore();
}

void App::newGame() {
  b->newGame();
  cheat = FALSE;
  sb->changeItem("", 2);
  enableItems();
}

void App::quitGame() {
  delete this;
  kapp->quit();
}

void App::restartGame() {
  b->setUpdatesEnabled(FALSE);
  while(b->canUndo())
    b->undo();
  b->setUpdatesEnabled(TRUE);
  b->update();
  enableItems();
}

void App::isSolvable() {
  if(b->solvable())
	KMessageBox::information(this,
				i18n("This game is solveable"));
  else
	KMessageBox::information(this,
				 i18n("This game is NOT solveable"));
}

void App::pause() {
  bool paused = b->pause();
  lockMenus(paused);
  if(paused)
	actionCollection()->action("game_pause")->setText(i18n("R&esume game"));
  else
	actionCollection()->action("game_pause")->setText(i18n("&Pause game"));
  actionCollection()->action("game_pause")->setAccel(Qt::Key_P);
}

void App::undo() {
  if(b->canUndo()) {
    b->undo();
    cheat = TRUE;
    sb->changeItem(i18n("Cheat mode"), 2);
    enableItems();
  }
}

void App::redo() {
  if(b->canRedo()) 
    b->redo();
    enableItems();
}

void App::hint() {
#ifdef DEBUGGING
  b->makeHintMove();
#else
  b->getHint();
  cheat = TRUE;
  sb->changeItem(i18n("Cheat mode"), 2);
#endif
  enableItems();
}

void App::toggleGravity() {
  if(!b->canUndo()) {
    b->setGravityFlag(!b->gravityFlag());
    kapp->config()->writeEntry("Gravity", (int)b->gravityFlag());
  } else {
    ((KToggleAction*)actionCollection()->action("options_gravity"))->setChecked(b->gravityFlag());
  }
}

void App::toggleDisallowUnsolvable() {
  b->setSolvableFlag(!b->getSolvableFlag());
  kapp->config()->writeEntry("Solvable", (int)b->getSolvableFlag());
}

void App::changeSpeed() {
  int index = ((KSelectAction*)actionCollection()->action("options_speed"))->currentItem();
  b->setDelay(DELAY[index]);
  kapp->config()->writeEntry("Speed", index);
}

void App::changeSize() {
  int index = ((KSelectAction*)actionCollection()->action("options_size"))->currentItem();
  b->setSize(size_x[index], size_y[index]);
  b->newGame();
  kapp->config()->writeEntry("Size", 300 + index);// 300 is from the old QPopuMenu+ID way - before KAction
}

void App::changeLevel() {
  int index = ((KSelectAction*)actionCollection()->action("options_level"))->currentItem();
  b->setShuffle(index * 4 + 1);
  b->newGame();
  kapp->config()->writeEntry("Level", 311 + index); // 311 is from the old QPopuMenu+ID way - before KAction
}

void App::lockMenus(bool lock) {
//#warning FIXME
  QValueList<KAction*> list = actionCollection()->actions();
  for (int unsigned i = 0; i < list.count(); i++) {
      list[i]->setEnabled(!lock);
  }
  actionCollection()->action("game_pause")->setEnabled(true);

  //FIXME This is ugly - a better solution (which can be used directly in the loop
  //withput knowing which entries exist in the "help" menu) is wanted!
  actionCollection()->action(KStdAction::stdName(KStdAction::HelpContents))->setEnabled(true);
  actionCollection()->action(KStdAction::stdName(KStdAction::WhatsThis))->setEnabled(true);
  actionCollection()->action(KStdAction::stdName(KStdAction::ReportBug))->setEnabled(true);
  actionCollection()->action(KStdAction::stdName(KStdAction::AboutApp))->setEnabled(true);
  actionCollection()->action(KStdAction::stdName(KStdAction::AboutKDE))->setEnabled(true);

  enableItems();
}

void App::enableItems() {
  if(!b->isPaused()) {
    actionCollection()->action(KStdAction::stdName(KStdAction::Undo))->setEnabled(b->canUndo());
    actionCollection()->action(KStdAction::stdName(KStdAction::Redo))->setEnabled(b->canRedo());
    actionCollection()->action("game_restart")->setEnabled(b->canUndo());
    actionCollection()->action("options_gravity")->setEnabled(!b->canUndo());
    ((KToggleAction*)actionCollection()->action("options_gravity"))->setChecked(b->gravityFlag());
  }
}

void App::sizeChanged() {
  b->setFixedSize(b->sizeHint());
  /* The followin line fixes the autoresizing  problems.
     I don't know why, but this is the only way, it works. */
  setFixedSize(1,1);
}

void App::slotEndOfGame() {
  if(b->tilesLeft() > 0)
      KMessageBox::information(this,
			       i18n("No more moves possible!"),
			       i18n("End of game"));
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
      strncpy(hs.name, name.utf8(), sizeof(hs.name) - 1);
      hs.date = time((time_t*)0);
      hs.x = b->x_tiles();
      hs.y = b->y_tiles();
      int rank = insertHighscore(hs);
      showHighscore(rank);
    } else {
      QString s;
      s.sprintf(
		i18n("Congratulations! You made it in %02d:%02d:%02d").utf8(),
		b->getTimeForGame()/3600,
		(b->getTimeForGame() / 60)  % 60,
		b->getTimeForGame() % 60);
      
      KMessageBox::information(this, s, i18n("End of game"));
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
#if QT_VERSION < 300
  if(style().guiStyle() == MotifStyle)
    b->setFixedSize(b->sizeHint().width() + 10,
		    b->sizeHint().height() +10);
  else
#endif
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
  QStringList hi_x, hi_y, hi_sec, hi_date, hi_grav, hi_name;
  hi_x = highscoreTable->readList("x", HIGHSCORE_MAX);
  hi_y = highscoreTable->readList("y", HIGHSCORE_MAX);
  hi_sec = highscoreTable->readList("seconds", HIGHSCORE_MAX);
  hi_date = highscoreTable->readList("date", HIGHSCORE_MAX);
  hi_grav = highscoreTable->readList("gravity", HIGHSCORE_MAX);
  hi_name = highscoreTable->readList("name", HIGHSCORE_MAX);
  
  highscore.resize(0);

  for (int unsigned i = 0; i < hi_x.count(); i++) {
      highscore.resize(i+1);
    
      HighScore hs;
      memset(hs.name, 0, sizeof(hs.name));

      hs.x = hi_x[i].toInt();
      hs.y = hi_y[i].toInt();
      hs.seconds = hi_sec[i].toInt();
      hs.date = hi_date[i].toInt();
      hs.date = hi_date[i].toInt();
      hs.gravity = hi_grav[i].toInt();
      strncpy(hs.name, hi_name[i].utf8(), 16);
      
      highscore[i] = hs;
  }
}

void App::readOldHighscore() {
// this is for before-KHighscore-highscores
  int i;
  QString s, e, grp;
  KConfig *conf = kapp->config();

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
      
      QStringList e = conf->readListEntry(s, ' ');
      int nelem = e.count();
      hs.x = (*e.at(0)).toInt();
      hs.y = (*e.at(1)).toInt();
      hs.seconds = (*e.at(2)).toInt();
      hs.date = (*e.at(3)).toInt();

      if (nelem == 4) // old version <= 1.1
      {
        hs.gravity = 0;
        strncpy(hs.name, (*e.at(4)).utf8(), 16);
      }
      else
      {
        hs.gravity = (*e.at(4)).toInt();
        strncpy(hs.name, (*e.at(5)).utf8(), 16);
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

  // write in new KHighscore format
  writeHighscore();
  // read form KHighscore format
  readHighscore();
}

void App::writeHighscore() {
  int i;
  QString e;
  QStringList hi_x, hi_y, hi_sec, hi_date, hi_grav, hi_name;
  for(i = 0; i < (int)highscore.size(); i++) {
    HighScore hs = highscore[i];
//    e.sprintf("%d %d %d %ld %d %-16s",
//	      hs.x, hs.y, hs.seconds, hs.date, hs.gravity, hs.name);// original
//	      line - caused problems with "firstname lastname" names because of
//	      the space
    hi_x.append(QString::number(hs.x));
    hi_y.append(QString::number(hs.y));
    hi_sec.append(QString::number(hs.seconds));
    hi_date.append(QString::number(hs.date));
    hi_grav.append(QString::number(hs.gravity));
    hi_name.append(hs.name);
  }
  highscoreTable->writeList("x", hi_x);
  highscoreTable->writeList("y", hi_y);
  highscoreTable->writeList("seconds", hi_sec);
  highscoreTable->writeList("date", hi_date);
  highscoreTable->writeList("gravity", hi_grav);
  highscoreTable->writeList("name", hi_name);
  highscoreTable->sync();
}

void App::showHighscore(int focusitem)  {
  // this may look a little bit confusing...
  QDialog *dlg = new QDialog(0, "hall_Of_fame", TRUE);
  dlg->setCaption(i18n("Shisen-Sho: Hall of Fame"));

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
#if QT_VERSION < 300
  if(style().guiStyle() == MotifStyle)
    b->setFixedSize(b->sizeHint().width() + 10,
		    b->sizeHint().height() + 10);
  else
#endif
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
