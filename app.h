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


#ifndef __APP__H__
#define __APP__H__

#include <kmainwindow.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <kmenubar.h>
#include <klocale.h>
#include <time.h>
#include "board.h"

 class KHighscore;

struct HighScore {
  char name[32];
  int  seconds;
  int  x, y;
  time_t date;
  int gravity;
};

const unsigned HIGHSCORE_MAX = 10;

class App : public KMainWindow {
  Q_OBJECT

public:
  App();
  ~App();

private slots:
  void slotEndOfGame();
  void enableItems();
  void sizeChanged();
  void updateScore();

  void newGame();
  void quitGame();
  void restartGame();
  void isSolvable();
  void pause();
  void undo();
  void redo();
  void hint();
  void toggleDisallowUnsolvable();
  void toggleGravity();
  void changeLevel();
  void changeSpeed();
  void changeSize();
  void hallOfFame();



private:
  void lockMenus(bool);
  QString getPlayerName();

  /**
   * Read the old (befor @ref KHighscore) highscore table.
   *
   * This reads the config file first, then saves it in the new format and
   * re-reads it again as a KHighscore table.
   **/
  void readOldHighscore();
  void readHighscore();
  void writeHighscore();
  int  insertHighscore(HighScore &);
  int  getScore(HighScore &);
  bool isBetter(HighScore &, HighScore &);
  void showHighscore(int focusitem = -1);

  void initKAction();

private:
  Board *b;
  KStatusBar *sb;
  QArray<HighScore> highscore;
  KHighscore* highscoreTable;
  bool cheat;
};

#endif
