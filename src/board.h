/***************************************************************************
 *   KShisen - A japanese game similar to mahjongg                         *
 *   Copyright 1997  Mario Weilguni <mweilguni@sime.com>                   *
 *   Copyright 2002-2004  Dave Corrie <kde@davecorrie.com>                 *
 *   Copyright 2007  Mauricio Piacentini <mauricio@tabuleiro.com>          *
 *   Copyright 2009-2016  Frederik Schwarzer <schwarzer@kde.org>           *
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

// KMahjonggLib integration and SVG support for KDE 4: Mauricio Piacentini <mauricio@tabuleiro.com>

#ifndef KSHISEN_BOARD_H
#define KSHISEN_BOARD_H

// STL
#include <vector>

// Qt
#include <QList>
#include <QSize>
#include <QWidget>

// KDE
#include <KRandomSequence>

// KDEGames
#include <KGameClock>
#include <KgSound>

// LibKMahjongg
#include <kmahjonggbackground.h>
#include <kmahjonggtileset.h>

// KShisen
#include "debug.h"
#include "move.h"
#include "possiblemove.h"

namespace KShisen
{
/**
 * A list of possible moves the player has to choose between
 */
using PossibleMoves = QList<PossibleMove>;

/**
 * @brief Class holding the game board and its functions.
 */
class Board : public QWidget
{
    Q_OBJECT

public:
    explicit Board(QWidget * parent = nullptr);

    /// Number of different kinds of tiles in the game.
    static int const nTiles = 42;

    virtual void paintEvent(QPaintEvent * e);
    virtual void mousePressEvent(QMouseEvent * e);
    virtual void resizeEvent(QResizeEvent * e);

    void setDelay(int);
    int delay() const;

    /// Returns if undo step is available
    bool canUndo() const;
    /// Returns if redo step is available
    bool canRedo() const;
    /// Undoes one step
    void undo();
    /// Redoes one step
    void redo();

    void setSize(int x, int y);
    void resizeBoard();

    void showHint();
    bool hint_I(PossibleMoves & possibleMoves) const;

#ifdef DEBUGGING
    void makeHintMove();
    void finish();
    void dumpBoard() const;
    void dumpBoard(const std::vector<int> & board) const;
#endif

    /// Returns the number of tiles left on the board
    int tilesLeft() const;
    /// Returns the current game time in seconds
    int currentTime() const;

    /// Returns whether the current game is solvable
    bool isSolvable(bool restore); // const?

    bool solvableFlag() const;
    void setSolvableFlag(bool enabled);
    bool gravityFlag() const;
    void setGravityFlag(bool enabled);
    void setChineseStyleFlag(bool enabled);
    void setTilesCanSlideFlag(bool enabled);

    /// Returns possible number of tiles in X direction
    int xTiles() const;
    /// Returns possible number of tiles in Y direction
    int yTiles() const;
    /// Returns overall possible number of tiles in current game board size
    int tiles() const;

    /// Resets the game timer
    void resetTimer();
    /// Resets the undo history
    void resetUndo();
    /// Resets the redo history
    void resetRedo();
    /// Sets whether there are no matching tiles left
    void setGameStuckEnabled(bool enabled);
    /// Sets whether the game is over
    void setGameOverEnabled(bool enabled);
    /// Sets whether the game is in cheat mode
    void setCheatModeEnabled(bool enabled);

    /** Returns whether the game is over.
     * @return True if game is over, False if game is not over
     */
    bool isOver() const;

    /** Returns whether the game is in pause mode.
     * @return True if game is paused, False if game is not paused
     */
    bool isPaused() const;

    /** Returns whether there are still matching tiles left.
     * @return True if there are no matching tiles left, False if there are matching tiles left
     */
    bool isStuck() const;

    /** Returns whether player is in cheat mode.
    * @return True if the player is in cheat mode, False if not
    */
    bool hasCheated() const;

signals:
    void markMatched(); // unused?
    void newGameStarted();
    void changed();
    void tileCountChanged();
    void endOfGame();
    void resized();
    void invalidMove();
    void tilesDoNotMatch();
    void selectATile();
    void selectAMove();
    void selectAMatchingTile();
    void cheatStatusChanged();

public slots:
    /** Does most of the newGame work.
     * This slot is called from the KShisen::invokeNewGame() signal from KShisen and
     * should call KShisen::newGame again to do the work that cannot be done
     * from Board.
     */
    void newGame();

    /// Controls the pause mode
    void setPauseEnabled(bool enabled);

    /** Enables / disables sounds.
     * @param enabled Whether sound shall be enabled
     */
    void setSoundsEnabled(bool enabled);
    /// Loads the game settings
    void loadSettings();
    /// Loads the given tileset
    bool loadTileset(QString const & pathToTileset);
    /// Loads the given background
    bool loadBackground(QString const & pathToBackground);

private slots:
    void undrawConnection();

protected:
    virtual QSize sizeHint() const;

private: // functions
    /** Calculates the board's offset.
     * The board is centred inside the main playing area. xOffset()/yOffset()
     * provide the coordinates of the top-left corner of the board.
     */
    int xOffset() const;
    int yOffset() const;

    /** Returns the line width to use.
     * The line width should be relative to the tile size, however, if the tile size is too small, keep a minimum line width.
     */
    int lineWidth() const;

    void setField(TilePos const & tilePos, int value);
    int field(TilePos const & tilePos) const;
    void updateField(TilePos const & tilePos);
    void showInfoRect(QPainter &, const QString & message);
    void drawTiles(QPainter &, QPaintEvent *);
    void clearHighlight();

    /** Checks if two tiles can match.
     * This is sed for connecting them and for highlighting tiles of the same group.
     */
    bool tilesMatch(int tile1, int tile2) const;

    /** Checks if a path between two tiles can be made with a single line.
     * @param tilePos1 coordinates of the first tile
     * @param tilePos2 coordinates of the second tile
     */
    bool canMakePath(TilePos const & tilePos1, TilePos const & tilePos2) const;

    /** Checks if the tile at \p tilePos1 can be slid to \p tilePos2.
     * @param tilePos1 coordinates of the slide's initial position
     * @param tilePos2 coordinates of the slide's final position
     * @param path The movement of the last tile slided will be stored in the path
     */
    bool canSlideTiles(TilePos const & tilePos1, TilePos const & tilePos2, Path & path) const;

    /** Checks if a path between two tiles can be made with 2 or 3 lines.
    * @param tilePos1 coordinate of the first tile
    * @param tilePos2 coordinate of the second tile
    * @param possibleMoves All the possible moves are stored here
    * @return The number of paths found
    */
    int findPath(TilePos const & tilePos1, TilePos const & tilePos2, PossibleMoves & possibleMoves) const;

    /** Find a path of 1 or 2 segments between tiles.
     * @param tilePos1 coordinates of the first tile
     * @param tilePos2 coordinates of the second tile
     * @param possibleMoves All the possible moves are stored here
     * @return The number of paths found
     */
    int findSimplePath(TilePos const & tilePos1, TilePos const & tilePos2, PossibleMoves & possibleMoves) const;
    void performMove(PossibleMove & possibleMoves);
    void performSlide(TilePos const & tilePos, Path const & slide);
    void reverseSlide(TilePos const & tilePos, int slideX1, int slideY1, int slideX2, int slideY2);
    bool isTileHighlighted(TilePos const & tilePos) const;
    void drawConnection();
    void drawPossibleMoves(bool b);
    /** Calculated the middle coordinates of the given tile position.
     * @param tilePos Tile position
     * @return The middle coordinates of the tile at \p tilePos
     */
    QPoint midCoord(TilePos const & tilePos) const;
    void unmarkTile();
    void marked(TilePos const & tilePos);
    void madeMove(TilePos const & tilePos1, TilePos const & tilePos2, Path slide = Path());

    /** Applies gravity to all columns.
     */
    void applyGravity();

    /** Returns True if @p tilePos is a valid position on Board.
     * @return Wether @p tiePos is valid.
     */
    bool isValidPos(TilePos const & tilePos) const;

    /** Returns True if @p tilePos is a valid position on Board including outline.
     * @return Wether @p tiePos is valid.
     */
    bool isValidPosWithOutline(TilePos const & tilePos) const;

private:
    KGameClock m_gameClock;

    KMahjonggTileset m_tiles;
    KMahjonggBackground m_background;

    KRandomSequence m_random;

    QList<Move *> m_undo; ///< Undo history
    QList<Move *> m_redo; ///< Redo history

    int m_markX;
    int m_markY;
    Path m_connection;
    PossibleMoves m_possibleMoves;
    std::vector<int> m_field; ///< Matrix holding the game board grid
    int m_xTiles;
    int m_yTiles;
    int m_delay;
    int m_level;
    int m_shuffle;

    // The game can be in one of the following states.
    enum class GameState { Normal,
                           Paused,
                           Stuck,
                           Over };
    GameState m_gameState;
    bool m_cheat; ///< Whether the cheat mode is set

    bool m_gravityFlag; ///< Whether game is played with gravity
    bool m_solvableFlag; ///< Whether game is solvable
    bool m_chineseStyleFlag; ///< Whether game follows Chinese rules
    bool m_tilesCanSlideFlag; ///< Whether tiles can slide when connecting

    int m_highlightedTile;

    bool m_paintConnection;
    bool m_paintPossibleMoves;
    bool m_paintInProgress;
    TilePos m_tileRemove1;
    TilePos m_tileRemove2;
    KgSound m_soundPick; ///< Sound object to play when tile is selected
    KgSound m_soundFall; ///< Sound object to play when tiles fall down in gravity mode
};
}

#endif // KSHISEN_BOARD_H

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
