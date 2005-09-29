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


#ifndef __APP__H__
#define __APP__H__

// Should this get the whole HAVE_SYS_TIME_H TIME_WITH_SYS_TIME treatment?
#include <time.h>

#include <kmainwindow.h>
#include "board.h"

class KHighscore;

struct HighScore
{
	QString name;
	int seconds;
	int x, y;
	time_t date;
	int gravity;
};

const unsigned HIGHSCORE_MAX = 10;

class App : public KMainWindow
{
	Q_OBJECT

public:
	App(QWidget *parent = 0, const char *name=0);

private slots:
	void loadSettings();

	void slotEndOfGame();
	void enableItems();
	void updateScore();
	void showSettings();

	void newGame();
	void quitGame();
	void restartGame();
	void isSolvable();
	void pause();
	void undo();
	void redo();
	void hint();
	void hallOfFame();
	void keyBindings();
	void boardResized();

private:
	void lockMenus(bool);
	QString getPlayerName();

	/**
	 * Read the old (pre- @ref KHighscore) highscore table.
	 *
	 * This reads the config file first, then saves it in the new format and
	 * re-reads it again as a KHighscore table.
	 **/
	void readOldHighscore();
	void readHighscore();
	void writeHighscore();
	int insertHighscore(const HighScore &);
	int getScore(const HighScore &);
	bool isBetter(const HighScore &, const HighScore &);
	void showHighscore(int focusitem = -1);

	void initKAction();
	void setCheatMode();
	void resetCheatMode();

private:
	QString lastPlayerName;
	Board *board;
	QValueVector<HighScore> highscore;
	KHighscore* highscoreTable;
	bool cheat;

	enum statusBarItems { SBI_TIME, SBI_TILES, SBI_CHEAT };

};

#endif
