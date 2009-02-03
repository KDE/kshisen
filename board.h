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
 * Copyright (C) 1997 by Mario Weilguni <mweilguni@sime.com>
 * Copyright (C) 2002-2004 Dave Corrie  <kde@davecorrie.com>
 * Copyright (c) 2007 Mauricio Piacentini <mauricio@tabuleiro.com>
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

// KMahjonggLib integration and SVG support for KDE 4: Mauricio Piacentini <mauricio@tabuleiro.com>

#ifndef BOARD_H
#define BOARD_H

#include "debug.h"

#include <QList>
#include <QWidget>

#include <kdebug.h>
#include <krandomsequence.h>

#include <kmahjonggtileset.h>
#include <kmahjonggbackground.h>

// Should this get the whole HAVE_SYS_TIME_H TIME_WITH_SYS_TIME treatment?
#include <ctime>

/**
 * @brief Struct holding a position on the board (x,y)
 */
struct Position {
    Position() : x(0), y(0) { }
    Position(int _x, int _y) : x(_x), y(_y) { }
    int x; ///< x position */
    int y; ///< y position */
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
    PossibleMove(Path& p) :
            path(p), hasSlide(false) { }
    PossibleMove(Path& p, Path& s) :
            path(p), hasSlide(true), slide(s) { }

    bool isInPath(int x, int y) const;

    void Debug() const {
        kDebug() << "PossibleMove";
        QList<Position>::const_iterator i;
        for (i = path.begin(); i != path.end(); ++i)
            kDebug() << "    Path:" << i->x << "," << i->y;

        if (hasSlide) {
            kDebug() << "   hasSlide";
            for (i = slide.begin(); i != slide.end(); ++i)
                kDebug() << "    Slide:" << i->x << "," << i->y;
        }
    }

    Path path; ///< path used to connect the two tiles
    bool hasSlide; ///< flag set if the move requires a slide
    Path slide; ///< path representing the movement of the last sliding tile
};

/**
 * A list of possible moves the player has to choose between
 */
typedef QList<PossibleMove> PossibleMoves;


/**
 * @brief Class holding a move on the board made by the player
 *
 * Contains all the information needed to undo or redo the move.
 */
class Move
{
public:
    Move(int _x1, int _y1, int _x2, int _y2, int _tile) :
            x1(_x1), y1(_y1), x2(_x2), y2(_y2), tile1(_tile), tile2(_tile), hasSlide(false), slide_x1(-1), slide_y1(-1), slide_x2(-1), slide_y2(-1) { }
    Move(int _x1, int _y1, int _x2, int _y2, int _tile1, int _tile2) :
            x1(_x1), y1(_y1), x2(_x2), y2(_y2), tile1(_tile1), tile2(_tile2), hasSlide(false), slide_x1(-1), slide_y1(-1), slide_x2(-1), slide_y2(-1) { }
    Move(int _x1, int _y1, int _x2, int _y2, int _tile1, int _tile2, int _slide_x1, int _slide_y1, int _slide_x2, int _slide_y2) :
            x1(_x1), y1(_y1), x2(_x2), y2(_y2), tile1(_tile1), tile2(_tile2), hasSlide(true), slide_x1(_slide_x1), slide_y1(_slide_y1), slide_x2(_slide_x2), slide_y2(_slide_y2) { }

    int x1, y1, x2, y2; ///< coordinates of the two tiles that matched
    int tile1; ///< type of tile at first set of coordinates
    int tile2; ///< type of tile at second set of coordinates
    bool hasSlide; ///< if we performed a slide during the move
    int slide_x1, slide_y1; ///< original coordinates of the last slided tile
    int slide_x2, slide_y2; ///< final coordinates of the last slided tile
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

    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void resizeEvent(QResizeEvent*);

    void setDelay(int);
    int  getDelay() const;

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
    QSize unscaledSize(); // const?
    void newGame();
    void setShuffle(int);
    int  getShuffle() const;

    void showHint();
    bool getHint_I(PossibleMoves& p) const;

#ifdef DEBUGGING
    void makeHintMove();
    void finish();
    void dumpBoard() const;
    void dumpBoard(const int*) const;
#endif

    /// Returns the number of tiles left on the board
    int tilesLeft() const;
    int getCurrentTime() const;
    int getTimeForGame() const;

    bool solvable(bool noRestore = false); // const?

    bool getSolvableFlag() const;
    void setSolvableFlag(bool);
    bool gravityFlag() const;
    void setGravityFlag(bool);
    void setChineseStyleFlag(bool);
    void setTilesCanSlideFlag(bool);

    int xTiles() const;
    int yTiles() const;

    /// Returns whether the game is in pause mode
    bool isPaused() const {
        return m_isPaused;
    }
    /// Resets the game timer
    void resetTimer();
    /// Resets the undo history
    void resetUndo();
    /// Resets the redo history
    void resetRedo();
    void gameOver();
    /// Returns whether the game is over
    bool isOver() const;

signals:
    void markMatched();
    void changed();
    void endOfGame();
    void resized();
    void invalidMove();
    void tilesDontMatch();
    void selectATile();
    void selectAMove();
    void selectAMatchingTile();

public slots:
    bool pause(); // make void and use isPaused() for checking?
    void loadSettings();
    bool loadTileset(const QString &);
    bool loadBackground(const QString &);

private slots:
    void undrawConnection();
    bool gravity(int, bool);

protected:
    virtual QSize sizeHint() const;

private: // functions
    void initBoard();

    int xOffset() const;
    int yOffset() const;

    int lineWidth(); // const?

    void setField(int x, int y, int value);
    int getField(int x, int y) const;
    void updateField(int, int);
    void clearHighlight();
    bool tilesMatch(int tile1, int tile2) const;
    bool canMakePath(int x1, int y1, int x2, int y2) const;
    bool canSlideTiles(int x1, int y1, int x2, int y2, Path& p) const;
    int findPath(int x1, int y1, int x2, int y2, PossibleMoves& p) const;
    int findSimplePath(int x1, int y1, int x2, int y2, PossibleMoves& p) const;
    void performMove(PossibleMove& p);
    void performSlide(int x, int y, Path& s);
    void reverseSlide(int x, int y, int s_x1, int s_y1, int s_x2, int s_y2);
    bool isTileHighlighted(int x, int y) const;
    void drawConnection(int timeout);
    void drawPossibleMoves();
    void undrawPossibleMoves();
    QPoint midCoord(int x, int y); // const?
    void marked(int x, int y);
    void madeMove(int x1, int y1, int x2, int y2);
    void madeMoveWithSlide(int x1, int y1, int x2, int y2, Path& s);
    void gravity(bool);

private:
    time_t m_startTime;
    time_t m_timeForGame;

    KMahjonggTileset m_tiles;
    KMahjonggBackground m_background;

    KRandomSequence m_random;

    QList<Move*> m_undo; ///< Undo history
    QList<Move*> m_redo; ///< Redo history

    // int undraw_timer_id; // not used?
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
    bool m_isOver; ///< Whether game is over
    time_t m_pauseStart;

    bool m_gravityFlag; ///< Whether gravity flag is set
    bool m_solvableFlag; ///< Whether solvable flag is set
    bool m_chineseStyleFlag; ///< Whether Chinese style flag is set
    bool m_tilesCanSlideFlag; ///< Whether tiles can slide flag is set
    QList<int> m_gravCols;
    //int grav_col_1, grav_col_2;

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
