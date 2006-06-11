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

#include <kapplication.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <kdebug.h>

#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qtimer.h>

#include "board.h"
#include "prefs.h"

#define EMPTY		0
#define DEFAULTDELAY	500
#define DEFAULTSHUFFLE	4

static int size_x[5] = {14, 18, 24, 26, 30};
static int size_y[5] = { 6,  8, 12, 14, 16};
static int DELAY[5] = {1000, 750, 500, 250, 125};

Board::Board(QWidget *parent, const char *name) : 
       QWidget(parent, name, WResizeNoErase), field(0),
       _x_tiles(0), _y_tiles(0),
       _delay(125), paused(false),
       gravity_flag(true), _solvable_flag(true),
	     grav_col_1(-1), grav_col_2(-1), highlighted_tile(-1)
{
	// Randomize
	setShuffle(DEFAULTSHUFFLE);

	random.setSeed(0);
	starttime = time((time_t *)0);

	setDelay(DEFAULTDELAY);
	_redo.setAutoDelete(true);
	_undo.setAutoDelete(true);

	QPixmap bg(KGlobal::dirs()->findResource("appdata", "kshisen_bgnd.png"));
	setBackgroundPixmap(bg);

	loadSettings();
}

Board::~Board()
{
	delete [] field;
}

void Board::loadSettings(){
	int index = Prefs::size();
	setSize(size_x[index], size_y[index]);

	setShuffle(Prefs::level() * 4 + 1);
	setGravityFlag(Prefs::gravity());
	setSolvableFlag(Prefs::solvable());
	setDelay(DELAY[Prefs::speed()]);
}

int Board::x_tiles() const
{
	return _x_tiles;
}

int Board::y_tiles() const
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

int Board::getField(int x, int y) const
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
	int pos_x = (e->pos().x() - xOffset()) / tiles.tileWidth();
	int pos_y = (e->pos().y() - yOffset()) / tiles.tileHeight();

	if(e->pos().x() < xOffset() || e->pos().y() < yOffset() ||
		pos_x >= x_tiles() || pos_y >= y_tiles())
	{
		pos_x = -1;
		pos_y = -1;
	}

	// Mark tile
	if(e->button() == LeftButton)
	{
		clearHighlight();

		if(pos_x != -1)
			marked(pos_x, pos_y);
	}

	// Assist by highlighting all tiles of same type
	if(e->button() == RightButton)
	{
		int clicked_tile = getField(pos_x, pos_y);

		// Clear marked tile
		if(mark_x != -1 && getField(mark_x, mark_y) != clicked_tile)
		{
			// We need to set mark_x and mark_y to -1 before calling
			// updateField() to ensure the tile is redrawn as unmarked.
			int oldmarkx = mark_x;
			int oldmarky = mark_y;
			mark_x = -1;
			mark_y = -1;
			updateField(oldmarkx, oldmarky, false);
		}
		else
		{
			mark_x = -1;
			mark_y = -1;
		}

		// Perform highlighting
		if(clicked_tile != highlighted_tile)
		{
			int old_highlighted = highlighted_tile;
			highlighted_tile = clicked_tile;
			for(int i = 0; i < x_tiles(); i++)
			{
				for(int j = 0; j < y_tiles(); j++)
				{
					const int field_tile = getField(i, j);
					if(field_tile != EMPTY)
					{
						if(field_tile == old_highlighted)
							updateField(i, j, false);
						else if(field_tile == clicked_tile)
							updateField(i, j, false);
					}
				}
			}
		}
	}
}

// The board is centred inside the main playing area. xOffset/yOffset provide
// the coordinates of the top-left corner of the board.
int Board::xOffset() const
{
	return (width() - (tiles.tileWidth() * x_tiles())) / 2;
}

int Board::yOffset() const
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
	emit resized();
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

QSize Board::unscaledSize() const
{
	int w = tiles.unscaledTileWidth() * x_tiles() + tiles.unscaledTileWidth();
	int h = tiles.unscaledTileHeight() * y_tiles() + tiles.unscaledTileWidth();
	return QSize(w, h);
}

void Board::newGame()
{
	//kdDebug() << "NewGame" << endl;
	int i, x, y, k;

	mark_x = -1;
	mark_y = -1;
	highlighted_tile = -1; // will clear previous highlight

	_undo.clear();
	_redo.clear();
	connection.clear();

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
		update();
		starttime = time((time_t *)0);
		emit changed();
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
		update();
		starttime = time((time_t *)0);
		emit changed();
		return;
	}


	int fsize = x_tiles() * y_tiles() * sizeof(int);
	int *oldfield = new int[x_tiles() * y_tiles()];
	memcpy(oldfield, field, fsize);			// save field
	int *tiles = new int[x_tiles() * y_tiles()];
	int *pos = new int[x_tiles() * y_tiles()];

	while(!solvable(true))
	{
		//kdDebug() << "Not solvable" << endl;
		//dumpBoard();

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

	update();
	starttime = time((time_t *)0);
	emit changed();
}

bool Board::isTileHighlighted(int x, int y) const
{
	if(x == mark_x && y == mark_y)
		return true;

	if(getField(x, y) == highlighted_tile)
		return true;

	if(!connection.empty())
	{
		if(x == connection.front().x && y == connection.front().y)
			return true;

		if(x == connection.back().x && y == connection.back().y)
			return true;
	}

	return false;
}

void Board::updateField(int x, int y, bool erase)
{
	QRect r(xOffset() + x * tiles.tileWidth(),
	        yOffset() + y * tiles.tileHeight(),
	        tiles.tileWidth(),
	        tiles.tileHeight());

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
					if(isTileHighlighted(i, j))
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
	// make sure that the previous connection is correctly undrawn
	undrawConnection();

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

	int fld1 = getField(mark_x, mark_y);
	int fld2 = getField(x, y);

	// both field same?
	if(fld1 != fld2)
		return;

	// trace
	if(findPath(mark_x, mark_y, x, y, connection))
	{
		madeMove(mark_x, mark_y, x, y);
		drawConnection(getDelay());
		setField(mark_x, mark_y, EMPTY);
		setField(x, y, EMPTY);
		grav_col_1 = x;
		grav_col_2 = mark_x;
		mark_x = -1;
		mark_y = -1;

		// game is over?
		// Must delay until after tiles fall to make this test
		// See undrawConnection GP.
	}
	else
	{
		connection.clear();
	}
}


void Board::clearHighlight()
{
	if(highlighted_tile != -1)
	{
		int old_highlight = highlighted_tile;
		highlighted_tile = -1;

		for(int i = 0; i < x_tiles(); i++)
			for(int j = 0; j < y_tiles(); j++)
				if(old_highlight == getField(i, j))
					updateField(i, j, false);
	}
}

// Can we make a path between two tiles with a single line?
bool Board::canMakePath(int x1, int y1, int x2, int y2) const
{
	if(x1 == x2)
	{
		for(int i = std::min(y1, y2) + 1; i < std::max(y1, y2); i++)
			if(getField(x1, i) != EMPTY)
				return false;

		return true;
	}

	if(y1 == y2)
	{
		for(int i = std::min(x1, x2) + 1; i < std::max(x1, x2); i++)
			if(getField(i, y1) != EMPTY)
				return false;

		return true;
	}

	return false;
}

bool Board::findPath(int x1, int y1, int x2, int y2, Path& p) const
{
	p.clear();

	if(findSimplePath(x1, y1, x2, y2, p))
		return true;

	// Find a path of 3 segments
	const int dx[4] = { 1, 0, -1, 0 };
	const int dy[4] = { 0, 1, 0, -1 };

	for(int i = 0; i < 4; i++)
	{
		int newx = x1 + dx[i];
		int newy = y1 + dy[i];
		while(newx >= -1 && newx <= x_tiles() &&
			newy >= -1 && newy <= y_tiles() &&
			getField(newx, newy) == EMPTY)
		{
			if(findSimplePath(newx, newy, x2, y2, p))
			{
				p.push_front(Position(x1, y1));
				return true;
			}
			newx += dx[i];
			newy += dy[i];
		}
	}

	return false;
}

// Find a path of 1 or 2 segments between tiles. Returns whether
// a path was found, and if so, the path is returned via 'p'.
bool Board::findSimplePath(int x1, int y1, int x2, int y2, Path& p) const
{
	// Find direct line (path of 1 segment)
	if(canMakePath(x1, y1, x2, y2))
	{
		p.push_back(Position(x1, y1));
		p.push_back(Position(x2, y2));
		return true;
	}

	// If the tiles are in the same row or column, then a
	// a 'simple path' cannot be found between them
	if(x1 == x2 || y1 == y2)
		return false;

	// Find path of 2 segments (route A)
	if(getField(x2, y1) == EMPTY && canMakePath(x1, y1, x2, y1) &&
		canMakePath(x2, y1, x2, y2))
	{
		p.push_back(Position(x1, y1));
		p.push_back(Position(x2, y1));
		p.push_back(Position(x2, y2));
		return true;
	}

	// Find path of 2 segments (route B)
	if(getField(x1, y2) == EMPTY && canMakePath(x1, y1, x1, y2) &&
		canMakePath(x1, y2, x2, y2))
	{
		p.push_back(Position(x1, y1));
		p.push_back(Position(x1, y2));
		p.push_back(Position(x2, y2));
		return true;
	}

	return false;
}

void Board::drawConnection(int timeout)
{
	if(connection.empty())
		return;

	// lighten the fields
	updateField(connection.front().x, connection.front().y);
	updateField(connection.back().x, connection.back().y);

	QPainter p;
	p.begin(this);
	p.setPen(QPen(QColor("red"), tiles.lineWidth()));

	// Path.size() will always be >= 2
	Path::const_iterator pathEnd = connection.end();
	Path::const_iterator pt1 = connection.begin();
	Path::const_iterator pt2 = pt1;
	++pt2;
	while(pt2 != pathEnd)
	{
		p.drawLine( midCoord(pt1->x, pt1->y), midCoord(pt2->x, pt2->y) );
		++pt1;
		++pt2;
	}

	p.flush();
	p.end();

	QTimer::singleShot(timeout, this, SLOT(undrawConnection()));
}

void Board::undrawConnection()
{
	if(grav_col_1 != -1 || grav_col_2 != -1)
	{
		gravity(grav_col_1, true);
		gravity(grav_col_2, true);
		grav_col_1 = -1;
		grav_col_2 = -1;
	}

	// is already undrawn?
	if(connection.empty())
		return;

	// Redraw all affected fields

	Path oldConnection = connection;
	connection.clear();

	// Path.size() will always be >= 2
	Path::const_iterator pathEnd = oldConnection.end();
	Path::const_iterator pt1 = oldConnection.begin();
	Path::const_iterator pt2 = pt1;
	++pt2;
	while(pt2 != pathEnd)
	{
		if(pt1->y == pt2->y)
		{
			for(int i = std::min(pt1->x, pt2->x); i <= std::max(pt1->x, pt2->x); i++)
				updateField(i, pt1->y);
		}
		else
		{
			for(int i = std::min(pt1->y, pt2->y); i <= std::max(pt1->y, pt2->y); i++)
				updateField(pt1->x, i);
		}
		++pt1;
		++pt2;
	}

	Path dummyPath;
	// game is over?
	if(!getHint_I(dummyPath))
	{
		time_for_game = (int)difftime( time((time_t)0), starttime);
		emit endOfGame();
	}
}

QPoint Board::midCoord(int x, int y) const
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

int Board::getDelay() const
{
	return _delay;
}

void Board::madeMove(int x1, int y1, int x2, int y2)
{
	Move *m = new Move(x1, y1, x2, y2, getField(x1, y1));
	_undo.append(m);
	while(_redo.count())
		_redo.removeFirst();
	emit changed();
}

bool Board::canUndo() const
{
	return !_undo.isEmpty();
}

bool Board::canRedo() const
{
	return !_redo.isEmpty();
}

void Board::undo()
{
	if(canUndo())
	{
		clearHighlight();
		undrawConnection();
		Move* m = _undo.last();
		_undo.take();
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
		_redo.prepend(m);
		emit changed();
	}
}

void Board::redo()
{
	if(canRedo())
	{
		clearHighlight();
		undrawConnection();
		Move* m = _redo.take(0);
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

void Board::showHint()
{
	undrawConnection();

	if(getHint_I(connection))
		drawConnection(1000);
}


#ifdef DEBUGGING
void Board::makeHintMove()
{
	Path p;

	if(getHint_I(p))
	{
		mark_x = -1;
		mark_y = -1;
		marked(p.front().x, p.front().y);
		marked(p.back().x, p.back().y);
	}
}

void Board::finish()
{
	Path p;
	bool ready=false;

	while(!ready && getHint_I(p))
	{
		mark_x = -1;
		mark_y = -1;
		if(tilesLeft() == 2)
			ready = true;
		marked(p.front().x, p.front().y);
		marked(p.back().x, p.back().y);
		kapp->processEvents();
		usleep(250*1000);
	}
}

void Board::dumpBoard() const
{
	kdDebug() << "Board contents:" << endl;
	for(int y = 0; y < y_tiles(); ++y)
	{
		QString row;
		for(int x = 0; x < x_tiles(); ++x)
		{
			int tile = getField(x, y);
			if(tile == EMPTY)
				row += " --";
			else
				row += QString("%1").arg(getField(x, y), 3);
		}
		kdDebug() << row << endl;
	}
}
#endif

bool Board::getHint_I(Path& p) const
{
	//dumpBoard();
	short done[TileSet::nTiles];
	for( short index = 0; index < TileSet::nTiles; index++ )
		done[index] = 0;

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
								if(findPath(x, y, xx, yy, p))
								{
									//kdDebug() << "path.size() == " << p.size() << endl;
									//for(Path::const_iterator i = p.begin(); i != p.end(); ++i)
									//	kdDebug() << "pathEntry: (" << i->x << ", " << i->y
									//		<< ") => " << getField(i->x, i->y) << endl;
									return true;
								}
						}
					}
				}
				done[tile - 1]++;
			}
		}
	}

	return false;
}

void Board::setShuffle(int newvalue)
{
	if(newvalue != _shuffle){
		_shuffle = newvalue;
		newGame();
	}
}

int Board::getShuffle() const
{
	return _shuffle;
}

int Board::tilesLeft() const
{
	int left = 0;

	for(int i = 0; i < x_tiles(); i++)
		for(int j = 0; j < y_tiles(); j++)
			if(getField(i, j) != EMPTY)
				left++;

	return left;
}

int Board::getCurrentTime() const
{
	return (int)difftime(time((time_t *)0),starttime);
}

int Board::getTimeForGame() const
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
	int *oldfield = 0;

	if(!norestore)
	{
		oldfield = new int [x_tiles() * y_tiles()];
		memcpy(oldfield, field, x_tiles() * y_tiles() * sizeof(int));
	}

	Path p;
	while(getHint_I(p))
	{
		kdFatal(getField(p.front().x, p.front().y) != getField(p.back().x, p.back().y))
			<< "Removing unmateched tiles: (" << p.front().x << ", " << p.front().y << ") => "
			<< getField(p.front().x, p.front().y) << " (" << p.back().x << ", " << p.back().y << ") => "
            << getField(p.back().x, p.back().y) << endl;
		setField(p.front().x, p.front().y, EMPTY);
		setField(p.back().x, p.back().y, EMPTY);
		//if(gravityFlag())
		//{
		//	gravity(p.front().x, false);
		//	gravity(p.back().x, false);
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

bool Board::getSolvableFlag() const
{
	return _solvable_flag;
}

void Board::setSolvableFlag(bool value)
{
	if(value && !_solvable_flag && !solvable()){
		_solvable_flag = value;
		newGame(); 
	}
	else 
		_solvable_flag = value;
}

bool Board::gravityFlag() const
{
	return gravity_flag;
}

void Board::setGravityFlag(bool b)
{
	if( gravity_flag != b ){
		if(canUndo() || canRedo())
			newGame();
		gravity_flag = b;
	}
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

QSize Board::sizeHint() const
{
	int dpi = QPaintDeviceMetrics(this).logicalDpiX();
	if (dpi < 75)
	   dpi = 75;
	return QSize(9*dpi,7*dpi);
}

#include "board.moc"
