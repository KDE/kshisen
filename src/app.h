/*
    KShisen - A japanese game similar to Mahjongg
    SPDX-FileCopyrightText: 1997 Mario Weilguni <mweilguni@sime.com>
    SPDX-FileCopyrightText: 2002-2004 Dave Corrie <kde@davecorrie.com>
    SPDX-FileCopyrightText: 2009, 2016 Frederik Schwarzer <schwarzer@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KSHISEN_APP_H
#define KSHISEN_APP_H

// KF
#include <KXmlGuiWindow>

// Qt
class QLabel;

// KShisen
namespace KShisen
{
class Board;
}

namespace KShisen
{
/**
 * @brief Class holding the application and its functions
 */
class App : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit App(QWidget * parent = nullptr);

private Q_SLOTS:
    void slotEndOfGame();

    /** Updates actions, to enable and disable them where needed.
     * According to the current state of the game (game over, pause ...) some
     * actions might better be disabled. This is the place to do so.
     */
    void updateItems();
    /// Updated the time display in the status bar
    void updateTimeDisplay();
    /// Updates the tiles removed display in the status bar
    void updateTileDisplay();
    /// Updates the cheat status display in the status bar
    void updateCheatDisplay();
    /// Shows the settings dialog
    void showSettingsDialog(); // const?

    void notifyTilesDoNotMatch();
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
    void setPauseEnabled(bool enabled);

    /** Undoes one move.
     * The Undo action should set the cheat flag, so the user cannot end up in
     * the highscore dialog by making bad decisions. :)
     */
    void undo();
    /// Redoes an undone move
    void redo();

    /** Shows a hint and sets cheat flag.
     * The Hint action should set the cheat flag, so the user cannot end up in
     * the highscore dialog by having been told what to do. :)
     */
    void hint();
    /// Calls showHighscore without arguments
    void keyBindings();
    /// Shows the highscore table
    void showHighscores(); // const?

Q_SIGNALS:
    /** Invokes the creation of a new game.
     * This signal is connected to the newGame() slot of the Board, which
     * then does its job and sends a signal back to this class so the rest
     * of the work can be done here.
     * @see newGame()
     * @see Board::newGame()
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
     * - cheat mode
     */
    void setupStatusBar();
    /// Sets up the needed actions and adds them to the action collection
    void setupActions();
    /// Sets the cheat mode
    void setCheatModeEnabled(bool enabled);

private:
    QLabel * m_gameTipLabel{nullptr}; ///< Status bar area for game tips
    QLabel * m_gameTimerLabel{nullptr}; ///< Status bar area for the timer
    QLabel * m_gameTilesLabel{nullptr}; ///< Status bar area for the tile counter
    QLabel * m_gameCheatLabel{nullptr}; ///< Status bar area for the cheat mode
    Board * m_board{nullptr}; ///< Holds the game board
};
} // namespace KShisen

#endif // KSHISEN_APP_H

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
