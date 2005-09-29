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
#include <kseparator.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kaction.h>
#include <kstdgameaction.h>
#include <khighscore.h>
#include <kdebug.h>
#include <kkeydialog.h>
#include <kpopupmenu.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kconfigdialog.h>

#include <qlayout.h>
#include <qtimer.h>
#include <qlineedit.h>

#include <cmath>

#include "app.h"
#include "prefs.h"
#include "settings.h"

App::App(QWidget *parent, const char *name) : KMainWindow(parent, name),
   cheat(false)
{
	highscoreTable = new KHighscore(this);

	// TODO?
	// Would it make sense long term to have a kconfig update rather then
	// havin both formats supported in the code?
	if(highscoreTable->hasTable())
		readHighscore();
	else
		readOldHighscore();

	statusBar()->insertItem("", SBI_TIME);
	statusBar()->insertItem("", SBI_TILES);
	statusBar()->insertFixedItem(i18n(" Cheat mode "), SBI_CHEAT);
	statusBar()->changeItem("", SBI_CHEAT);

	initKAction();

	board = new Board(this, "board");
	loadSettings();

	setCentralWidget(board);

	setupGUI();

	connect(board, SIGNAL(changed()), this, SLOT(enableItems()));

	QTimer *t = new QTimer(this);
	t->start(1000);
	connect(t, SIGNAL(timeout()), this, SLOT(updateScore()));
	connect(board, SIGNAL(endOfGame()), this, SLOT(slotEndOfGame()));
	connect(board, SIGNAL(resized()), this, SLOT(boardResized()));

	kapp->processEvents();

	updateScore();
	enableItems();
}

void App::initKAction()
{
	// Game
	KStdGameAction::gameNew(this, SLOT(newGame()), actionCollection());
	KStdGameAction::restart(this, SLOT(restartGame()), actionCollection());
	KStdGameAction::pause(this, SLOT(pause()), actionCollection());
	KStdGameAction::highscores(this, SLOT(hallOfFame()), actionCollection());
	KStdGameAction::quit(this, SLOT(quitGame()), actionCollection());

	// Move
	KStdGameAction::undo(this, SLOT(undo()), actionCollection());
	KStdGameAction::redo(this, SLOT(redo()), actionCollection());
	KStdGameAction::hint(this, SLOT(hint()), actionCollection());
	//new KAction(i18n("Is Game Solvable?"), 0, this,
	//	SLOT(isSolvable()), actionCollection(), "move_solvable");

#ifdef DEBUGGING
	(void)new KAction(i18n("&Finish"), 0, board, SLOT(finish()), actionCollection(), "move_finish");
#endif

	// Settings
	KStdAction::preferences(this, SLOT(showSettings()), actionCollection());
}

void App::hallOfFame()
{
	showHighscore();
}

void App::newGame()
{
	board->newGame();
	resetCheatMode();
	enableItems();
}

void App::quitGame()
{
	kapp->quit();
}

void App::restartGame()
{
	board->setUpdatesEnabled(false);
	while(board->canUndo())
		board->undo();
	board->setUpdatesEnabled(true);
	board->update();
	enableItems();
}

void App::isSolvable()
{
	if(board->solvable())
		KMessageBox::information(this, i18n("This game is solvable."));
	else
		KMessageBox::information(this, i18n("This game is NOT solvable."));
}

void App::pause()
{
	bool paused = board->pause();
	lockMenus(paused);
}

void App::undo()
{
	if(board->canUndo())
	{
		board->undo();
		setCheatMode();
		enableItems();
	}
}

void App::redo()
{
	if(board->canRedo())
		board->redo();
	enableItems();
}

void App::hint()
{
#ifdef DEBUGGING
	board->makeHintMove();
#else
	board->showHint();
	setCheatMode();
#endif
	enableItems();
}

void App::loadSettings()
{
	// Setting 'Prefer Unscaled Tiles' to on is known to fail in the following
	//  situation: The Keramik window decoration is in use AND caption bubbles
	//  stick out above the title bar (i.e. Keramik's 'Draw small caption
	// bubbles on active windows' configuration entry is set to off) AND the
	// kshisen window is maximized.
	//
	// The user can work-around this situation by un-maximizing the window first.
	if(Prefs::unscaled())
	{
		QSize s = board->unscaledSize();

		// We would have liked to have used KMainWindow::sizeForCentralWidgetSize(),
		// but this function does not seem to work when the toolbar is docked on the
		// left. sizeForCentralWidgetSize() even reports a value 1 pixel too small
		// when the toolbar is docked at the top...
		// These bugs present in KDE: 3.1.90 (CVS >= 20030225)
		//resize(sizeForCentralWidgetSize(s));

		s += size() - board->size(); // compensate for chrome (toolbars, statusbars etc.)
		resize(s);
		//kdDebug() << "App::preferUnscaled() set size to: " << s.width() << " x " << s.height() << endl;
	}
}

void App::lockMenus(bool lock)
{
	// Disable all actions apart from (un)pause, quit and those that are help-related.
	// (Only undo/redo and hint actually *need* to be disabled, but disabling everything
	// provides a good visual hint to the user, that they need to unpause to continue.
	KPopupMenu* help = dynamic_cast<KPopupMenu*>(child("help", "KPopupMenu", false));
	KActionPtrList actions = actionCollection()->actions();
	KActionPtrList::iterator actionIter = actions.begin();
	KActionPtrList::iterator actionIterEnd = actions.end();

	while(actionIter != actionIterEnd)
	{
		KAction* a = *actionIter;
		if(!a->isPlugged(help))
			a->setEnabled(!lock);
		++actionIter;
	}

	actionCollection()->action(KStdGameAction::name(KStdGameAction::Pause))->setEnabled(true);
	actionCollection()->action(KStdGameAction::name(KStdGameAction::Quit))->setEnabled(true);

	enableItems();
}

void App::enableItems()
{
	if(!board->isPaused())
	{
		actionCollection()->action(KStdGameAction::name(KStdGameAction::Undo))->setEnabled(board->canUndo());
		actionCollection()->action(KStdGameAction::name(KStdGameAction::Redo))->setEnabled(board->canRedo());
		actionCollection()->action(KStdGameAction::name(KStdGameAction::Restart))->setEnabled(board->canUndo());
	}
}

void App::boardResized()
{
	// If the board has been resized to a size that requires scaled tiles, then the
	// 'Prefer Unscaled Tiles' option should be set to off.

	//kdDebug() << "App::boardResized " << b->width() << " x " << b->height() << endl;
	bool unscaled = Prefs::unscaled();
	if(unscaled && board->size() != board->unscaledSize())
		Prefs::setUnscaled(false);
}

void App::slotEndOfGame()
{
	if(board->tilesLeft() > 0)
	{
		KMessageBox::information(this, i18n("No more moves possible!"), i18n("End of Game"));
	}
	else
	{
		// create highscore entry
		HighScore hs;
		hs.seconds = board->getTimeForGame();
		hs.x = board->x_tiles();
		hs.y = board->y_tiles();
		hs.gravity = (int)board->gravityFlag();

		// check if we made it into Top10
		bool isHighscore = false;
		if(highscore.size() < HIGHSCORE_MAX)
			isHighscore = true;
		else if(isBetter(hs, highscore[HIGHSCORE_MAX-1]))
			isHighscore = true;

		if(isHighscore && !cheat)
		{
			hs.name = getPlayerName();
			hs.date = time((time_t*)0);
			int rank = insertHighscore(hs);
			showHighscore(rank);
		}
		else
		{
			QString s = i18n("Congratulations! You made it in %1:%2:%3")
				.arg(QString().sprintf("%02d", board->getTimeForGame()/3600))
				.arg(QString().sprintf("%02d", (board->getTimeForGame() / 60) % 60))
				.arg(QString().sprintf("%02d", board->getTimeForGame() % 60));

			KMessageBox::information(this, s, i18n("End of Game"));
		}
	}

	resetCheatMode();
	board->newGame();
}

void App::updateScore()
{
	int t = board->getTimeForGame();
	QString s = i18n(" Your time: %1:%2:%3 %4")
		.arg(QString().sprintf("%02d", t / 3600 ))
		.arg(QString().sprintf("%02d", (t / 60) % 60 ))
		.arg(QString().sprintf("%02d", t % 60 ))
		.arg(board->isPaused()?i18n("(Paused) "):QString::null);

	statusBar()->changeItem(s, SBI_TIME);

	// Number of tiles
	int tl = (board->x_tiles() * board->y_tiles());
	s = i18n(" Removed: %1/%2 ")
		.arg(QString().sprintf("%d", tl - board->tilesLeft()))
		.arg(QString().sprintf("%d", tl ));

	statusBar()->changeItem(s, SBI_TILES);
}

void App::setCheatMode()
{
	// set the cheat mode if not set
	if(!cheat)
	{
		cheat = true;
		statusBar()->changeItem(i18n(" Cheat mode "), SBI_CHEAT);
	}
}

void App::resetCheatMode()
{
	// reset cheat mode if set
	if(cheat)
	{
		cheat = false;
		statusBar()->changeItem("", SBI_CHEAT);
	}
}

QString App::getPlayerName()
{
	QDialog *dlg = new QDialog(this, "Hall of Fame", true);

	QLabel  *l1  = new QLabel(i18n("You've made it into the \"Hall Of Fame\". Type in\nyour name so mankind will always remember\nyour cool rating."), dlg);
	l1->setFixedSize(l1->sizeHint());

	QLabel *l2 = new QLabel(i18n("Your name:"), dlg);
	l2->setFixedSize(l2->sizeHint());

	QLineEdit *e = new QLineEdit(dlg);
	e->setText("XXXXXXXXXXXXXXXX");
	e->setMinimumWidth(e->sizeHint().width());
	e->setFixedHeight(e->sizeHint().height());
	e->setText( lastPlayerName );
	e->setFocus();

	QPushButton *b = new KPushButton(KStdGuiItem::ok(), dlg);
	b->setDefault(true);
	b->setFixedSize(b->sizeHint());

	connect(b, SIGNAL(released()), dlg, SLOT(accept()));
	connect(e, SIGNAL(returnPressed()), dlg, SLOT(accept()));

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

	lastPlayerName = e->text();
	delete dlg;

	if(lastPlayerName.isEmpty())
		return " ";
	return lastPlayerName;
}

int App::getScore(const HighScore &hs)
{
	double ntiles = hs.x*hs.y;
	double tilespersec = ntiles/(double)hs.seconds;

	double sizebonus = std::sqrt(ntiles/(double)(14.0 * 6.0));
	double points = tilespersec / 0.14 * 100.0;

	if(hs.gravity)
		return (int)(2.0 * points * sizebonus);
	else
		return (int)(points * sizebonus);
}

bool App::isBetter(const HighScore &hs, const HighScore &than)
{
	if(getScore(hs) > getScore(than))
		return true;
	else
		return false;
}

int App::insertHighscore(const HighScore &hs)
{
	int i;

	if(highscore.size() == 0)
	{
		highscore.resize(1);
		highscore[0] = hs;
		writeHighscore();
		return 0;
	}
	else
	{
		HighScore last = highscore[highscore.size() - 1];
		if(isBetter(hs, last) || (highscore.size() < HIGHSCORE_MAX))
		{
			if(highscore.size() == HIGHSCORE_MAX)
			{
				highscore[HIGHSCORE_MAX - 1] = hs;
			}
			else
			{
				highscore.resize(highscore.size()+1);
				highscore[highscore.size() - 1] = hs;
			}

			// sort in new entry
			int bestsofar = highscore.size() - 1;
			for(i = highscore.size() - 1; i > 0; i--)
			{
				if(isBetter(highscore[i], highscore[i-1]))
				{
					// swap entries
					HighScore temp = highscore[i-1];
					highscore[i-1] = highscore[i];
					highscore[i] = temp;
					bestsofar = i - 1;
				}
			}

			writeHighscore();
			return bestsofar;
		}
	}
	return -1;
}

void App::readHighscore()
{
	QStringList hi_x, hi_y, hi_sec, hi_date, hi_grav, hi_name;
	hi_x = highscoreTable->readList("x", HIGHSCORE_MAX);
	hi_y = highscoreTable->readList("y", HIGHSCORE_MAX);
	hi_sec = highscoreTable->readList("seconds", HIGHSCORE_MAX);
	hi_date = highscoreTable->readList("date", HIGHSCORE_MAX);
	hi_grav = highscoreTable->readList("gravity", HIGHSCORE_MAX);
	hi_name = highscoreTable->readList("name", HIGHSCORE_MAX);

	highscore.resize(0);

	for (unsigned int i = 0; i < hi_x.count(); i++)
	{
		highscore.resize(i+1);

		HighScore hs;

		hs.x = hi_x[i].toInt();
		hs.y = hi_y[i].toInt();
		hs.seconds = hi_sec[i].toInt();
		hs.date = hi_date[i].toInt();
		hs.date = hi_date[i].toInt();
		hs.gravity = hi_grav[i].toInt();
		hs.name = hi_name[i];

		highscore[i] = hs;
	}
}

void App::readOldHighscore()
{
	// this is for before-KHighscore-highscores
	int i;
	QString s, e, grp;
	KConfig *conf = kapp->config();

	highscore.resize(0);
	i = 0;
	bool eol = false;
	grp = conf->group();
	conf->setGroup("Hall of Fame");
	while ((i < (int)HIGHSCORE_MAX) && !eol)
	{
		s.sprintf("Highscore_%d", i);
		if(conf->hasKey(s))
		{
			e = conf->readEntry(s);
			highscore.resize(i+1);

			HighScore hs;

			QStringList e = conf->readListEntry(s, ' ');
			int nelem = e.count();
			hs.x = (*e.at(0)).toInt();
			hs.y = (*e.at(1)).toInt();
			hs.seconds = (*e.at(2)).toInt();
			hs.date = (*e.at(3)).toInt();

			if(nelem == 4) // old version <= 1.1
			{
				hs.gravity = 0;
				hs.name = *e.at(4);
			}
			else
			{
				hs.gravity = (*e.at(4)).toInt();
				hs.name = *e.at(5);
			}

			highscore[i] = hs;
		}
		else
		{
			eol = true;
		}
		i++;
	}

//	// freshly installed, add my own highscore
//	if(highscore.size() == 0)
//	{
//		HighScore hs;
//		hs.x = 28;
//		hs.y = 16;
//		hs.seconds = 367;
//		hs.name = "Mario";
//		highscore.resize(1);
//		highscore[0] = hs;
//	}

	// restore old group
	conf->setGroup(grp);

	// write in new KHighscore format
	writeHighscore();
	// read form KHighscore format
	readHighscore();
}

void App::writeHighscore()
{
	int i;
	QStringList hi_x, hi_y, hi_sec, hi_date, hi_grav, hi_name;
	for(i = 0; i < (int)highscore.size(); i++)
	{
		HighScore hs = highscore[i];
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

void App::showHighscore(int focusitem)
{
	// this may look a little bit confusing...
	QDialog *dlg = new QDialog(0, "hall_Of_fame", true);
	dlg->setCaption(i18n("Hall of Fame"));

	QVBoxLayout *tl = new QVBoxLayout(dlg, 10);

	QLabel *l = new QLabel(i18n("Hall of Fame"), dlg);
	QFont f = font();
	f.setPointSize(24);
	f.setBold(true);
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
	f.setBold(true);
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

	for(i = 0; i < 10; i++)
	{
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
		if(i < highscore.size())
		{
			ti = ti.addSecs(hs.seconds);
			s.sprintf("%02d:%02d:%02d", ti.hour(), ti.minute(), ti.second());
			e[i][2] = new QLabel(s, dlg);
		}
		else
		{
			e[i][2] = new QLabel("", dlg);
		}

		// insert size
		if(i < highscore.size())
			s.sprintf("%d x %d", hs.x, hs.y);
		else
			s = "";

		e[i][3] = new QLabel(s, dlg);

		// insert score
		if(i < highscore.size())
		{
			s = QString("%1 %2")
			    .arg(getScore(hs))
			    .arg(hs.gravity ? i18n("(gravity)") : QString(""));
		}
		else
		{
			s = "";
		}

		e[i][4] = new QLabel(s, dlg);
		e[i][4]->setAlignment(AlignRight);
	}

	f = font();
	f.setBold(true);
	f.setItalic(true);
	for(i = 0; i < 10; i++)
	{
		for(j = 0; j < 5; j++)
		{
			e[i][j]->setMinimumHeight(e[i][j]->sizeHint().height());

			if(j == 1)
				e[i][j]->setMinimumWidth(std::max(e[i][j]->sizeHint().width(), 100));
			else
				e[i][j]->setMinimumWidth(std::max(e[i][j]->sizeHint().width(), 60));

			if((int)i == focusitem)
				e[i][j]->setFont(f);

			table->addWidget(e[i][j], i+2, j, AlignCenter);
		}
	}

	QPushButton *b = new KPushButton(KStdGuiItem::close(), dlg);

	b->setFixedSize(b->sizeHint());

	// connect the "Close"-button to done
	connect(b, SIGNAL(clicked()), dlg, SLOT(accept()));
	b->setDefault(true);
	b->setFocus();

	// make layout
	tl->addSpacing(10);
	tl->addWidget(b);
	tl->activate();
	tl->freeze();

	dlg->exec();
	delete dlg;
}

void App::keyBindings()
{
	KKeyDialog::configure(actionCollection(), this);
}

/**
 * Show Settings dialog.
 */
void App::showSettings(){
	if(KConfigDialog::showDialog("settings"))
		return;

	KConfigDialog *dialog = new KConfigDialog(this, "settings", Prefs::self(), KDialogBase::Swallow);
	Settings *general = new Settings(0, "General");
	dialog->addPage(general, i18n("General"), "package_settings");
	connect(dialog, SIGNAL(settingsChanged()), this, SLOT(loadSettings()));
	connect(dialog, SIGNAL(settingsChanged()), board, SLOT(loadSettings()));
	dialog->show();
}

#include "app.moc"
