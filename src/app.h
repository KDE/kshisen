/***************************************************************************
 *   KShisen - A japanese game similar to mahjongg                         *
 *   Copyright 1997  Mario Weilguni <mweilguni@sime.com>                   *
 *   Copyright 2002-2004  Dave Corrie <kde@davecorrie.com>                 *
 *   Copyright 2009,2010  Frederik Schwarzer <schwarzerf@gmail.com>        *
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

class QLabel;

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

    /** Updates actions, to enable and disable them where needed.
     * According to the current state of the game (game over, pause ...) some
     * actions might better be disabled. This is the place to do so.
     */
    void updateItems();
    /// Updated the time display in the status bar
    void updateTimeDisplay();
    /// Updates the penalty timer in the status bar
    void updatePenaltyDisplay();
    /// Updates the tiles removed display in the status bar
    void updateTileDisplay();
    /// Shows the settings dialog
    void showSettings(); // const?

    void notifyTilesDontMatch();
    void notifyInvalidMove();
    void notifySelectATile();
    void notifySelectAMatchingTile();
    void notifySelectAMove();

    /** Sets some flags for a new game.
     * This should be called from Board::newGame().
     */
    void newGame();

    /** Restarts the current game.
     * Currently this is done by undoing all moves done by the user as yet and
     * resetting the move history and the timer.
     * This might change over time. However, just make sure, the user gets his
     * currently played game as if it was started by the New Game action.
     */
    void restartGame();
    // void isSolvable(); // currently not used
    /// Toggles the pause mode
    void togglePause();

    /** Controls the pause mode.
     * If the game is paused, do not show the board and disable actions like undo
     * and such.
     */
    void setPauseEnabled(bool enable);

    /** Undoes one move.
     * The Undo action should add a time penalty, so the user cannot end up in
     * the highscore dialog by making bad decisions. :)
     */
    void undo();
    /// Redoes an undone move
    void redo();

    /** Shows a hint.
     * The Hint action should add a time penalty, so the user cannot end up in
     * the highscore dialog by having been told what to do. :)
     */
    void hint();
    /// Calls showHighscore without arguments
    void keyBindings();
    /// Shows the highscore table
    void showHighscores(); // const?

signals:
    /** Invokes the creation of a new game.
     * This signal is connected to the newGame() slot of the Board, which
     * then does its job and sends a signal back to this class so the rest
     * of the work can be done here.
     */
    void invokeNewGame();

private:
    /// Calculates the scores
    int score(int x, int y, int seconds, bool gravity) const;

    /** Sets up the status bar areas.
     * There are four areas in the status bar:
     * - game tip
     * - timer
     * - tile count
     * - penalty time
     */
    void setupStatusBar();
    /// Sets up the needed actions and adds them to the action collection
    void setupActions();
    /// Adds the given penalty time to the game time
    void imposePenalty(int);
    /// Resets the penalty time
    void resetPenalty();

private:
    QLabel *m_gameTipLabel; ///< Status bar area for game tips
    QLabel *m_gameTimerLabel; ///< Status bar area for the timer
    QLabel *m_gamePenaltyLabel; ///< Status bar area for the penalty timer
    QLabel *m_gameTilesLabel; ///< Status bar area for the tile counter
    Board *m_board; ///< Holds the game board
    int m_penaltyTime; ///< Holds the current penalty time if the player used hint or undo
};

#endif // APP_H

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
