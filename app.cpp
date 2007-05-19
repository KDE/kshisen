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
 * Copyright (C)  1997 by Mario Weilguni <mweilguni@sime.com>
 * Copyright (C) 2002-2004 Dave Corrie  <kde@davecorrie.com>
 * Copyright (C) 2007 Mauricio Piacentini <mauricio@tabuleiro.com>
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

#include "app.h"

#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kseparator.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kaction.h>
#include <kstandardgameaction.h>
#include <khighscore.h>
#include <kdebug.h>
#include <kshortcutsdialog.h>
#include <kmenu.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <KStandardGuiItem>
#include <kconfigdialog.h>

#include <QLayout>
#include <QTimer>
#include <QLineEdit>

#include <cmath>
#include <kglobal.h>
#include "prefs.h"
#include "ui_settings.h"

#include <kmahjonggconfigdialog.h>

class Settings : public QWidget, public Ui::Settings
{
public:
	Settings(QWidget *parent)
		: QWidget(parent)
	{
		setupUi(this);
	}
};

App::App(QWidget *parent) : KXmlGuiWindow(parent),
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

	setupStatusBar();
	initKAction();

	board = new Board(this);
	board->setObjectName( "board" );

	setCentralWidget(board);

	setupGUI();

	connect(board, SIGNAL(changed()), this, SLOT(enableItems()));
	connect(board, SIGNAL(tilesDontMatch()), this, SLOT(notifyTilesDontMatch()));
	connect(board, SIGNAL(invalidMove()), this, SLOT(notifyInvalidMove()));
	connect(board, SIGNAL(selectATile()), this, SLOT(notifySelectATile()));
	connect(board, SIGNAL(selectAMatchingTile()), this, SLOT(notifySelectAMatchingTile()));
	connect(board, SIGNAL(selectAMove()), this, SLOT(notifySelectAMove()));

	QTimer *t = new QTimer(this);
	t->start(1000);
	connect(t, SIGNAL(timeout()), this, SLOT(updateScore()));
	connect(board, SIGNAL(endOfGame()), this, SLOT(slotEndOfGame()));

	qApp->processEvents();

	updateScore();
	enableItems();
}

void App::setupStatusBar()
{
	gameTipLabel= new QLabel(i18n("Select a tile"), statusBar());
	statusBar()->addWidget(gameTipLabel, 1);
	
	gameTimerLabel = new QLabel(i18n("Time: 0:00:00"), statusBar());
	statusBar()->addWidget(gameTimerLabel);
	
	gameTilesLabel = new QLabel(i18n("Removed: 0/0"), statusBar());
	statusBar()->addWidget(gameTilesLabel);
	
	gameCheatLabel = new QLabel(i18n("Cheat mode"), statusBar());
	statusBar()->addWidget(gameCheatLabel);
	gameCheatLabel->hide();
}

void App::initKAction()
{
	// Game
	KStandardGameAction::gameNew(this, SLOT(newGame()), actionCollection());
	KStandardGameAction::restart(this, SLOT(restartGame()), actionCollection());
	KStandardGameAction::pause(this, SLOT(pause()), actionCollection());
	KStandardGameAction::highscores(this, SLOT(hallOfFame()), actionCollection());
	KStandardGameAction::quit(this, SLOT(close()), actionCollection());

	// Move
	KStandardGameAction::undo(this, SLOT(undo()), actionCollection());
	KStandardGameAction::redo(this, SLOT(redo()), actionCollection());
	KStandardGameAction::hint(this, SLOT(hint()), actionCollection());
	//new KAction(i18n("Is Game Solvable?"), 0, this,
	//	SLOT(isSolvable()), actionCollection(), "move_solvable");

#ifdef DEBUGGING
	// broken ..
	//(void)new KAction(i18n("&Finish"), 0, board, SLOT(finish()), actionCollection(), "move_finish");
#endif

	// Settings
	KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());
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
	lockMenus(board->pause());
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

void App::lockMenus(bool lock)
{
	// Disable all actions apart from (un)pause, quit and those that are help-related.
	// (Only undo/redo and hint actually *need* to be disabled, but disabling everything
	// provides a good visual hint to the user, that they need to unpause to continue.
	KMenu* help = findChild<KMenu*>("help" );
	QList<QAction*> actions = actionCollection()->actions();
	QList<QAction*>::const_iterator actionIter = actions.begin();
	QList<QAction*>::const_iterator actionIterEnd = actions.end();

	while(actionIter != actionIterEnd)
	{
		QAction* a = *actionIter;
		if(!a->associatedWidgets().contains(help))
			a->setEnabled(!lock);
		++actionIter;
	}

	actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Pause))->setEnabled(true);
	actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Quit))->setEnabled(true);

	enableItems();
}

void App::enableItems()
{
	if(!board->isPaused())
	{
		actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Undo))->setEnabled(board->canUndo());
		actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Redo))->setEnabled(board->canRedo());
		actionCollection()->action(KStandardGameAction::name(KStandardGameAction::Restart))->setEnabled(board->canUndo());
	}
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
			QString s = i18n("Congratulations! You made it in %1:%2:%3",
				 QString().sprintf("%02d", board->getTimeForGame()/3600),
				 QString().sprintf("%02d", (board->getTimeForGame() / 60) % 60),
				 QString().sprintf("%02d", board->getTimeForGame() % 60));

			if(isHighscore) // player would have been in the hisghscores if he did not cheat
			{
				s += '\n' + i18n("You could have been in the higscores if you did not use Undo or Hint. Try without them next time.");
			}

			KMessageBox::information(this, s, i18n("End of Game"));
		}
	}

	resetCheatMode();
	board->newGame();
}

void App::notifySelectATile()
{
	gameTipLabel->setText(i18n("Select a tile"));
}

void App::notifySelectAMatchingTile()
{
	gameTipLabel->setText(i18n("Select a matching tile"));
}

void App::notifySelectAMove()
{
	gameTipLabel->setText(i18n("Select the move you want by clicking on the blue line"));
}

void App::notifyTilesDontMatch()
{
	gameTipLabel->setText(i18n("This tile did not match the one you selected"));
}

void App::notifyInvalidMove()
{
	gameTipLabel->setText(i18n("You cannot make this move"));
}

void App::updateScore()
{
	int t = board->getTimeForGame();
	QString s = i18n(" Your time: %1:%2:%3 %4",
		 QString().sprintf("%02d", t / 3600 ),
		 QString().sprintf("%02d", (t / 60) % 60 ),
		 QString().sprintf("%02d", t % 60 ),
		 board->isPaused()?i18n("(Paused) "):QString());

	//statusBar()->changeItem(s, SBI_TIME);
	gameTimerLabel->setText(s);

	// Number of tiles
	int tl = (board->x_tiles() * board->y_tiles());
	s = i18n(" Removed: %1/%2 ",
		 QString().sprintf("%d", tl - board->tilesLeft()),
		 QString().sprintf("%d", tl ));

	//statusBar()->changeItem(s, SBI_TILES);
	gameTilesLabel->setText(s);
}

void App::setCheatMode()
{
	// set the cheat mode if not set
	if(!cheat)
	{
		cheat = true;
		gameCheatLabel->show();
	}
}

void App::resetCheatMode()
{
	// reset cheat mode if set
	if(cheat)
	{
		cheat = false;
		gameCheatLabel->hide();
	}
}

QString App::getPlayerName()
{
	KDialog *dlg = new KDialog(this);
	dlg->setObjectName( "Hall of Fame" );
	dlg->setModal( true );
        QLabel  *l1  = new QLabel(i18n("You have made it into the \"Hall Of Fame\". Type in\nyour name so mankind will always remember\nyour cool rating."), dlg);
	l1->setFixedSize(l1->sizeHint());

	QLabel *l2 = new QLabel(i18n("Your name:"), dlg);
	l2->setFixedSize(l2->sizeHint());

	QLineEdit *e = new QLineEdit(dlg);
	e->setText("XXXXXXXXXXXXXXXX");
	e->setMinimumWidth(e->sizeHint().width());
	e->setFixedHeight(e->sizeHint().height());
	e->setText( lastPlayerName );
	e->setFocus();

	QPushButton *b = new KPushButton(KStandardGuiItem::ok(), dlg);
	b->setDefault(true);
	b->setFixedSize(b->sizeHint());

	connect(b, SIGNAL(released()), dlg, SLOT(accept()));
	connect(e, SIGNAL(returnPressed()), dlg, SLOT(accept()));

	// create layout
	QVBoxLayout *tl = new QVBoxLayout(dlg);
	tl->setMargin( 10 );
	QHBoxLayout *tl1 = new QHBoxLayout();
	tl->addWidget(l1);
	tl->addSpacing(5);
	tl->addLayout(tl1);
	tl1->addWidget(l2);
	tl1->addWidget(e);
	tl->addSpacing(5);
	tl->addWidget(b);
	tl->activate();
        tl->setSizeConstraint(QLayout::SetFixedSize);

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

	for (int i = 0; i < hi_x.count(); i++)
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
	QString s, e;
	KSharedConfig::Ptr conf = KGlobal::config();

	highscore.resize(0);
	i = 0;
	bool eol = false;
	KConfigGroup group = conf->group("Hall of Fame");
	while ((i < (int)HIGHSCORE_MAX) && !eol)
	{
		s.sprintf("Highscore_%d", i);
		if(conf->hasKey(s))
		{
			e = group.readEntry(s,QString());
			highscore.resize(i+1);

			HighScore hs;

			QStringList e = group.readEntry(s,QStringList(), ' ');
			int nelem = e.count();
			hs.x = e.at(0).toInt();
			hs.y = e.at(1).toInt();
			hs.seconds = e.at(2).toInt();
			hs.date = e.at(3).toInt();

			if(nelem == 4) // old version <= 1.1
			{
				hs.gravity = 0;
				hs.name = e.at(4);
			}
			else
			{
				hs.gravity = e.at(4).toInt();
				hs.name = e.at(5);
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
	highscoreTable->writeAndUnlock();
}

void App::showHighscore(int focusitem)
{
	// this may look a little bit confusing...
	KDialog *dlg = new KDialog;
	dlg->setObjectName( "hall_Of_fame" );
	dlg->setModal( true );
	dlg->setWindowTitle(i18n("Hall of Fame"));

	QVBoxLayout *tl = new QVBoxLayout(dlg);
	tl->setMargin( 10 );

	QLabel *l = new QLabel(i18n("Hall of Fame"), dlg);
	QFont f = font();
	f.setPointSize(24);
	f.setBold(true);
	l->setFont(f);
	l->setFixedSize(l->sizeHint());
	l->setFixedWidth(l->width() + 32);
	l->setAlignment(Qt::AlignCenter);
	tl->addWidget(l);

	// insert highscores in a gridlayout
	QGridLayout *table = new QGridLayout;
	tl->setSpacing(5);
	tl->addLayout(table, 1);

	// add a separator line
	KSeparator *sep = new KSeparator(dlg);
	table->addWidget(sep, 1, 0, 1, 5);

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
	signed i, j;

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
		e[i][4]->setAlignment(Qt::AlignRight);
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
				e[i][j]->setMinimumWidth(qMax(e[i][j]->sizeHint().width(), 100));
			else
				e[i][j]->setMinimumWidth(qMax(e[i][j]->sizeHint().width(), 60));

			if((int)i == focusitem)
				e[i][j]->setFont(f);

			table->addWidget(e[i][j], i+2, j, Qt::AlignCenter);
		}
	}

	QPushButton *b = new KPushButton(KStandardGuiItem::close(), dlg);

	b->setFixedSize(b->sizeHint());

	// connect the "Close"-button to done
	connect(b, SIGNAL(clicked()), dlg, SLOT(accept()));
	b->setDefault(true);
	b->setFocus();

	// make layout
	tl->addSpacing(10);
	tl->addWidget(b);
	tl->activate();
	tl->setSizeConstraint(QLayout::SetFixedSize);

	dlg->exec();
	delete dlg;
}

void App::keyBindings()
{
	KShortcutsDialog::configure( actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, this );

}

/**
 * Show Settings dialog.
 */
void App::showSettings(){
	if(KConfigDialog::showDialog("settings"))
		return;
	
	//Use the classes exposed by LibKmahjongg for our configuration dialog
	KMahjonggConfigDialog *dialog = new KMahjonggConfigDialog(this, "settings", Prefs::self());
	dialog->addPage(new Settings(0), i18n("General"), "package_settings");
	dialog->addTilesetPage();
	dialog->addBackgroundPage();
	connect(dialog, SIGNAL(settingsChanged(const QString &)), board, SLOT(loadSettings()));
	dialog->show();
}

#include "app.moc"
