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
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *******************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <algorithm>

#include <qpainter.h>
#include <qimage.h>
#include <qtimer.h>
#include <qptrlist.h>
#include <qbitarray.h>
#include <qbitmap.h>

#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kglobalsettings.h>
#include <kdebug.h>

#include "board.h"

#ifdef DEBUGGING
#include <unistd.h>
#endif

#define EMPTY		0
#define DEFAULTDELAY	500
#define DEFAULTSHUFFLE	4

Board::Board(QWidget *parent) : QWidget(parent, 0, WResizeNoErase)
{
	paused = false;
	trying = false;
	_solvable_flag = true;
	grav_col_1 = -1;
	grav_col_2 = -1;
	setGravityFlag(false);

	// Randomize
	setShuffle(DEFAULTSHUFFLE);

	random.setSeed(0);
	starttime = time((time_t *)0);

	setDelay(DEFAULTDELAY);
	_redo.setAutoDelete(true);
	_undo.setAutoDelete(true);

	field = 0;
	QPixmap bg(KGlobal::dirs()->findResource("appdata", "kshisen_bgnd.xpm"));
	setBackgroundPixmap(bg);
	connect(this, SIGNAL(fieldClicked(int, int)),
	        this, SLOT(marked(int, int)));
	connect(this, SIGNAL(madeMove(int, int, int, int)),
	        this, SLOT(slotMadeMove(int, int, int, int)));

	setShuffle(0);

	highlighted_tile = -1;
}

Board::~Board()
{
	delete [] field;
}

int Board::x_tiles()
{
	return _x_tiles;
}

int Board::y_tiles()
{
	return _y_tiles;
}

void Board::setField(int x, int y, int value)
{
	if(x < 0 || y < 0 || x >= x_tiles() || y >= y_tiles())
	{
		kdFatal() << "Attempted write to invalid field position "
			"(" << x << ", " << y << ")" << endl;
	}

	field[y * x_tiles() + x] = value;
}

int Board::getField(int x, int y)
{
#ifdef DEBUGGING
	if(x < -1 || y < -1 || x > x_tiles() || y > y_tiles())
	{
		kdFatal() << "Attempted read from invalid field position "
			"(" << x << ", " << y << ")" << endl;
	}
#endif

	if(x < 0 || y < 0 || x >= x_tiles() || y >= y_tiles())
		return EMPTY;

	return field[y * x_tiles() + x];
}

void Board::gravity(int col, bool update)
{
	if(gravity_flag)
	{
		int rptr = y_tiles()-1, wptr = y_tiles()-1;
		while(rptr >= 0)
		{
			if(getField(col, wptr) != EMPTY)
			{
				rptr--;
				wptr--;
			}
			else
			{
				if(getField(col, rptr) != EMPTY)
				{
					setField(col, wptr, getField(col, rptr));
					setField(col, rptr, EMPTY);
					if(update)
					{
						updateField(col, rptr);
						updateField(col, wptr);
					}
					wptr--;
					rptr--;
				}
				else
					rptr--;
			}
		}
	}
}

void Board::mousePressEvent(QMouseEvent *e)
{
	// Calculate field position
	int pos_x = -1;
	if(e->pos().x() >= xOffset())
		pos_x = (e->pos().x() - xOffset()) / tiles.tileWidth();

	int pos_y = -1;
	if(e->pos().y() >= yOffset())
		pos_y = (e->pos().y() - yOffset()) / tiles.tileHeight();

	// Mark tile
	if(e->button() == LeftButton)
	{
		if(highlighted_tile != -1)
		{
			int oldmarkx = mark_x;
			int oldmarky = mark_y;
			mark_x=-1;
			mark_y=-1;

			int old_highlighted = highlighted_tile;
			highlighted_tile = -1;

			for(int i = 0; i < x_tiles(); i++)
				for(int j = 0; j < y_tiles(); j++)
					if(old_highlighted == getField(i, j) && !(i == pos_x && j == pos_y))
						updateField(i, j, false);

			mark_x = oldmarkx;
			mark_y = oldmarky;   // no tile selected
		}

		if(pos_x >= 0 && pos_x < x_tiles() && pos_y >= 0 && pos_y < y_tiles())
			emit fieldClicked(pos_x, pos_y);
	}

	// Assist by lighting all tiles of same type
	if(e->button() == RightButton)
	{
		int old_highlighted = highlighted_tile;
		int field = getField(pos_x,pos_y);
		highlighted_tile = field;

		if(mark_x != -1 && getField(mark_x, mark_y) != highlighted_tile)
		{
			int oldmarkx = mark_x;
			int oldmarky = mark_y;
			mark_x=-1;
			mark_y=-1;
			updateField(oldmarkx, oldmarky, false);
		}

		for(int i = 0; i < x_tiles(); i++)
			for(int j = 0; j < y_tiles(); j++)
			{
				int field_tile = getField(i, j);
				if(field == field_tile )
				{
					mark_x=i;
					mark_y=j;
					updateField(i, j, false);
				}
				else if(old_highlighted == field_tile)
				{
					mark_x=-1;
					mark_y=-1;
					updateField(i, j, false);
				}
			}
		mark_x=-1;
		mark_y=-1;   // no tile selected
	}
}

// The board is centred inside the main playing area. xOffset/yOffset provide
// the coordinates of the top-left corner of the board.
int Board::xOffset()
{
	return (width() - (tiles.tileWidth() * x_tiles())) / 2;
}

int Board::yOffset()
{
	return (height() - (tiles.tileHeight() * y_tiles())) / 2;
}

void Board::setSize(int x, int y)
{
	if(x == x_tiles() && y == y_tiles())
		return;

	if(field != 0)
		delete [] field;

	field = new int[ x * y ];
	_x_tiles = x;
	_y_tiles = y;
	for(int i = 0; i < x; i++)
		for(int j = 0; j < y; j++)
			setField(i, j, EMPTY);

	// set the minimum size of the scalable window
	const double MINIMUM_SCALE = 0.2;
	int w = qRound(tiles.unscaledTileWidth() * MINIMUM_SCALE) * x_tiles();
	int h = qRound(tiles.unscaledTileHeight() * MINIMUM_SCALE) * y_tiles();
	w += tiles.unscaledTileWidth();
	h += tiles.unscaledTileWidth();

	setMinimumSize(w, h);

	resizeBoard();
	newGame();
	emit changed();
}

void Board::resizeEvent(QResizeEvent*)
{
	resizeBoard();
}

void Board::resizeBoard()
{
	// calculate tile size required to fit all tiles in the window
	int w = static_cast<int>( static_cast<double>(width() - tiles.unscaledTileWidth()) / x_tiles() );
	int h = static_cast<int>( static_cast<double>(height() - tiles.unscaledTileWidth()) / y_tiles() );

	const double MAXIMUM_SCALE = 2.0;
	w = std::min(w, static_cast<int>((tiles.unscaledTileWidth() * MAXIMUM_SCALE) + 0.5));
	h = std::min(h, static_cast<int>((tiles.unscaledTileHeight() * MAXIMUM_SCALE) + 0.5));

	tiles.resizeTiles(w, h);
}

void Board::newGame()
{
	int i, x, y, k;

	mark_x = -1;
	mark_y = -1;

	_undo.clear();
	_redo.clear();
	clearHistory();

	// distribute all tiles on board
	int cur_tile = 1;
	for(y = 0; y < y_tiles(); y += 4)
	{
		for(x = 0; x < x_tiles(); ++x)
		{
			for(k = 0; k < 4 && y + k < y_tiles(); k++)
				setField(x, y + k, cur_tile);

			cur_tile++;
			if(cur_tile > TileSet::nTiles)
				cur_tile = 1;
		}
	}

	if(getShuffle() == 0)
	{
		if(!trying)
		{
			update();
			starttime = time((time_t *)0);
			emit changed();
		}
		return;
	}

	// shuffle the field
	int tx = x_tiles();
	int ty = y_tiles();
	for(i = 0; i < x_tiles() * y_tiles() * getShuffle(); i++)
	{
		int x1 = random.getLong(tx);
		int y1 = random.getLong(ty);
		int x2 = random.getLong(tx);
		int y2 = random.getLong(ty);
		int t  = getField(x1, y1);
		setField(x1, y1, getField(x2, y2));
		setField(x2, y2, t);
	}

	// do not make solvable if _solvable_flag is false
	if(!_solvable_flag)
	{
		if(!trying)
		{
			update();
			starttime = time((time_t *)0);
			emit changed();
		}
		return;
	}


	int fsize = x_tiles() * y_tiles() * sizeof(int);
	int *oldfield = new int[x_tiles() * y_tiles()];
	memcpy(oldfield, field, fsize);			// save field
	int *tiles = new int[x_tiles() * y_tiles()];
	int *pos = new int[x_tiles() * y_tiles()];

	while(!solvable(true))
	{
		// generate a list of free tiles and positions
		int num_tiles = 0;
		for(i = 0; i < x_tiles() * y_tiles(); i++)
			if(field[i] != EMPTY)
			{
				pos[num_tiles] = i;
				tiles[num_tiles] = field[i];
				num_tiles++;
			}

		// restore field
		memcpy(field, oldfield, fsize);

		// redistribute unsolved tiles
		while(num_tiles > 0)
		{
			// get a random tile
			int r1 = random.getLong(num_tiles);
			int r2 = random.getLong(num_tiles);
			int tile = tiles[r1];
			int apos = pos[r2];

			// truncate list
			tiles[r1] = tiles[num_tiles-1];
			pos[r2] = pos[num_tiles-1];
			num_tiles--;

			// put this tile on the new position
			field[apos] = tile;
		}

		// remember field
		memcpy(oldfield, field, fsize);
	}


	// restore field
	memcpy(field, oldfield, fsize);
	delete tiles;
	delete pos;
	delete oldfield;

	if(!trying)
	{
		update();
		starttime = time((time_t *)0);
		emit changed();
	}
}

void Board::updateField(int x, int y, bool erase)
{
	if(trying)
		return;

	QRect r(xOffset() + x * tiles.tileWidth(),
	        yOffset() + y * tiles.tileHeight(),
	        tiles.tileWidth(),
	        tiles.tileHeight());
	if(isUpdatesEnabled())
		repaint(r, erase);
}

void Board::paintEvent(QPaintEvent *e)
{

	QRect ur = e->rect();            // rectangle to update
	QPixmap pm(ur.size());           // Pixmap for double-buffering
	pm.fill(this, ur.topLeft());     // fill with widget background
	QPainter p(&pm);
	p.translate(-ur.x(), -ur.y());   // use widget coordinate system

	if(paused)
	{
		p.setFont(KGlobalSettings::largeFont());
		p.drawText(rect(), Qt::AlignCenter, i18n("Game Paused"));
	}
	else
	{
		int w = tiles.tileWidth();
		int h = tiles.tileHeight();
		for(int i = 0; i < x_tiles(); i++)
		{
			for(int j = 0; j < y_tiles(); j++)
			{
				int tile = getField(i, j);
				if(tile == EMPTY)
					continue;

				int xpos = xOffset() + i * w;
				int ypos = yOffset() + j * h;
				QRect r(xpos, ypos, w, h);
				if(e->rect().intersects(r))
				{
					// check if it is a marked piece
					if(tile == highlighted_tile || (i == mark_x && j == mark_y))
						p.drawPixmap(xpos, ypos, tiles.highlightedTile(tile-1));
					else
						p.drawPixmap(xpos, ypos, tiles.tile(tile-1));
				}
			}
		}
	}
	p.end();
	bitBlt( this, ur.topLeft(), &pm );
}

void Board::marked(int x, int y)
{
	// make sure that the last arrow is correctly undrawn
	undrawArrow();

	if(getField(x, y) == EMPTY)
		return;

	if(x == mark_x && y == mark_y)
	{
		// unmark the piece
		mark_x = -1;
		mark_y = -1;
		updateField(x, y, false);
		return;
	}

	if(mark_x == -1)
	{
		mark_x = x;
		mark_y = y;
		updateField(x, y, false);
		return;
	}
	else
	{
		int fld1 = getField(mark_x, mark_y);
		int fld2 = getField(x, y);

		// both field same?
		if(fld1 != fld2)
		{
			emit markError();
			return;
		}

		// trace
		if(findPath(mark_x, mark_y, x, y))
		{
			emit madeMove(mark_x, mark_y, x, y);
			drawArrow(mark_x, mark_y, x, y);
			setField(mark_x, mark_y, EMPTY);
			setField(x, y, EMPTY);
			grav_col_1 = x;
			grav_col_2 = mark_x;
			mark_x = -1;
			mark_y = -1;

			// game is over?
			// Must delay until after tiles fall to make this test
			// See undrawArrow GP.
		}
		else
		{
			clearHistory();
			emit markError();
		}
	}
}

bool Board::canMakePath(int x1, int y1, int x2, int y2)
{
	int i;

	if(x1 == x2)
	{
		for(i = std::min(y1, y2)+1; i < std::max(y1, y2); i++)
			if(getField(x1, i) != EMPTY)
				return false;

		return true;
	}

	if(y1 == y2)
	{
		for(i = std::min(x1, x2)+1; i < std::max(x1, x2); i++)
			if(getField(i, y1) != EMPTY)
				return false;

		return true;
	}

	return false;
}

bool Board::findPath(int x1, int y1, int x2, int y2)
{
	clearHistory();

	if(findSimplePath(x1, y1, x2, y2))
	{
		return true;
	}
	else
	{
		// find 3-way path
		int dx[4] = {1, 0, -1, 0};
		int dy[4] = {0, 1, 0, -1};
		int i;

		for(i = 0; i < 4; i++)
		{
			int newx = x1 + dx[i], newy = y1 + dy[i];
			while(newx >= -1 && newx <= x_tiles() &&
				newy >= -1 && newy <= y_tiles() &&
				getField(newx, newy) == EMPTY)
			{
				if(findSimplePath(newx, newy, x2, y2))
				{
					// make place for history point
					for(int j = 3; j > 0; j--)
						history[j] = history[j-1];

					// insert history point
					history[0].x = x1;
					history[0].y = y1;
					return true;
				}

				newx += dx[i];
				newy += dy[i];
			}
		}

		clearHistory();
		return false;
	}

	return false;
}

bool Board::findSimplePath(int x1, int y1, int x2, int y2)
{
	bool r = false;

	// find direct line
	if(canMakePath(x1, y1, x2, y2))
	{
		history[0].x = x1;
		history[0].y = y1;
		history[1].x = x2;
		history[1].y = y2;
		r = true;
	}
	else
	{
		if(!(x1 == x2 || y1 == y2)) // requires complex path
		{
			if(getField(x2, y1) == EMPTY &&
			        canMakePath(x1, y1, x2, y1) && canMakePath(x2, y1, x2, y2))
			{
				history[0].x = x1;
				history[0].y = y1;
				history[1].x = x2;
				history[1].y = y1;
				history[2].x = x2;
				history[2].y = y2;
				r = true;
			}
			else if(getField(x1, y2) == EMPTY &&
			        canMakePath(x1, y1, x1, y2) && canMakePath(x1, y2, x2, y2))
			{
				history[0].x = x1;
				history[0].y = y1;
				history[1].x = x1;
				history[1].y = y2;
				history[2].x = x2;
				history[2].y = y2;
				r = true;
			}
		}
	}

	return r;
}

void Board::drawArrow(int x1, int y1, int x2, int y2)
{
	if(trying)
		return;

	// find out number of array
	int num = 0;
	while(num < 4 && history[num].x != -2)
		num++;

	// lighten the fields
	// remember mark_x,mark_y
	int mx = mark_x, my = mark_y;
	mark_x = x1;
	mark_y = y1;
	updateField(x1, y1);
	mark_x = x2;
	mark_y = y2;
	updateField(x2, y2);

	// restore the mark
	mark_x = mx;
	mark_y = my;

	QPainter p;
	p.begin(this);
	p.setPen(QPen(QColor("red"), tiles.lineWidth()));
	num = 0;
	while(num < 3 && history[num+1].x != -2)
	{
		p.drawLine(midCoord(history[num].x, history[num].y),
		           midCoord(history[num+1].x, history[num+1].y));
		num++;
	}
	p.flush();
	p.end();

	QTimer::singleShot(getDelay(), this, SLOT(undrawArrow()));
}

void Board::undrawArrow()
{
	if(trying)
		return;

	if(grav_col_1 != -1 || grav_col_2 != -1)
	{
		gravity(grav_col_1, true);
		gravity(grav_col_2, true);
		grav_col_1 = -1;
		grav_col_2 = -1;
	}

	// is already undrawn?
	if(history[0].x == -2)
		return;

	// redraw all affected fields
	int num = 0;
	while(num < 3 && history[num+1].x != -2)
	{
		if(history[num].y == history[num+1].y)
		{
			for(int i = std::min(history[num].x, history[num+1].x);
			        i <= std::max(history[num].x, history[num+1].x); i++)
				updateField(i, history[num].y);
		}
		else
		{
			for(int i = std::min(history[num].y, history[num+1].y);
			        i <= std::max(history[num].y, history[num+1].y); i++)
				updateField(history[num].x, i);
		}
		num++;
	}

	clearHistory();

	int dummyx;
	History dummyh[4];
	// game is over?
	if(!getHint_I(dummyx,dummyx,dummyx,dummyx,dummyh))
	{
		time_for_game = (int)difftime( time((time_t)0), starttime);
		emit endOfGame();
	}
}

QPoint Board::midCoord(int x, int y)
{
	QPoint p;
	int w = tiles.tileWidth();
	int h = tiles.tileHeight();

	if(x == -1)
		p.setX(xOffset() - (w / 4));
	else if(x == x_tiles())
		p.setX(xOffset() + (w * x_tiles()) + (w / 4));
	else
		p.setX(xOffset() + (w * x) + (w / 2));

	if(y == -1)
		p.setY(yOffset() - (w / 4));
	else if(y == y_tiles())
		p.setY(yOffset() + (h * y_tiles()) + (w / 4));
	else
		p.setY(yOffset() + (h * y) + (h / 2));

	return p;
}

void Board::setDelay(int newvalue)
{
	_delay = newvalue;
}

int Board::getDelay()
{
	return _delay;
}

void Board::slotMadeMove(int x1, int y1, int x2, int y2)
{
	Move *m = new Move(x1, y1, x2, y2, getField(x1, y1));
	_undo.append(m);
	while(_redo.count())
		_redo.removeFirst();
	emit changed();
}

bool Board::canUndo()
{
	return (bool)(_undo.count() > 0);
}

bool Board::canRedo()
{
	return (bool)(_redo.count() > 0);
}

void Board::undo()
{
	if(canUndo())
	{
		undrawArrow();
		Move *m = _undo.take(_undo.count() - 1);
		if(gravityFlag())
		{
			int y;

			// When both tiles reside in the same column, the order of undo is
			// significant (we must undo the lower tile first).
			if(m->x1 == m->x2 && m->y1 < m->y2)
			{
				std::swap(m->x1, m->x2);
				std::swap(m->y1, m->y2);
			}

			for(y = 0; y < m->y1; y++)
			{
				setField(m->x1, y, getField(m->x1, y+1));
				updateField(m->x1, y);
			}

			for(y = 0; y < m->y2; y++)
			{
				setField(m->x2, y, getField(m->x2, y+1));
				updateField(m->x2, y);
			}
		}

		setField(m->x1, m->y1, m->tile);
		setField(m->x2, m->y2, m->tile);
		updateField(m->x1, m->y1);
		updateField(m->x2, m->y2);
		_redo.insert(0, m);
		emit changed();
	}
}

void Board::redo()
{
	if(canRedo())
	{
		undrawArrow();
		Move *m = _redo.take(0);
		setField(m->x1, m->y1, EMPTY);
		setField(m->x2, m->y2, EMPTY);
		updateField(m->x1, m->y1);
		updateField(m->x2, m->y2);
		gravity(m->x1, true);
		gravity(m->x2, true);
		_undo.append(m);
		emit changed();
	}
}

void Board::clearHistory()
{
	// init history
	for(int i = 0; i < 4; i++)
	{
		history[i].x = -2;
		history[i].y = -2;
	}
}

void Board::getHint()
{
	int x1, y1, x2, y2;
	History h[4];

	if(getHint_I(x1, y1, x2, y2, h))
	{
		undrawArrow();
		for(int i = 0; i < 4; i++)
			history[i] = h[i];

		int old_delay = getDelay();
		setDelay(1000);
		drawArrow(x1, y1, x2, y2);
		setDelay(old_delay);
	}
}


#ifdef DEBUGGING
void Board::makeHintMove()
{
	int x1, y1, x2, y2;
	History h[4];

	if(getHint_I(x1, y1, x2, y2, h))
	{
		mark_x = -1;
		mark_y = -1;
		marked(x1, y1);
		marked(x2, y2);
	}
}

void Board::finish()
{
	int x1, y1, x2, y2;
	History h[4];
	bool ready=false;

	while(!ready && getHint_I(x1, y1, x2, y2, h))
	{
		mark_x = -1;
		mark_y = -1;
		if(tilesLeft() == 2)
			ready = true;
		marked(x1, y1);
		marked(x2, y2);
		kapp->processEvents();
		usleep(250*1000);
	}
}
#endif

bool Board::getHint_I(int &x1, int &y1, int &x2, int &y2, History h[4])
{
	short done[TileSet::nTiles];
	for( short index = 0; index < TileSet::nTiles; index++ )
		done[index] = 0;

	// remember old history
	History old[4];
	for(int i = 0; i < 4; i++)
		old[i] = history[i];

	// initial no hint
	x1 = -1;
	x2 = -1;
	y1 = -1;
	y2 = -1;

	for(int x = 0; x < x_tiles(); x++)
	{
		for(int y = 0; y < y_tiles(); y++)
		{
			int tile = getField(x, y);
			if(tile != EMPTY && done[tile - 1] != 4)
			{
				// for all these types of tile search path's
				for(int xx = 0; xx < x_tiles(); xx++)
				{
					for(int yy = 0; yy < y_tiles(); yy++)
					{
						if(xx != x || yy != y)
						{
							if(getField(xx, yy) == tile)
							{
								if(findPath(x, y, xx, yy))
								{
									for(int i = 0; i < 4; i++)
										h[i] = history[i];

									x1 = x;
									x2 = xx;
									y1 = y;
									y2 = yy;
									for(int i = 0; i < 4; i++)
										history[i] = old[i];
									return true;
								}
							}
						}
					}
				}

				clearHistory();
				done[tile - 1]++;
			}
		}
	}

	for(int i = 0; i < 4; i++)
		history[i] = old[i];

	return false;
}

void Board::setShuffle(int newvalue)
{
	_shuffle = newvalue;
}

int Board::getShuffle()
{
	return _shuffle;
}

int Board::tilesLeft()
{
	int left = 0;

	for(int i = 0; i < x_tiles(); i++)
		for(int j = 0; j < y_tiles(); j++)
			if(getField(i, j) != EMPTY)
				left++;

	return left;
}

int Board::getCurrentTime()
{
	return (int)difftime(time((time_t *)0),starttime);
}

int Board::getTimeForGame()
{
	if(tilesLeft() == 0)
	{
		return time_for_game;
	}
	else
	{
		if(paused)
			return (int)difftime(pause_start, starttime);
		else
			return (int)difftime(time((time_t *)0), starttime);
	}
}

bool Board::solvable(bool norestore)
{
	int x1, y1, x2, y2;
	History h[4];
	int *oldfield = 0;

	if(!norestore)
	{
		oldfield = new int [x_tiles() * y_tiles()];
		memcpy(oldfield, field, x_tiles() * y_tiles() * sizeof(int));
	}

	while(getHint_I(x1, y1, x2, y2, h))
	{
		setField(x1, y1, EMPTY);
		setField(x2, y2, EMPTY);
		//if(gravityFlag())
		//{
		//	gravity(x1, false);
		//	gravity(x2, false);
		//}
	}

	int left = tilesLeft();

	if(!norestore)
	{
		memcpy(field, oldfield, x_tiles() * y_tiles() * sizeof(int));
		delete [] oldfield;
	}

	return (bool)(left == 0);
}

bool Board::getSolvableFlag()
{
	return _solvable_flag;
}

void Board::setSolvableFlag(bool value)
{
	_solvable_flag = value;
}

bool Board::gravityFlag()
{
	return gravity_flag;
}

void Board::setGravityFlag(bool b)
{
	gravity_flag = b;
}

bool Board::pause()
{
	paused = !paused;
	if(paused)
		pause_start = time((time_t *)0);
	else
		starttime += (time_t) difftime( time((time_t *)0), pause_start);
	update();

	return paused;
}

#include "board.moc"
