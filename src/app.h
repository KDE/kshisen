/***************************************************************************
 *   KShisen - A japanese game similar to mahjongg                         *
 *   Copyright 1997  Mario Weilguni <mweilguni@sime.com>                   *
 *   Copyright 2002-2004  Dave Corrie <kde@davecorrie.com>                 *
 *   Copyright 2009  Frederik Schwarzer <schwarzerf@gmail.com>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

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
    QString name; ///< The player's name
    int seconds; ///< Time the player needed
    int x, y; ///< Size of the played game board
    time_t date;
    int gravity; ///< Whether gravity was enabled
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
    /// Toggles the pause mode
    void togglePause();
    /// Controls the pause mode
    void setPauseEnabled(bool enable);
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
    /// Shows a dialog to receive the player's name
    bool receivePlayerName();

    /// Reads the old (pre- @ref KHighscore) highscore table
    void readOldHighscore(); // still needed?
    void readHighscore(); // const?
    void writeHighscore();
    /// Inserts given highscore entry to the list
    int insertHighscore(const HighScore &);
    int score(const HighScore &); // const?
    /// Compares the two given scores
    bool isBetter(const HighScore &, const HighScore &); // const?
    /// Shows the highscore table
    void showHighscore(int focusitem = -1); // const?

    /// Sets up the status bar areas
    void setupStatusBar();
    /// Sets up the needed actions and adds them to the action collection
    void setupActions();
    /// Sets the cheat mode
    void setCheatModeEnabled(bool enabled);

private:
    QString m_lastPlayerName; ///< Holds the afore entered player name
    QLabel *m_gameTipLabel; ///< Status bar area for game tips
    QLabel *m_gameTimerLabel; ///< Status bar area for the timer
    QLabel *m_gameTilesLabel; ///< Status bar area for the tile counter
    QLabel *m_gameCheatLabel; ///< Status bar area for the cheat mode
    Board *m_board; ///< Holds the game board
    QVector<HighScore> m_highscore; ///< List of highscore entries
    KHighscore* m_highscoreTable;
    bool m_cheat; ///< Whether the cheat mode is set
};

#endif // APP_H

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
