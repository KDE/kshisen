/***************************************************************************
 *   KShisen - A japanese game similar to mahjongg                         *
 *   Copyright 1997  Mario Weilguni <mweilguni@sime.com>                   *
 *   Copyright 2002-2004  Dave Corrie <kde@davecorrie.com>                 *
 *   Copyright 2007  Mauricio Piacentini <mauricio@tabuleiro.com>          *
 *   Copyright 2009-2012  Frederik Schwarzer <schwarzer@kde.org>           *
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

#ifndef BOARD_H
#define BOARD_H

#include "debug.h"

#include <KgSound>

#include <kgameclock.h>
#include <kmahjonggtileset.h>
#include <kmahjonggbackground.h>

#include <krandomsequence.h>

#include "kshisen_debug.h"
#include <QList>
#include <QSize>
#include <QWidget>

#include <vector>


/**
 * @brief Struct holding a position on the board (x,y)
 */
struct Position {
    Position() : x(0), y(0) { }
    Position(int _x, int _y) : x(_x), y(_y) { }
    int x; ///< x position
    int y; ///< y position
};

/**
 * A list of positions (at least 2) makes a Path
 */
typedef QList<Position> Path;

/**
 * @brief Class holding a possible move and its functions
 *
 * A PossibleMove is a connection Path between two tiles
 * and optionally a slide Path.
 * Sometimes for a couple of tiles to match there may be multiple
 * possible moves for the player to choose between.
 */
class PossibleMove
{
public:
    explicit PossibleMove(Path &path) :
        m_path(path), m_hasSlide(false) { }
    PossibleMove(Path &path, Path &slide) :
        m_path(path), m_hasSlide(true), m_slide(slide) { }

    bool isInPath(int x, int y) const;

    void Debug() const {
        qCDebug(KSHISEN_LOG) << "PossibleMove";

        for (auto iter = m_path.constBegin(); iter != m_path.constEnd(); ++iter) {
            qCDebug(KSHISEN_LOG) << "    Path:" << iter->x << "," << iter->y;
        }

        if (m_hasSlide) {
            qCDebug(KSHISEN_LOG) << "   hasSlide";
            for (auto iter = m_slide.constBegin(); iter != m_slide.constEnd(); ++iter) {
                qCDebug(KSHISEN_LOG) << "    Slide:" << iter->x << "," << iter->y;
            }
        }
    }

    Path m_path; ///< path used to connect the two tiles
    bool m_hasSlide; ///< flag set if the move requires a slide
    Path m_slide; ///< path representing the movement of the last sliding tile
};

/**
 * A list of possible moves the player has to choose between
 */
typedef QList<PossibleMove> PossibleMoves;


/**
 * @brief Class holding a move on the board made by the player
 *
 * Contains all the information needed to undo or redo a move.
 */
class Move
{
public:
    Move(int x1, int y1, int x2, int y2, int tile) :
        m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2), m_tile1(tile), m_tile2(tile), m_hasSlide(false), m_slideX1(-1), m_slideY1(-1), m_slideX2(-1), m_slideY2(-1) { }
    Move(int x1, int y1, int x2, int y2, int tile1, int tile2) :
        m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2), m_tile1(tile1), m_tile2(tile2), m_hasSlide(false), m_slideX1(-1), m_slideY1(-1), m_slideX2(-1), m_slideY2(-1) { }
    Move(int x1, int y1, int x2, int y2, int tile1, int tile2, int slideX1, int slideY1, int slideX2, int slideY2) :
        m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2), m_tile1(tile1), m_tile2(tile2), m_hasSlide(true), m_slideX1(slideX1), m_slideY1(slideY1), m_slideX2(slideX2), m_slideY2(slideY2) { }

    int m_x1, m_y1, m_x2, m_y2; ///< coordinates of the two tiles that matched
    int m_tile1; ///< type of tile at first set of coordinates
    int m_tile2; ///< type of tile at second set of coordinates
    bool m_hasSlide; ///< if we performed a slide during the move
    int m_slideX1; ///< original x coordinate of the last slided tile
    int m_slideY1; ///< original y coordinate of the last slided tile
    int m_slideX2; ///< final x coordinate of the last slided tile
    int m_slideY2; ///< final y coordinate of the last slided tile
};


/**
 * @brief Class holding the game board and its functions.
 */
class Board : public QWidget
{
    Q_OBJECT

public:
    Board(QWidget *parent = 0);
    ~Board();

    static const int nTiles = 42;

    virtual void paintEvent(QPaintEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

    void setDelay(int);
    int  delay() const;

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
    bool hint_I(PossibleMoves &possibleMoves) const;

#ifdef DEBUGGING
    void makeHintMove();
    void finish();
    void dumpBoard() const;
    void dumpBoard(std::vector<int>) const;
#endif

    /// Returns the number of tiles left on the board
    int tilesLeft() const;
    /// Returns the current game time in seconds
    int currentTime() const;

    /// Returns whether the current game is solvable
    bool solvable(bool noRestore = false); // const?

    bool solvableFlag() const;
    void setSolvableFlag(bool b);
    bool gravityFlag() const;
    void setGravityFlag(bool b);
    void setChineseStyleFlag(bool b);
    void setTilesCanSlideFlag(bool b);

    int xTiles() const;
    int yTiles() const;

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
    bool loadTileset(const QString &);
    /// Loads the given background
    bool loadBackground(const QString &);

private slots:
    void undrawConnection();

    /** Returns whether the given column is affected by gravity.
     * @param column The column to check
     * @param update FIXME: What is it for?
     */
    bool gravity(int column, bool update);

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

    void setField(int x, int y, int value);
    int field(int x, int y) const;
    void updateField(int, int);
    void showInfoRect(QPainter &, const QString &message);
    void drawTiles(QPainter &, QPaintEvent *);
    void clearHighlight();

    /** Checks if two tiles can match.
     * This is sed for connecting them and for highlighting tiles of the same group.
     */
    bool tilesMatch(int tile1, int tile2) const;

    /** Checks if a path between two tiles can be made with a single line.
     * @param x1 x coordinate of the first tile
     * @param y1 y coordinate of the first tile
     * @param x2 x coordinate of the second tile
     * @param y2 y coordinate of the second tile
     */
    bool canMakePath(int x1, int y1, int x2, int y2) const;

    /** Checks if the tile at (x1,y1) can be slid to (x2,y2).
     * @param x1 x coordinate of the slide's initial position
     * @param y1 y coordinate of the slide's initial position
     * @param x2 x coordinate of the slide's final position
     * @param y2 y coordinate of the slide's final position
     * @param path The movement of the last tile slided will be stored in the path
     */
    bool canSlideTiles(int x1, int y1, int x2, int y2, Path &path) const;

    /** Checks if a path between two tiles can be made with 2 or 3 lines.
    * @param x1 x coordinate of the first tile
    * @param y1 y coordinate of the first tile
    * @param x2 x coordinate of the second tile
    * @param y2 y coordinate of the second tile
    * @param possibleMoves All the possible moves are stored here
    * @return The number of paths found
    */
    int findPath(int x1, int y1, int x2, int y2, PossibleMoves &possibleMoves) const;

    /** Find a path of 1 or 2 segments between tiles.
     * @param x1 x coordinate of the first tile
     * @param y1 y coordinate of the first tile
     * @param x2 x coordinate of the second tile
     * @param y2 y coordinate of the second tile
     * @param possibleMoves All the possible moves are stored here
     * @return The number of paths found
     */
    int findSimplePath(int x1, int y1, int x2, int y2, PossibleMoves &possibleMoves) const;
    void performMove(PossibleMove &possibleMoves);
    void performSlide(int x, int y, Path& s);
    void reverseSlide(int x, int y, int slideX1, int slideY1, int slideX2, int slideY2);
    bool isTileHighlighted(int x, int y) const;
    void drawConnection();
    void drawPossibleMoves(bool b);
    QPoint midCoord(int x, int y) const;
    void unmarkTile();
    void marked(int x, int y);
    void madeMove(int x1, int y1, int x2, int y2, Path slide = Path());

    /** Checks all columns and populate the affected columns in m_gravCols.
     * @param update FIXME: What is it for?
     */
    void gravity(bool update);

private:
    KGameClock m_gameClock;

    KMahjonggTileset m_tiles;
    KMahjonggBackground m_background;

    KRandomSequence m_random;

    QList<Move*> m_undo; ///< Undo history
    QList<Move*> m_redo; ///< Redo history

    int m_markX;
    int m_markY;
    Path m_connection;
    PossibleMoves m_possibleMoves;
    std::vector<int> m_field; ///< Matrix pointer holding the game board grid
    int m_xTiles;
    int m_yTiles;
    int m_delay;
    int m_level;
    int m_shuffle;

    enum class GameState { Normal, Paused, Stuck, Over }; ///< Whether game is paused, has no more matching tiles or is over
    GameState m_gameState;
    bool m_cheat; ///< Whether the cheat mode is set

    bool m_gravityFlag; ///< Whether game is played with gravity
    bool m_solvableFlag; ///< Whether game is solvable
    bool m_chineseStyleFlag; ///< Whether game follows Chinese rules
    bool m_tilesCanSlideFlag; ///< Whether tiles can slide when connecting
    QList<int> m_gravCols;

    int m_highlightedTile;

    bool m_paintConnection;
    bool m_paintPossibleMoves;
    bool m_paintInProgress;
    QPair<int, int> m_tileRemove1;
    QPair<int, int> m_tileRemove2;
    KgSound m_soundPick; ///< Sound object to play when tile is selected
    KgSound m_soundFall; ///< Sound object to play when tiles fall down in gravity mode
};

#endif // BOARD_H

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
