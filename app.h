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


#ifndef APP_H
#define APP_H

// Should this get the whole HAVE_SYS_TIME_H TIME_WITH_SYS_TIME treatment?
#include <time.h>

#include <kxmlguiwindow.h>
#include "board.h"

class KHighscore;
class QLabel;

struct HighScore {
    QString name;
    int seconds;
    int x, y;
    time_t date;
    int gravity;
};

const signed HIGHSCORE_MAX = 10;

class App : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit App(QWidget *parent = 0);

private slots:
    void slotEndOfGame();
    void updateItems();
    void updateScore();
    void showSettings(); // const?

    void notifyTilesDontMatch();
    void notifyInvalidMove();
    void notifySelectATile();
    void notifySelectAMatchingTile();
    void notifySelectAMove();

    void newGame();
    void restartGame();
    // void isSolvable(); // currently not used
    void pause();
    void undo();
    void redo();
    void hint();
    void hallOfFame(); // const?
    void keyBindings();

private:
    QString getPlayerName();

    /**
     * Read the old (pre- @ref KHighscore) highscore table.
     *
     * This reads the config file first, then saves it in the new format and
     * re-reads it again as a KHighscore table.
     **/
    void readOldHighscore(); // still needed?
    void readHighscore(); // const?
    void writeHighscore();
    int insertHighscore(const HighScore &);
    int getScore(const HighScore &); // const?
    bool isBetter(const HighScore &, const HighScore &); // const?
    void showHighscore(int focusitem = -1); // const?

    void setupStatusBar();
    void setupActions();
    void setCheatMode();
    void resetCheatMode();

private:
    QString m_lastPlayerName;
    QLabel *m_gameTipLabel;
    QLabel *m_gameTimerLabel;
    QLabel *m_gameTilesLabel;
    QLabel *m_gameCheatLabel;
    Board *m_board;
    QVector<HighScore> m_highscore;
    KHighscore* m_highscoreTable;
    bool m_cheat;
};

#endif // APP_H

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
