/***************************************************************************
 *   KShisen - A japanese game similar to mahjongg                         *
 *   Copyright 1997  Mario Weilguni <mweilguni@sime.com>                   *
 *   Copyright 2002-2004  Dave Corrie <kde@davecorrie.com>                 *
 *   Copyright 2007  Mauricio Piacentini <mauricio@tabuleiro.com>          *
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

// KMahjonggLib integration and SVG support for KDE 4: Mauricio Piacentini <mauricio@tabuleiro.com>

#ifndef BOARD_H
#define BOARD_H

#include "debug.h"

#include <kgameclock.h>
#include <kmahjonggtileset.h>
#include <kmahjonggbackground.h>

#include <kdebug.h>
#include <krandomsequence.h>

#include <QList>
#include <QSize>
#include <QWidget>

static int sizeX[6] = {14, 16, 18, 24, 26, 30};
static int sizeY[6] = { 6,  9,  8, 12, 14, 16};

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
    PossibleMove(Path &path) :
            m_path(path), m_hasSlide(false) { }
    PossibleMove(Path &path, Path &slide) :
            m_path(path), m_hasSlide(true), m_slide(slide) { }

    bool isInPath(int x, int y) const;

    void Debug() const {
        kDebug() << "PossibleMove";
        QList<Position>::const_iterator iter;
        for (iter = m_path.constBegin(); iter != m_path.constEnd(); ++iter) {
            kDebug() << "    Path:" << iter->x << "," << iter->y;
        }

        if (m_hasSlide) {
            kDebug() << "   hasSlide";
            for (iter = m_slide.constBegin(); iter != m_slide.constEnd(); ++iter) {
                kDebug() << "    Slide:" << iter->x << "," << iter->y;
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
    void newGame();
    void setShuffle(int);
    int  shuffle() const;

    void showHint();
    bool hint_I(PossibleMoves &possibleMoves) const;

#ifdef DEBUGGING
    void makeHintMove();
    void finish();
    void dumpBoard() const;
    void dumpBoard(const int *) const;
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
    /// Returns whether the game is over
    bool isOver() const;
    /// Returns whether the game is in pause mode
    bool isPaused() const;
    /// Returns whether there are still matching tiles left
    bool isStuck() const;
    /// Returns whether player is in cheat mode
    bool hasCheated() const;

signals:
    void markMatched(); // unused?
    void changed();
    void endOfGame();
    void resized();
    void invalidMove();
    void tilesDontMatch();
    void selectATile();
    void selectAMove();
    void selectAMatchingTile();
    void cheatStatusChanged();

public slots:
    /// Controls the pause mode
    void setPauseEnabled(bool enabled);
    /// Loads the game settings
    void loadSettings();
    /// Loads the given tileset
    bool loadTileset(const QString &);
    /// Loads the given background
    bool loadBackground(const QString &);

private slots:
    void undrawConnection();
    /// Returns whether the given column is affected by gravity
    bool gravity(int column, bool update);

protected:
    virtual QSize sizeHint() const;

private: // functions
    int xOffset() const;
    int yOffset() const;

    /// Returns the line width to use
    int lineWidth() const;

    void setField(int x, int y, int value);
    int field(int x, int y) const;
    void updateField(int, int);
    void clearHighlight();
    /// Checks if two tiles can match
    bool tilesMatch(int tile1, int tile2) const;
    /// Checks if a path between two tiles can be made with a single line
    bool canMakePath(int x1, int y1, int x2, int y2) const;
    /// Checks if the tile at (x1,y1) can be slid to (x2,y2)
    bool canSlideTiles(int x1, int y1, int x2, int y2, Path &path) const;
    /// Checks if a path between two tiles can be made with 2 or 3 lines
    int findPath(int x1, int y1, int x2, int y2, PossibleMoves &possibleMoves) const;
    /// Find a path of 1 or 2 segments between tiles.
    int findSimplePath(int x1, int y1, int x2, int y2, PossibleMoves &possibleMoves) const;
    void performMove(PossibleMove &possibleMoves);
    void performSlide(int x, int y, Path& s);
    void reverseSlide(int x, int y, int slideX1, int slideY1, int slideX2, int slideY2);
    bool isTileHighlighted(int x, int y) const;
    void drawConnection(int timeout);
    void drawPossibleMoves(bool b);
    QPoint midCoord(int x, int y) const;
    void marked(int x, int y);
    void madeMove(int x1, int y1, int x2, int y2);
    void madeMoveWithSlide(int x1, int y1, int x2, int y2, Path &slide);
    /// Checks all columns and populate the affected columns in m_gravCols
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
    int *m_field; ///< Matrix pointer holding the game board grid
    int m_xTiles;
    int m_yTiles;
    int m_delay;
    int m_shuffle;

    bool m_isPaused; ///< Whether game is paused
    bool m_isStuck; ///< Whether game has no more matching tiles
    bool m_isOver; ///< Whether game is over
    bool m_cheat; ///< Whether the cheat mode is set

    bool m_gravityFlag; ///< Whether gravity flag is set
    bool m_solvableFlag; ///< Whether solvable flag is set
    bool m_chineseStyleFlag; ///< Whether Chinese style flag is set
    bool m_tilesCanSlideFlag; ///< Whether tiles can slide flag is set
    QList<int> m_gravCols;

    int m_highlightedTile;

    int m_connectionTimeout;
    bool m_paintConnection;
    bool m_paintPossibleMoves;
    QPair<int, int> m_tileRemove1;
    QPair<int, int> m_tileRemove2;
};

#endif // BOARD_H

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
