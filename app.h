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

#include "kapp.h"
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <kmenubar.h>
#include <klocale.h>
#include <ktopwidget.h>
#include <time.h>
#include "board.h"

struct HighScore {
  char name[32];
  int  seconds;
  int  x, y;
  time_t date;
};

const unsigned HIGHSCORE_MAX = 10;

class App : public KTopLevelWidget {
  Q_OBJECT
public:
  App();
  ~App();

private slots:
  void menuCallback(int);
  void slotEndOfGame();
  void enableItems();
  void sizeChanged();
  void updateScore();

private:
  QString getPlayerName();
  void readHighscore();
  void writeHighscore();
  int  insertHighscore(HighScore &);
  int  getScore(HighScore &);
  bool isBetter(HighScore &, HighScore &);
  void showHighscore(int focusitem = -1);

private:
  Board *b;
  KMenuBar *mb;
  KToolBar *tb;
  KStatusBar *sb;
  KLocale *locale;
  QArray<HighScore> highscore;
  bool cheat;
};

#endif
