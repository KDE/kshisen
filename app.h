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

#include "board.h"

#include <kxmlguiwindow.h>

// Should this get the whole HAVE_SYS_TIME_H TIME_WITH_SYS_TIME treatment?
#include <ctime>

class KHighscore;
class QLabel;

/**
 * @brief Struct holding an item for the highscore list
 */
struct HighScore {
    QString name;
    int seconds;
    int x, y;
    time_t date;
    int gravity;
};

const signed HIGHSCORE_MAX = 10;

/**
 * @brief Class holding the application and its functions
 */
class App : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit App(QWidget *parent = 0);

private slots:
    void slotEndOfGame();

    /// Updates actions, to enable and disable them where needed
    void updateItems();
    void updateScore();
    /// Shows the settings dialog
    void showSettings(); // const?

    void notifyTilesDontMatch();
    void notifyInvalidMove();
    void notifySelectATile();
    void notifySelectAMatchingTile();
    void notifySelectAMove();

    /// Starts a new game
    void newGame();
    /// Restarts the current game
    void restartGame();
    // void isSolvable(); // currently not used
    /// Pauses the game
    void pause();
    /// Undoes one move
    void undo();
    /// Redoes an undone move
    void redo();
    /// Shows a hint and sets cheat flag
    void hint();
    /// Calls showHighscore without arguments
    void hallOfFame(); // const?
    void keyBindings();

private:
    /// Returns a previously entered player name
    QString getPlayerName();

    /// Reads the old (pre- @ref KHighscore) highscore table
    void readOldHighscore(); // still needed?
    void readHighscore(); // const?
    void writeHighscore();
    int insertHighscore(const HighScore &);
    int getScore(const HighScore &); // const?
    /// Compares the two given scores
    bool isBetter(const HighScore &, const HighScore &); // const?
    /// Shows the highscore table
    void showHighscore(int focusitem = -1); // const?

    /// Sets up the status bar areas
    void setupStatusBar();
    /// Sets up the needed actions and adds them to the action collection
    void setupActions();
    /// Sets the cheat mode
    void setCheatMode(bool b = true);

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
