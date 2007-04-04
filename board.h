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

#ifndef __BOARD__H__
#define __BOARD__H__

// Should this get the whole HAVE_SYS_TIME_H TIME_WITH_SYS_TIME treatment?
#include <time.h>

#include <krandomsequence.h>
#include <QList>
#include <QWidget>
#include "kmahjonggtileset.h"
#include "kmahjonggbackground.h"
#include "debug.h"

struct Position
{
	Position() : x(0), y(0) { }
	Position(int _x, int _y) : x(_x), y(_y) { }
	int x;
	int y;
};

typedef QList<Position> Path;

class Move
{
public:
	Move(int _x1, int _y1, int _x2, int _y2, int _tile) :
		x1(_x1), y1(_y1), x2(_x2), y2(_y2), tile1(_tile), tile2(_tile), hasSlide(false), slide_x1(-1), slide_y1(-1), slide_x2(-1), slide_y2(-1) { }
	Move(int _x1, int _y1, int _x2, int _y2, int _tile1, int _tile2) :
		x1(_x1), y1(_y1), x2(_x2), y2(_y2), tile1(_tile1), tile2(_tile2), hasSlide(false), slide_x1(-1), slide_y1(-1), slide_x2(-1), slide_y2(-1) { }
	Move(int _x1, int _y1, int _x2, int _y2, int _tile1, int _tile2, int _slide_x1, int _slide_y1, int _slide_x2, int _slide_y2) :
		x1(_x1), y1(_y1), x2(_x2), y2(_y2), tile1(_tile1), tile2(_tile2), hasSlide(true), slide_x1(_slide_x1), slide_y1(_slide_y1), slide_x2(_slide_x2), slide_y2(_slide_y2) { }

	int x1, y1, x2, y2; // coordinates of the two tiles that matched
	int tile1;  // type of tile at first set of coordinates
	int tile2;  // type of tile at second set of coordinates
	bool hasSlide; // if we performed a slide during the move
	int slide_x1, slide_y1; // original coordinates of the last slided tile
	int slide_x2, slide_y2; // final coordinates of the last slided tile
};

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

	bool canUndo() const;
	bool canRedo() const;
	void redo();
	void undo();

	void setSize(int x, int y);
	void resizeBoard();
	QSize unscaledSize() ;
	void newGame();
	void setShuffle(int);
	int  getShuffle() const;

	void showHint();
	bool getHint_I(Path& p) const;

#ifdef DEBUGGING
	void makeHintMove();
	void finish();
	void dumpBoard() const;
#endif

	int tilesLeft() const;
	int getCurrentTime() const;
	int getTimeForGame() const;

	bool solvable(bool norestore = false);

	bool getSolvableFlag() const;
	void setSolvableFlag(bool);
	bool gravityFlag() const;
	void setGravityFlag(bool);

	int x_tiles() const;
	int y_tiles() const;

	bool isPaused() const { return paused; }

signals:
	void markMatched();
	void changed();
	void endOfGame();
	void resized();

public slots:
	bool pause();
        void loadSettings();
        bool loadTileset    ( const QString & );
        bool loadBackground ( const QString & );

private slots:
	void undrawConnection();
	void gravity(int, bool);

protected:
	virtual QSize sizeHint() const;

private: // functions
	void initBoard();

	int xOffset() ;
	int yOffset() ;

        int lineWidth();

	void setField(int x, int y, int value);
	int getField(int x, int y) const;
	void updateField(int, int);
	void clearHighlight();
        bool tilesMatch(int tile1, int tile2) const;
	bool canMakePath(int x1, int y1, int x2, int y2) const;
	bool canSlideTiles(int x1, int y1, int x2, int y2, Path& p) const;
	// kept for compat for now
	bool findPath(int x1, int y1, int x2, int y2, Path& p) const;
	bool findSimplePath(int x1, int y1, int x2, int y2, Path& p) const;
	// with slide
	bool findPath(int x1, int y1, int x2, int y2, Path& p, Path& s) const;
	bool findSimplePath(int x1, int y1, int x2, int y2, Path& p, Path& s) const;
	void performSlide(int x, int y, Path& s);
	void reverseSlide(int x, int y, int s_x1, int s_y1, int s_x2, int s_y2);
	bool isTileHighlighted(int x, int y) const;
	void drawConnection(int timeout);
	QPoint midCoord(int x, int y);
	void marked(int x, int y);
	void madeMove(int x1, int y1, int x2, int y2);
	void madeMoveWithSlide(int x1, int y1, int x2, int y2, Path& s);

private:
	time_t starttime;
	time_t time_for_game;

	KMahjonggTileset tiles;
        KMahjonggBackground background;

	KRandomSequence random;

	QList<Move*> _undo;
	QList<Move*> _redo;

	int undraw_timer_id;
	int mark_x;
	int mark_y;
	Path connection;
	Path slide;
	int *field;
	int _x_tiles;
	int _y_tiles;
	int _delay;
	int _shuffle;

	bool paused;
	time_t pause_start;

	bool gravity_flag;
	bool _solvable_flag;
        bool _chineseStyle_flag;
        bool _tilesCanSlide_flag;
	int grav_col_1, grav_col_2;

	int highlighted_tile;

	int _connectionTimeout;
	bool _paintConnection;
	QPair<int, int> tileRemove1, tileRemove2;
};

#endif
