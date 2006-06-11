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
 * created 1997 by Mario Weilguni <mweilguni@sime.com>
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

#ifndef __BOARD__H__
#define __BOARD__H__

// Should this get the whole HAVE_SYS_TIME_H TIME_WITH_SYS_TIME treatment?
#include <time.h>

#include <krandomsequence.h>
#include <list>
#include "tileset.h"
#include "debug.h"

struct Position
{
	Position() : x(0), y(0) { }
	Position(int _x, int _y) : x(_x), y(_y) { }
	int x;
	int y;
};

typedef std::list<Position> Path;

class Move
{
public:
	Move(int _x1, int _y1, int _x2, int _y2, int _tile) :
		x1(_x1), y1(_y1), x2(_x2), y2(_y2), tile(_tile) { }

	int x1, y1, x2, y2;
	int tile;
};

class Board : public QWidget
{
	Q_OBJECT

public:
	Board(QWidget *parent = 0, const char *name=0);
	~Board();

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
	QSize unscaledSize() const;
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

	bool solvable(bool norestore = FALSE);

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
	
private slots:
	void undrawConnection();
	void gravity(int, bool);

protected:
	virtual QSize sizeHint() const;

private: // functions
	void initBoard();

	int xOffset() const;
	int yOffset() const;

	void setField(int x, int y, int value);
	int getField(int x, int y) const;
	void updateField(int, int, bool erase = true);
	void clearHighlight();
	bool canMakePath(int x1, int y1, int x2, int y2) const;
	bool findPath(int x1, int y1, int x2, int y2, Path& p) const;
	bool findSimplePath(int x1, int y1, int x2, int y2, Path& p) const;
	bool isTileHighlighted(int x, int y) const;
	void drawConnection(int timeout);
	QPoint midCoord(int x, int y) const;
	void marked(int x, int y);
	void madeMove(int x1, int y1, int x2, int y2);

private:
	time_t starttime;
	time_t time_for_game;

	TileSet tiles;

	KRandomSequence random;

	QPtrList<Move> _undo;
	QPtrList<Move> _redo;

	int undraw_timer_id;
	int mark_x;
	int mark_y;
	Path connection;
	int *field;
	int _x_tiles;
	int _y_tiles;
	int _delay;
	int _shuffle;

	bool paused;
	time_t pause_start;

	bool gravity_flag;
	bool _solvable_flag;
	int grav_col_1, grav_col_2;

	int highlighted_tile;
};

#endif
