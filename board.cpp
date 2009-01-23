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
 * Copyright (C) 2007 Mauricio Piacentini <mauricio@tabuleiro.com>
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

#define USE_UPDATE 1
#include "board.h"

#include <klocale.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>

#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include "prefs.h"

#define EMPTY           0
#define DEFAULTDELAY    500
#define DEFAULTSHUFFLE  4
#define SEASONS_START   28
#define FLOWERS_START   39

static int size_x[6] = {14, 16, 18, 24, 26, 30};
static int size_y[6] = { 6, 9, 8, 12, 14, 16};
static int DELAY[5] = {1000, 750, 500, 250, 125};

bool PossibleMove::isInPath(int x, int y) const
{
    if (x == path.last().x && y == path.last().y) {
        return false;
    }
    kDebug() << "isInPath:" << x << "," << y;
    Debug();
    QList<Position>::const_iterator j;
    // a path has at least 2 positions
    j = path.begin();
    int path_x = j->x;
    int path_y = j->y;
    ++j;
    for (; j != path.end(); ++j) {
        // to fix
        if ((x == j->x && ((y > path_y && y <= j->y) || (y < path_y && y >= j->y)))
                || (y == j->y && ((x > path_x && x <= j->x) || (x < path_x && x >= j->x)))) {
            kDebug() << "isInPath:" << x << "," << y << "found in path" << path_x << "," << path_y << " => " << j->x << "," << j->y;
            return true;
        }
        path_x = j->x;
        path_y = j->y;
    }
    return false;
}

Board::Board(QWidget *parent) :
        QWidget(parent), m_field(0),
        m_xTiles(0), m_yTiles(0),
        m_delay(125), m_isPaused(false),
        m_gravityFlag(true), m_solvableFlag(true), m_chineseStyleFlag(false), m_tilesCanSlideFlag(false),
        m_highlightedTile(-1), m_paintConnection(false), m_paintPossibleMoves(false)
{
    m_tileRemove1.first = -1;
    // Randomize
    setShuffle(DEFAULTSHUFFLE);

    m_random.setSeed(0);
    resetTimer();

    setDelay(DEFAULTDELAY);

    QPalette palette;
    palette.setBrush(backgroundRole(), m_background.getBackground());
    setPalette(palette);

    loadSettings();
}

Board::~Board()
{
    delete [] m_field;
}

void Board::loadSettings()
{
    if (!loadTileset(Prefs::tileSet())) {
        qDebug() << "An error occurred when loading the tileset" << Prefs::tileSet() << "KShisen will continue with the default tileset.";
    }

    // Load background
    if (!loadBackground(Prefs::background())) {
        qDebug() << "An error occurred when loading the background" << Prefs::background() << "KShisen will continue with the default background.";
    }

    // special rule
    setChineseStyleFlag(Prefs::chineseStyle());
    setTilesCanSlideFlag(Prefs::tilesCanSlide());
    // need to load solvable before size
    // because setSize call newGame which uses
    // the solvable flag
    // same with shuffle
    setSolvableFlag(Prefs::solvable());
    //setShuffle(Prefs::level() * 4 + 1);
    // actually there is no need to call setShuffle
    // and setShuffle will call newGame
    m_shuffle = Prefs::level() * 4 + 1;
    int index = Prefs::size();
    setSize(size_x[index], size_y[index]);
    setGravityFlag(Prefs::gravity());
    setDelay(DELAY[Prefs::speed()]);
}

bool Board::loadTileset(const QString &path)
{
    if (m_tiles.loadTileset(path)) {
        if (m_tiles.loadGraphics()) {
            Prefs::setTileSet(path);
            Prefs::self()->writeConfig();
            resizeBoard();
        }
        return true;
    }
    //Try default
    if (m_tiles.loadDefault()) {
        if (m_tiles.loadGraphics()) {
            Prefs::setTileSet(m_tiles.path());
            Prefs::self()->writeConfig();
            resizeBoard();
        }
    }
    return false;
}

bool Board::loadBackground(const QString& pszFileName)
{
    if (m_background.load(pszFileName, width(), height())) {
        if (m_background.loadGraphics()) {
            Prefs::setBackground(pszFileName);
            Prefs::self()->writeConfig();
            resizeBoard();
            return true;
        }
    }
    //Try default
    if (m_background.loadDefault()) {
        if (m_background.loadGraphics()) {
            Prefs::setBackground(m_background.path());
            Prefs::self()->writeConfig();
            resizeBoard();
        }
    }
    return false;
}

int Board::x_tiles() const
{
    return m_xTiles;
}

int Board::y_tiles() const
{
    return m_yTiles;
}

void Board::setField(int x, int y, int value)
{
    if (x < 0 || y < 0 || x >= x_tiles() || y >= y_tiles()) {
        kFatal() << "Attempted write to invalid field position "
        "(" << x << "," << y << ")";
    }

    m_field[y * x_tiles() + x] = value;
}

int Board::getField(int x, int y) const
{
#ifdef DEBUGGING
    if (x < -1 || y < -1 || x > x_tiles() || y > y_tiles()) {
        kFatal() << "Attempted read from invalid field position "
        "(" << x << "," << y << ")";
    }
#endif

    if (x < 0 || y < 0 || x >= x_tiles() || y >= y_tiles()) {
        return EMPTY;
    }

    return m_field[y * x_tiles() + x];
}

// check all columns and populate the affected
// columns in m_gravCols
void Board::gravity(bool update)
{
    m_gravCols.clear();
    if (!m_gravityFlag) {
        return;
    }
    for (int i = 0; i < x_tiles(); ++i) {
        if (gravity(i, update)) {
            m_gravCols.append(i);
        }
    }
}

// return whether the column col is affected by gravity
bool Board::gravity(int col, bool update)
{
    bool affected = false;
    if (m_gravityFlag) {
        int rptr = y_tiles() - 1, wptr = y_tiles() - 1;
        while (rptr >= 0) {
            if (getField(col, wptr) != EMPTY) {
                rptr--;
                wptr--;
            } else {
                if (getField(col, rptr) != EMPTY) {
                    setField(col, wptr, getField(col, rptr));
                    setField(col, rptr, EMPTY);
                    affected = true;
                    if (update) {
                        updateField(col, rptr);
                        updateField(col, wptr);
                    }
                    wptr--;
                    rptr--;
                } else {
                    rptr--;
                }
            }
        }
    }
    return affected;
}

void Board::mousePressEvent(QMouseEvent *e)
{
    // Calculate field position
    int pos_x = (e->pos().x() - xOffset()) / (m_tiles.qWidth() * 2);
    int pos_y = (e->pos().y() - yOffset()) / (m_tiles.qHeight() * 2);

    if (e->pos().x() < xOffset() || e->pos().y() < yOffset() ||
            pos_x >= x_tiles() || pos_y >= y_tiles()) {
        pos_x = -1;
        pos_y = -1;
    }

    // Mark tile
    if (e->button() == Qt::LeftButton) {
        clearHighlight();

        if (pos_x != -1) {
            marked(pos_x, pos_y);
        }
    }

    // Assist by highlighting all tiles of same type
    if (e->button() == Qt::RightButton) {
        int clicked_tile = getField(pos_x, pos_y);

        // Clear marked tile
        if (m_markX != -1 && getField(m_markX, m_markY) != clicked_tile) {
            // We need to set m_markX and m_markY to -1 before calling
            // updateField() to ensure the tile is redrawn as unmarked.
            int oldmarkx = m_markX;
            int oldmarky = m_markY;
            m_markX = -1;
            m_markY = -1;
            updateField(oldmarkx, oldmarky);
        } else {
            m_markX = -1;
            m_markY = -1;
        }

        // Perform highlighting
        if (clicked_tile != m_highlightedTile) {
            int oldHighlighted = m_highlightedTile;
            m_highlightedTile = clicked_tile;
            for (int i = 0; i < x_tiles(); ++i) {
                for (int j = 0; j < y_tiles(); ++j) {
                    const int field_tile = getField(i, j);
                    if (field_tile != EMPTY) {
                        if (field_tile == oldHighlighted) {
                            updateField(i, j);
                        } else if (field_tile == clicked_tile) {
                            updateField(i, j);
                        } else if (m_chineseStyleFlag) {
                            if (clicked_tile >= SEASONS_START && clicked_tile <= (SEASONS_START + 3) && field_tile >= SEASONS_START && field_tile <= (SEASONS_START + 3)) {
                                updateField(i, j);
                            } else if (clicked_tile >= FLOWERS_START && clicked_tile <= (FLOWERS_START + 3) && field_tile >= FLOWERS_START && field_tile <= (FLOWERS_START + 3)) {
                                updateField(i, j);
                            }
                            // oldHighlighted
                            if (oldHighlighted >= SEASONS_START && oldHighlighted <= (SEASONS_START + 3) && field_tile >= SEASONS_START && field_tile <= (SEASONS_START + 3)) {
                                updateField(i, j);
                            } else if (oldHighlighted >= FLOWERS_START && oldHighlighted <= (FLOWERS_START + 3) && field_tile >= FLOWERS_START && field_tile <= (FLOWERS_START + 3)) {
                                updateField(i, j);
                            }
                        }
                    }
                }
            }
        }
    }
}

// The board is centred inside the main playing area. xOffset/yOffset provide
// the coordinates of the top-left corner of the board.
int Board::xOffset()
{
    int tw = m_tiles.qWidth() * 2;
    return (width() - (tw * x_tiles())) / 2;
}

int Board::yOffset()
{
    int th = m_tiles.qHeight() * 2;
    return (height() - (th * y_tiles())) / 2;
}

void Board::setSize(int x, int y)
{
    if (x == x_tiles() && y == y_tiles()) {
        return;
    }

    if (m_field != 0) {
        delete [] m_field;
    }

    m_field = new int[ x * y ];
    m_xTiles = x;
    m_yTiles = y;
    for (int i = 0; i < x; ++i) {
        for (int j = 0; j < y; ++j) {
            setField(i, j, EMPTY);
        }
    }

    // set the minimum size of the scalable window
    const double MINIMUM_SCALE = 0.2;
    //int w = qRound(m_tiles.unscaledTileWidth() * MINIMUM_SCALE) * x_tiles();
    //int h = qRound(m_tiles.unscaledTileHeight() * MINIMUM_SCALE) * y_tiles();
    int w = qRound(m_tiles.qWidth() * 2.0 * MINIMUM_SCALE) * x_tiles();
    int h = qRound(m_tiles.qHeight() * 2.0 * MINIMUM_SCALE) * y_tiles();
    w += m_tiles.width();
    h += m_tiles.width();

    setMinimumSize(w, h);

    resizeBoard();
    newGame();
    emit changed();
}

void Board::resizeEvent(QResizeEvent* event)
{
    kDebug() << "[resizeEvent]";
    if (event->spontaneous()) {
        kDebug() << "[resizeEvent] spontaneous";
    }
    resizeBoard();
    emit resized();
}

void Board::resizeBoard()
{
    // calculate tile size required to fit all tiles in the window
    QSize newsize = m_tiles.preferredTileSize(QSize(width(), height()), x_tiles(), y_tiles());
    m_tiles.reloadTileset(newsize);
    //recalculate bg, if needed
    m_background.sizeChanged(width(), height());
    //reload our bg brush, using the cache in libkmahjongg if possible
    QPalette palette;
    palette.setBrush(backgroundRole(), m_background.getBackground());
    setPalette(palette);
}

QSize Board::unscaledSize()
{
    int w = m_tiles.qWidth() * 2 * x_tiles() + m_tiles.width();
    int h = m_tiles.qHeight() * 2 * y_tiles() + m_tiles.width();
    return QSize(w, h);
}

void Board::newGame()
{
    kDebug() << "NewGame";
    int i, x, y;//, k; k is unused now

    m_markX = -1;
    m_markY = -1;
    m_highlightedTile = -1; // will clear previous highlight

    resetUndo();
    resetRedo();
    m_connection.clear();
    m_possibleMoves.clear();

    // distribute all tiles on board
    int cur_tile = 1;
    int tile_count = 0;
    /*
     * Note by jwickers: i changed the way to distribute tiles
     *  in chinese mahjongg there are 4 tiles of each
     *  except flowers and seasons (4 flowers and 4 seasons,
     *  but one unique tile of each, that is why they are
     *  the only ones numbered)
     * That uses the chineseStyle flag
     */
    for (y = 0; y < y_tiles(); ++y) {
        for (x = 0; x < x_tiles(); ++x) {
            // do not duplicate flowers or seasons
            if (!m_chineseStyleFlag || !((cur_tile >= SEASONS_START && cur_tile <= (SEASONS_START + 3)) || (cur_tile >= FLOWERS_START && cur_tile <= (FLOWERS_START + 3)))) {
                setField(x, y, cur_tile);
                if (++tile_count >= 4) {
                    tile_count = 0;
                    ++cur_tile;
                }
            } else {
                tile_count = 0;
                setField(x, y, cur_tile++);
            }
            if (cur_tile > Board::nTiles) {
                cur_tile = 1;
            }
        }
    }

    if (getShuffle() == 0) {
        update();
        resetTimer();
        emit changed();
        return;
    }

    // shuffle the field
    int tx = x_tiles();
    int ty = y_tiles();
    for (i = 0; i < x_tiles() * y_tiles() * getShuffle(); ++i) {
        int x1 = m_random.getLong(tx);
        int y1 = m_random.getLong(ty);
        int x2 = m_random.getLong(tx);
        int y2 = m_random.getLong(ty);
        int t  = getField(x1, y1);
        setField(x1, y1, getField(x2, y2));
        setField(x2, y2, t);
    }

    // do not make solvable if m_solvableFlag is false
    if (!m_solvableFlag) {
        update();
        resetTimer();
        emit changed();
        return;
    }


    int fsize = x_tiles() * y_tiles() * sizeof(int);
    int *oldfield = new int[x_tiles() * y_tiles()];
    memcpy(oldfield, m_field, fsize);   // save field
    int *tiles = new int[x_tiles() * y_tiles()];
    int *pos = new int[x_tiles() * y_tiles()];
    //jwickers: in case the game cannot make the game solvable we do not want to run an infinite loop
    int max_attempts = 200;

    while (!solvable(true) && max_attempts > 0) {
        //kDebug() << "Not solvable";
        //dumpBoard();

        // generate a list of free tiles and positions
        int num_tiles = 0;
        for (i = 0; i < x_tiles() * y_tiles(); ++i) {
            if (m_field[i] != EMPTY) {
                pos[num_tiles] = i;
                tiles[num_tiles] = m_field[i];
                ++num_tiles;
            }
        }

        // restore field
        memcpy(m_field, oldfield, fsize);

        // redistribute unsolved tiles
        while (num_tiles > 0) {
            // get a random tile
            int r1 = m_random.getLong(num_tiles);
            int r2 = m_random.getLong(num_tiles);
            int tile = tiles[r1];
            int apos = pos[r2];

            // truncate list
            tiles[r1] = tiles[num_tiles-1];
            pos[r2] = pos[num_tiles-1];
            num_tiles--;

            // put this tile on the new position
            m_field[apos] = tile;
        }

        // remember field
        memcpy(oldfield, m_field, fsize);
        max_attempts--;
    }
    // debug, tell if make solvable failed
    if (max_attempts == 0) {
        kDebug() << "NewGame make solvable failed";
    }


    // restore field
    memcpy(m_field, oldfield, fsize);
    delete [] tiles;
    delete [] pos;
    delete [] oldfield;

    update();
    resetTimer();
    emit changed();
}

/*
 * Check that two tiles can match
 *  used for connecting them or highlighting tiles of the same group
 */
bool Board::tilesMatch(int tile1, int tile2) const
{
    // identical tiles always match
    if (tile1 == tile2) {
        return true;
    }
    // when chinese style is set, there are special rules
    // for flowers and seasons
    if (m_chineseStyleFlag) {
        // if both tiles are seasons
        if (tile1 >= SEASONS_START && tile1 <= SEASONS_START + 3
                && tile2 >= SEASONS_START && tile2 <= SEASONS_START + 3) {
            return true;
        }
        // if both tiles are flowers
        if (tile1 >= FLOWERS_START && tile1 <= FLOWERS_START + 3
                && tile2 >= FLOWERS_START && tile2 <= FLOWERS_START + 3) {
            return true;
        }
    }
    return false;
}

bool Board::isTileHighlighted(int x, int y) const
{
    if (x == m_markX && y == m_markY) {
        return true;
    }

    if (tilesMatch(m_highlightedTile, getField(x, y))) {
        return true;
    }

    // m_tileRemove1.first != -1 is used because the repaint of the first if
    // on undrawConnection highlihgted the tiles that fell because of gravity
    if (!m_connection.isEmpty() && m_tileRemove1.first != -1) {
        if (x == m_connection.first().x && y == m_connection.first().y) {
            return true;
        }

        if (x == m_connection.last().x && y == m_connection.last().y) {
            return true;
        }
    }

    return false;
}

void Board::updateField(int x, int y)
{
    QRect r(xOffset() + x * m_tiles.qWidth() * 2,
            yOffset() + y * m_tiles.qHeight() * 2,
            m_tiles.width(),
            m_tiles.height());

#ifdef USE_UPDATE
    update(r);
#else
    repaint(r);
#endif
}

void Board::paintEvent(QPaintEvent *e)
{
    QRect ur = e->rect();            // rectangle to update
    QPixmap pm(ur.size());           // Pixmap for double-buffering
    pm.fill(this, ur.topLeft());     // fill with widget background
    QPainter p(&pm);
    p.translate(-ur.x(), -ur.y());   // use widget coordinate system

    if (m_isPaused) {
        p.setFont(KGlobalSettings::largeFont());
        p.drawText(rect(), Qt::AlignCenter, i18n("Game Paused"));
    } else {
        int w = m_tiles.width();
        int h = m_tiles.height();
        int fw = m_tiles.qWidth() * 2;
        int fh = m_tiles.qHeight() * 2;
        for (int i = 0; i < x_tiles(); ++i) {
            for (int j = 0; j < y_tiles(); ++j) {
                int tile = getField(i, j);
                if (tile == EMPTY) {
                    continue;
                }

                int xpos = xOffset() + i * fw;
                int ypos = yOffset() + j * fh;
                QRect r(xpos, ypos, w, h);
                if (e->rect().intersects(r)) {
                    if (isTileHighlighted(i, j)) {
                        p.drawPixmap(xpos, ypos, m_tiles.selectedTile(1));
                    } else {
                        p.drawPixmap(xpos, ypos, m_tiles.unselectedTile(1));
                    }

                    //draw face
                    p.drawPixmap(xpos, ypos, m_tiles.tileface(tile - 1));
                }
            }
        }
    }
    p.end();

    p.begin(this);
    p.drawPixmap(ur.topLeft(), pm);

    if (m_paintConnection) {
        p.setPen(QPen(QColor("red"), lineWidth()));

        // Path.size() will always be >= 2
        Path::const_iterator pathEnd = m_connection.constEnd();
        Path::const_iterator pt1 = m_connection.constBegin();
        Path::const_iterator pt2 = pt1;
        ++pt2;
        while (pt2 != pathEnd) {
            p.drawLine(midCoord(pt1->x, pt1->y), midCoord(pt2->x, pt2->y));
            ++pt1;
            ++pt2;
        }
        QTimer::singleShot(m_connectionTimeout, this, SLOT(undrawConnection()));
        m_paintConnection = false;
    }
    if (m_paintPossibleMoves) {
        p.setPen(QPen(QColor("blue"), lineWidth()));
        // paint all possible moves
        QList<PossibleMove>::iterator i;
        for (i = m_possibleMoves.begin(); i != m_possibleMoves.end(); ++i) {
            // Path.size() will always be >= 2
            Path::const_iterator pathEnd = i->path.constEnd();
            Path::const_iterator pt1 = i->path.constBegin();
            Path::const_iterator pt2 = pt1;
            ++pt2;
            while (pt2 != pathEnd) {
                p.drawLine(midCoord(pt1->x, pt1->y), midCoord(pt2->x, pt2->y));
                ++pt1;
                ++pt2;
            }
        }
        m_paintConnection = false;
    }
    p.end(); //this
}

void Board::reverseSlide(int x, int y, int s_x1, int s_y1, int s_x2, int s_y2)
{
    //kDebug() << "reverseSlide" << x << " " << y;

    // s_x2 is the current location of the last tile to slide
    // s_x1 is its destination
    // calculate the offset for the tiles to slide
    int dx = s_x1 - s_x2;
    int dy = s_y1 - s_y2;
    /*kDebug() << "reverseSlide last to slide is x=" << s_x2 << ", y=" << s_y2;
      kDebug() << "reverseSlide slide to x=" << s_x1 << ", y=" << s_y1;
      kDebug() << "reverseSlide offset dx=" << dx << ", dy=" << dy;*/
    int current_tile;
    // move all tiles between s_x2, s_y2 and x, y to slide with that offset
    if (dx == 0) {
        if (y < s_y2) {
            for (int i = y + 1; i <= s_y2; ++i) {
                current_tile = getField(x, i);
                if (current_tile == EMPTY) {
                    continue;
                }
                setField(x, i, EMPTY);
                setField(x, i + dy, current_tile);
                updateField(x, i);
                updateField(x, i + dy);
            }
        } else {
            for (int i = y - 1; i >= s_y2; i--) {
                current_tile = getField(x, i);
                if (current_tile == EMPTY) {
                    continue;
                }
                setField(x, i, EMPTY);
                setField(x, i + dy, current_tile);
                updateField(x, i);
                updateField(x, i + dy);
            }
        }
    } else if (dy == 0) {
        if (x < s_x2) {
            for (int i = x + 1; i <= s_x2; ++i) {
                current_tile = getField(i, y);
                if (current_tile == EMPTY) {
                    continue;
                }
                setField(i, y, EMPTY);
                setField(i + dx, y, current_tile);
                updateField(i, y);
                updateField(i + dx, y);
            }
        } else {
            for (int i = x - 1; i >= s_x2; i--) {
                current_tile = getField(i, y);
                if (current_tile == EMPTY) {
                    continue;
                }
                setField(i, y, EMPTY);
                setField(i + dx, y, current_tile);
                updateField(i, y);
                updateField(i + dx, y);
            }
        }
    }
}

void Board::performSlide(int x, int y, Path& s)
{
    //kDebug() << "performSlide" << x << " " << y;

    // check if there is something to slide
    if (s.empty()) {
        return;
    }

    // slide.first is the current location of the last tile to slide
    // slide.last is its destination
    // calculate the offset for the tiles to slide
    int dx = s.last().x - s.first().x;
    int dy = s.last().y - s.first().y;
    /*kDebug() << "performSlide last to slide is x=" << s.first().x << ", y=" << s.first().y;
      kDebug() << "performSlide slide to x=" << s.last().x << ", y=" << s.last().y;
      kDebug() << "performSlide offset dx=" << dx << ", dy=" << dy;*/
    int current_tile;
    // move all tiles between m_markX, m_markY and the last tile to slide with that offset
    if (dx == 0) {
        if (y < s.first().y) {
            for (int i = s.first().y; i > y; --i) {
                current_tile = getField(x, i);
                setField(x, i, EMPTY);
                setField(x, i + dy, current_tile);
                updateField(x, i);
                updateField(x, i + dy);
            }
        } else {
            for (int i = s.first().y; i < y; ++i) {
                current_tile = getField(x, i);
                setField(x, i, EMPTY);
                setField(x, i + dy, current_tile);
                updateField(x, i);
                updateField(x, i + dy);
            }
        }
    } else if (dy == 0) {
        if (x < s.first().x) {
            for (int i = s.first().x; i > x; --i) {
                current_tile = getField(i, y);
                setField(i, y, EMPTY);
                setField(i + dx, y, current_tile);
                updateField(i, y);
                updateField(i + dx, y);
            }
        } else {
            for (int i = s.first().x; i < x; ++i) {
                current_tile = getField(i, y);
                setField(i, y, EMPTY);
                setField(i + dx, y, current_tile);
                updateField(i, y);
                updateField(i + dx, y);
            }
        }
    }
}

void Board::performMove(PossibleMove& p)
{
    m_connection = p.path;
#ifdef DEBUGGING
    // DEBUG undo, save board state
    int fsize = x_tiles() * y_tiles() * sizeof(int);
    int *saved1 = new int[x_tiles() * y_tiles()];
    memcpy(saved1, m_field, fsize);
#endif
    // if the tiles can slide, we have to update the slided tiles too
    // and store the slide in a Move
    if (p.hasSlide) {
        performSlide(m_markX, m_markY, p.slide);
        madeMoveWithSlide(m_markX, m_markY, p.path.last().x, p.path.last().y, p.slide);
    } else {
        madeMove(m_markX, m_markY, p.path.last().x, p.path.last().y);
    }
    undrawPossibleMoves();
    drawConnection(getDelay());
    m_tileRemove1 = QPair<int, int>(m_markX, m_markY);
    m_tileRemove2 = QPair<int, int>(p.path.last().x, p.path.last().y);
    m_markX = -1;
    m_markY = -1;
    m_possibleMoves.clear();
#ifdef DEBUGGING
    // DEBUG undo, force gravity
    undrawConnection();
    // DEBUG undo, save board2 state
    int *saved2 = new int[x_tiles() * y_tiles()];
    int *saved3 = new int[x_tiles() * y_tiles()]; // after undo
    int *saved4 = new int[x_tiles() * y_tiles()]; // after redo
    memcpy(saved2, m_field, fsize);
    // DEBUG undo, undo move
    bool errorfound = false;
    if (canUndo()) {
        undo();
        // DEBUG undo, compare to saved board state
        for (int i = 0; i < x_tiles() * y_tiles(); ++i) {
            if (saved1[i] != m_field[i]) {
                kDebug() << "[DEBUG Undo 1], tile (" << i << ") was" << saved1[i] << "before more, it is" << m_field[i] << "after undo.";
                errorfound = true;
            }
        }
        // DEBUG undo, save board state
        memcpy(saved3, m_field, fsize);
        // DEBUG undo, redo
        if (canRedo()) {
            redo();
            undrawConnection();
            // DEBUG undo, compare to saved board2 state
            for (int i = 0; i < x_tiles() * y_tiles(); ++i) {
                if (saved2[i] != m_field[i]) {
                    kDebug() << "[DEBUG Undo 2], tile (" << i << ") was" << saved2[i] << "after more, it is" << m_field[i] << "after redo.";
                    errorfound = true;
                }
            }
            // DEBUG undo, save board state
            memcpy(saved4, m_field, fsize);
        }
    }
    // dumpBoard on error
    if (errorfound) {
        kDebug() << "[DEBUG] Before move";
        dumpBoard(saved1);
        kDebug() << "[DEBUG] After move";
        dumpBoard(saved2);
        kDebug() << "[DEBUG] Undo";
        dumpBoard(saved3);
        kDebug() << "[DEBUG] Redo";
        dumpBoard(saved4);
    }

    // DEBUG undo, free saved boards
    delete[] saved1;
    delete[] saved2;
    delete[] saved3;
    delete[] saved4;
#endif
}

void Board::marked(int x, int y)
{
    // make sure that the previous connection is correctly undrawn
    undrawConnection();

    if (x == m_markX && y == m_markY) {
        // unmark the piece
        m_markX = -1;
        m_markY = -1;
        undrawPossibleMoves();
        m_possibleMoves.clear();
        updateField(x, y);
        emit selectATile();
        return;
    }

    if (m_markX == -1) {
        if (getField(x, y) == EMPTY) {
            return;
        }
        m_markX = x;
        m_markY = y;
        undrawPossibleMoves();
        m_possibleMoves.clear();
        updateField(x, y);
        emit selectAMatchingTile();
        return;
    } else if (m_possibleMoves.count() > 1) {  // if the click is on any of the current possible moves, make that move
        //kDebug() << "marked: there may be a move to be selected";
        QList<PossibleMove>::iterator i;

        for (i = m_possibleMoves.begin(); i != m_possibleMoves.end(); ++i) {
            if (i->isInPath(x, y)) {
                performMove(*i);
                emit selectATile();
                return;
            }
        }
    }
    if (getField(x, y) == EMPTY) {
        return;
    }

    int fld1 = getField(m_markX, m_markY);
    int fld2 = getField(x, y);

    // both field match
    if (!tilesMatch(fld1, fld2)) {
        emit tilesDontMatch();
        return;
    }

    // trace and perform the move and get the list of possible moves
    if (findPath(m_markX, m_markY, x, y, m_possibleMoves)) {
        if (m_possibleMoves.count() > 1) {
            //kDebug() << "marked: there was" << m_possibleMoves.count() << "moves possible for this";
            QList<PossibleMove>::iterator i;
            int withSlide = 0;
            for (i = m_possibleMoves.begin(); i != m_possibleMoves.end(); ++i) {
                i->Debug();
                if (i->hasSlide) {
                    ++withSlide;
                }
            }
            // if all moves have no slide, it doesn't matter
            if (withSlide > 0) {
                drawPossibleMoves();
                emit selectAMove();
                return;
            }
        }

        // only one move possible, perform it
        performMove(m_possibleMoves.first());
        emit selectATile();
        // game is over?
        // Must delay until after tiles fall to make this test
        // See undrawConnection GP.
    } else {
        emit invalidMove();
        m_connection.clear();
    }
}


void Board::clearHighlight()
{
    if (m_highlightedTile != -1) {
        int oldHighlighted = m_highlightedTile;
        m_highlightedTile = -1;

        for (int i = 0; i < x_tiles(); ++i) {
            for (int j = 0; j < y_tiles(); ++j) {
                if (tilesMatch(oldHighlighted, getField(i, j))) {
                    updateField(i, j);
                }
            }
        }
    }
}

// Can we make a path between two tiles with a single line?
bool Board::canMakePath(int x1, int y1, int x2, int y2) const
{
    if (x1 == x2) {
        for (int i = qMin(y1, y2) + 1; i < qMax(y1, y2); ++i) {
            if (getField(x1, i) != EMPTY) {
                return false;
            }
        }
        return true;
    }

    if (y1 == y2) {
        for (int i = qMin(x1, x2) + 1; i < qMax(x1, x2); ++i) {
            if (getField(i, y1) != EMPTY) {
                return false;
            }
        }
        return true;
    }

    return false;
}

// Can we slide the tile at (x1,y1) to (x2,y2)
// the movement of the last tile slided will be stored in the Path p
bool Board::canSlideTiles(int x1, int y1, int x2, int y2, Path& p) const
{
    int distance = -1;
    p.clear();
    if (x1 == x2) {
        if (y1 > y2) {
            distance = y1 - y2;
            // count how much free space we have for sliding
            int start_free = -1;
            int end_free = -1;
            // find first tile empty
            for (int i = y1 - 1; i >= 0; i--) {
                if (getField(x1, i) == EMPTY) {
                    start_free = i;
                    break;
                }
            }
            // if not found, cannot slide
            // if the first free tile is just next to the sliding tile, no slide (should be a normal move)
            if (start_free == -1 || start_free == y1 - 1) {
                //kDebug() << "canSlideTiles no free";
                return false;
            }
            // find last tile empty
            for (int i = start_free - 1; i >= 0; i--) {
                if (getField(x1, i) != EMPTY) {
                    end_free = i;
                    break;
                }
            }
            // if not found, it is the border: 0

            // so we can slide of start_free - end_free, compare this to the distance
            //kDebug() << "canSlideTiles distance=" << distance << "free=" << (start_free - end_free);
            if (distance <= (start_free - end_free)) {
                // first position of the last slided tile
                p.append(Position(x1, start_free + 1));
                // final position of the last slided tile
                p.append(Position(x1, start_free + 1 - distance));
                return true;
            } else {
                return false;
            }
        } else if (y2 > y1) {
            distance = y2 - y1;
            // count how much free space we have for sliding
            int start_free = -1;
            int end_free = y_tiles();
            // find first tile empty
            for (int i = y1 + 1; i < y_tiles(); ++i) {
                if (getField(x1, i) == EMPTY) {
                    start_free = i;
                    break;
                }
            }
            // if not found, cannot slide
            // if the first free tile is just next to the sliding tile, no slide (should be a normal move)
            if (start_free == -1 || start_free == y1 + 1) {
                //kDebug() << "canSlideTiles no free";
                return false;
            }
            // find last tile empty
            for (int i = start_free + 1; i < y_tiles(); ++i) {
                if (getField(x1, i) != EMPTY) {
                    end_free = i;
                    break;
                }
            }
            // if not found, it is the border: y_tiles()-1

            // so we can slide of end_free - start_free, compare this to the distance
            //kDebug() << "canSlideTiles distance=" << distance << "free=" << (end_free - start_free);
            if (distance <= (end_free - start_free)) {
                // first position of the last slided tile
                p.append(Position(x1, start_free - 1));
                // final position of the last slided tile
                p.append(Position(x1, start_free - 1 + distance));
                return true;
            } else {
                return false;
            }
        }
        // y1 == y2 ?!
        return false;
    }

    if (y1 == y2) {
        if (x1 > x2) {
            distance = x1 - x2;
            // count how much free space we have for sliding
            int start_free = -1;
            int end_free = -1;
            // find first tile empty
            for (int i = x1 - 1; i >= 0; i--) {
                if (getField(i, y1) == EMPTY) {
                    start_free = i;
                    break;
                }
            }
            // if not found, cannot slide
            // if the first free tile is just next to the sliding tile, no slide (should be a normal move)
            if (start_free == -1 || start_free == x1 - 1) {
                //kDebug() << "canSlideTiles no free";
                return false;
            }
            // find last tile empty
            for (int i = start_free - 1; i >= 0; i--) {
                if (getField(i, y1) != EMPTY) {
                    end_free = i;
                    break;
                }
            }
            // if not found, it is the border: 0

            // so we can slide of start_free - end_free, compare this to the distance
            //kDebug() << "canSlideTiles distance=" << distance << "free=" << (start_free - end_free);
            if (distance <= (start_free - end_free)) {
                // first position of the last slided tile
                p.append(Position(start_free + 1, y1));
                // final position of the last slided tile
                p.append(Position(start_free + 1 - distance, y1));
                return true;
            } else {
                return false;
            }
        } else if (x2 > x1) {
            distance = x2 - x1;
            // count how much free space we have for sliding
            int start_free = -1;
            int end_free = x_tiles();
            // find first tile empty
            for (int i = x1 + 1; i < x_tiles(); ++i) {
                if (getField(i, y1) == EMPTY) {
                    start_free = i;
                    break;
                }
            }
            // if not found, cannot slide
            // if the first free tile is just next to the sliding tile, no slide (should be a normal move)
            if (start_free == -1 || start_free == x1 + 1) {
                //kDebug() << "canSlideTiles no free";
                return false;
            }
            // find last tile empty
            for (int i = start_free + 1; i < x_tiles(); ++i) {
                if (getField(i, y1) != EMPTY) {
                    end_free = i;
                    break;
                }
            }
            // if not found, it is the border: x_tiles()-1

            // so we can slide of end_free - start_free, compare this to the distance
            //kDebug() << "canSlideTiles distance=" << distance << "free=" << (end_free - start_free);
            if (distance <= (end_free - start_free)) {
                // first position of the last slided tile
                p.append(Position(start_free - 1, y1));
                // final position of the last slided tile
                p.append(Position(start_free - 1 + distance, y1));
                return true;
            } else {
                return false;
            }
        }
        // x1 == x2 ?!
        return false;
    }
    return false;
}

// Can we make a path between two tiles (with 2 or 3 lines) ?
// returns the number of possible paths
// put all the paths found in p
int Board::findPath(int x1, int y1, int x2, int y2, PossibleMoves& p) const
{
    p.clear();

    int n_path = 0;
    int n_simple_path = 0;

    // first find the simple paths
    n_path = findSimplePath(x1, y1, x2, y2, p);

    // if the tiles can slide, 2 lines max is allowed
    if (m_tilesCanSlideFlag) {
        return n_path;
    }

    // Find paths of 3 segments
    const int dx[4] = { 1, 0, -1, 0 };
    const int dy[4] = { 0, 1, 0, -1 };

    for (int i = 0; i < 4; ++i) {
        int newx = x1 + dx[i];
        int newy = y1 + dy[i];
        while (newx >= -1 && newx <= x_tiles() &&
                newy >= -1 && newy <= y_tiles() &&
                getField(newx, newy) == EMPTY) {
            if ((n_simple_path = findSimplePath(newx, newy, x2, y2, p))) {
                p.last().path.prepend(Position(x1, y1));
                n_path += n_simple_path;
            }
            newx += dx[i];
            newy += dy[i];
        }
    }
    return n_path;
}

// Find a path of 1 or 2 segments between tiles.
// Returns the number of paths found
// and all the possible moves in p
int Board::findSimplePath(int x1, int y1, int x2, int y2, PossibleMoves& p) const
{
    int n_path = 0;
    Path _p;
    // Find direct line (path of 1 segment)
    if (canMakePath(x1, y1, x2, y2)) {
        _p.append(Position(x1, y1));
        _p.append(Position(x2, y2));
        p.append(PossibleMove(_p));
        ++n_path;
    }

    // If the tiles are in the same row or column, then a
    // a 'simple path' cannot be found between them
    // That is, canMakePath should have returned true above if
    // that was possible
    if (x1 == x2 || y1 == y2) {
        return n_path;
    }

    // I isolate the special code when tiles can slide even if it duplicates code for now
    // Can we make a path sliding tiles ?, the slide move is always first, then a normal path
    if (m_tilesCanSlideFlag) {
        Path _s;
        // Find path of 2 segments (route A)
        if (canSlideTiles(x1, y1, x2, y1, _s) && canMakePath(x2, y1, x2, y2)) {
            _p.clear();
            _p.append(Position(x1, y1));
            _p.append(Position(x2, y1));
            _p.append(Position(x2, y2));
            p.append(PossibleMove(_p, _s));
            ++n_path;
        }

        // Find path of 2 segments (route B)
        if (canSlideTiles(x1, y1, x1, y2, _s) && canMakePath(x1, y2, x2, y2)) {
            _p.clear();
            _p.append(Position(x1, y1));
            _p.append(Position(x1, y2));
            _p.append(Position(x2, y2));
            p.append(PossibleMove(_p, _s));
            ++n_path;
        }
    }

    // Even if tiles can slide, a path could still be done without sliding

    // Find path of 2 segments (route A)
    if (getField(x2, y1) == EMPTY && canMakePath(x1, y1, x2, y1) &&
            canMakePath(x2, y1, x2, y2)) {
        _p.clear();
        _p.append(Position(x1, y1));
        _p.append(Position(x2, y1));
        _p.append(Position(x2, y2));
        p.append(PossibleMove(_p));
        ++n_path;
    }

    // Find path of 2 segments (route B)
    if (getField(x1, y2) == EMPTY && canMakePath(x1, y1, x1, y2) &&
            canMakePath(x1, y2, x2, y2)) {
        _p.clear();
        _p.append(Position(x1, y1));
        _p.append(Position(x1, y2));
        _p.append(Position(x2, y2));
        p.append(PossibleMove(_p));
        ++n_path;
    }

    return n_path;
}

void Board::drawPossibleMoves()
{
    if (m_possibleMoves.isEmpty()) {
        return;
    }

    m_paintPossibleMoves = true;
    update();
}

void Board::undrawPossibleMoves()
{
    if (m_possibleMoves.isEmpty()) {
        return;
    }

    m_paintPossibleMoves = false;
    update();
}

void Board::drawConnection(int timeout)
{
    if (m_connection.isEmpty()) {
        return;
    }

    int x1 = m_connection.first().x;
    int y1 = m_connection.first().y;
    int x2 = m_connection.last().x;
    int y2 = m_connection.last().y;
    // lighten the fields
    updateField(x1, y1);
    updateField(x2, y2);

    m_connectionTimeout = timeout;
    m_paintConnection = true;
    update();
}

void Board::undrawConnection()
{
    if (m_tileRemove1.first != -1) {
        setField(m_tileRemove1.first, m_tileRemove1.second, EMPTY);
        setField(m_tileRemove2.first, m_tileRemove2.second, EMPTY);
        m_tileRemove1.first = -1;
#ifdef USE_UPDATE
        update();
#else
        repaint();
#endif
    }

    /*if(grav_col_1 != -1 || grav_col_2 != -1)
      {
      gravity(grav_col_1, true);
      gravity(grav_col_2, true);
      grav_col_1 = -1;
      grav_col_2 = -1;
      }*/

    gravity(true);

    // is already undrawn?
    if (m_connection.isEmpty()) {
        return;
    }

    // Redraw all affected fields

    Path oldConnection = m_connection;
    m_connection.clear();
    m_paintConnection = false;

    // Path.size() will always be >= 2
    Path::const_iterator pathEnd = oldConnection.constEnd();
    Path::const_iterator pt1 = oldConnection.constBegin();
    Path::const_iterator pt2 = pt1;
    ++pt2;
    while (pt2 != pathEnd) {
        if (pt1->y == pt2->y) {
            for (int i = qMin(pt1->x, pt2->x); i <= qMax(pt1->x, pt2->x); ++i) {
                updateField(i, pt1->y);
            }
        } else {
            for (int i = qMin(pt1->y, pt2->y); i <= qMax(pt1->y, pt2->y); ++i) {
                updateField(pt1->x, i);
            }
        }
        ++pt1;
        ++pt2;
    }

    PossibleMoves dummyPossibleMoves;
    // game is over?
    if (!getHint_I(dummyPossibleMoves)) {
        m_timeForGame = (int)difftime(time((time_t)0), m_startTime);
        emit endOfGame();
    }
}

QPoint Board::midCoord(int x, int y)
{
    QPoint p;
    int w = m_tiles.qWidth() * 2;
    int h = m_tiles.qHeight() * 2;

    if (x == -1) {
        p.setX(xOffset() - (w / 4));
    } else if (x == x_tiles()) {
        p.setX(xOffset() + (w * x_tiles()) + (w / 4));
    } else {
        p.setX(xOffset() + (w * x) + (w / 2));
    }

    if (y == -1) {
        p.setY(yOffset() - (w / 4));
    } else if (y == y_tiles()) {
        p.setY(yOffset() + (h * y_tiles()) + (w / 4));
    } else {
        p.setY(yOffset() + (h * y) + (h / 2));
    }

    return p;
}

void Board::setDelay(int newvalue)
{
    m_delay = newvalue;
}

int Board::getDelay() const
{
    return m_delay;
}

void Board::madeMove(int x1, int y1, int x2, int y2)
{
    Path s;
    s.clear();
    madeMoveWithSlide(x1, y1, x2, y2, s);
}

void Board::madeMoveWithSlide(int x1, int y1, int x2, int y2, Path& s)
{
    Move *m;
    if (s.empty()) {
        m = new Move(x1, y1, x2, y2, getField(x1, y1), getField(x2, y2));
    } else {
        m = new Move(x1, y1, x2, y2, getField(x1, y1), getField(x2, y2), s.first().x, s.first().y, s.last().x, s.last().y);
    }
    m_undo.append(m);
    while (m_redo.count()) {
        delete m_redo.first();
        m_redo.removeFirst();
    }
    emit changed();
}

bool Board::canUndo() const
{
    return !m_undo.isEmpty();
}

bool Board::canRedo() const
{
    return !m_redo.isEmpty();
}

void Board::undo()
{
    if (canUndo()) {
        clearHighlight();
        undrawConnection();
        Move* m = m_undo.takeLast();
        if (gravityFlag()) {
            int y;

            // When both tiles reside in the same column, the order of undo is
            // significant (we must undo the lower tile first).
            // Also in that case there cannot be a slide
            if (m->x1 == m->x2 && m->y1 < m->y2) {
                qSwap(m->x1, m->x2);
                qSwap(m->y1, m->y2);
                qSwap(m->tile1, m->tile2);
            }

            // if there is no slide, keep previous implementation: move both column up
            if (!m->hasSlide) {
#ifdef DEBUGGING
                kDebug() << "[undo] gravity from a no slide move";
#endif
                // move tiles from the first column up
                for (y = 0; y < m->y1; ++y) {
                    setField(m->x1, y, getField(m->x1, y + 1));
                    updateField(m->x1, y);
                }

                // move tiles from the second column up
                for (y = 0; y < m->y2; ++y) {
                    setField(m->x2, y, getField(m->x2, y + 1));
                    updateField(m->x2, y);
                }
            } else { // else check all tiles from the slide that may have fallen down
#ifdef DEBUGGING
                kDebug() << "[undo] gravity from slide s1(" << m->slide_x1 << "," << m->slide_y1 << ")=>s2(" << m->slide_x2 << "," << m->slide_y2 << ") matching (" << m->x1 << "," << m->y1 << ")=>(" << m->x2 << "," << m->y2 << ")";
#endif
                // horizontal slide
                // because tiles that slides horizontaly may fall down
                // in columns different than the taken tiles columns
                // we need to take them back up then undo the slide
                if (m->slide_y1 == m->slide_y2) {
#ifdef DEBUGGING
                    kDebug() << "[undo] gravity from horizontal slide";
#endif
                    // last slide tile went from slide_x1 -> slide_x2
                    // the number of slided tiles is n = abs(x1 - slide_x1)
                    int n = m->x1 - m->slide_x1;
                    if (n < 0) {
                        n = -n;
                    }
                    // distance slided is
                    int dx = m->slide_x2 - m->slide_x1;
                    if (dx < 0) {
                        dx = -dx;
                    }
#ifdef DEBUGGING
                    kDebug() << "[undo] n =" << n;
#endif
                    // slided tiles may fall down after the slide
                    // so any tiles on top of the columns between
                    // slide_x2 -> slide_x2 +/- n (excluded) should go up to slide_y1
                    if (m->slide_x2 > m->slide_x1) {  // slide to the right
#ifdef DEBUGGING
                        kDebug() << "[undo] slide right";
#endif
                        for (int i = m->slide_x2; i > m->slide_x2 - n; --i) {
                            // find top tile
                            int j;
                            for (j = 0; j < y_tiles(); ++j) {
                                if (getField(i, j) != EMPTY) {
                                    break;
                                }
                            }

                            // ignore if the tile did not fall
                            if (j <= m->slide_y1) {
                                continue;
                            }
#ifdef DEBUGGING
                            kDebug() << "[undo] moving (" << i << "," << j << ") up to (" << i << "," << m->slide_y1 << ")";
#endif
                            // put it back up
                            setField(i, m->slide_y1, getField(i, j));
                            setField(i, j, EMPTY);
                            updateField(i, j);
                            updateField(i, m->slide_y1);
                        }
                    } else { // slide to the left
#ifdef DEBUGGING
                        kDebug() << "[undo] slide left";
#endif
                        for (int i = m->slide_x2; i < m->slide_x2 + n; ++i) {
                            // find top tile
                            int j;
                            for (j = 0; j < y_tiles(); ++j) {
                                if (getField(i, j) != EMPTY) {
                                    break;
                                }
                            }

                            // ignore if the tile did not fall
                            if (j <= m->slide_y1) {
                                continue;
                            }
#ifdef DEBUGGING
                            kDebug() << "[undo] moving (" << i << "," << j << ") up to (" << i << "," << m->slide_y1 << ")";
#endif
                            // put it back up
                            setField(i, m->slide_y1, getField(i, j));
                            setField(i, j, EMPTY);
                            updateField(i, j);
                            updateField(i, m->slide_y1);
                        }
                    }
                    // move tiles from the second column up
#ifdef DEBUGGING
                    kDebug() << "[undo] moving up column x2" << m->x2;
#endif
                    for (y = 0; y <= m->y2; ++y) {
#ifdef DEBUGGING
                        kDebug() << "[undo] moving up tile" << y + 1;
#endif
                        setField(m->x2, y, getField(m->x2, y + 1));
                        updateField(m->x2, y);
                    }
                    // and all columns that fell after the tiles slided between
                    // only if they were not replaced by a sliding tile !!
                    // x1 -> x1+dx should go up one
                    // if their height > slide_y1
                    // because they have fallen after the slide
                    if (m->slide_x2 > m->slide_x1) {  // slide to the right
                        if (m->slide_y1 > 0) {
                            for (int i = m->x1 + dx; i >= m->x1; --i) {
#ifdef DEBUGGING
                                kDebug() << "[undo] moving up column" << i << "until" << m->slide_y1;
#endif
                                for (int j = 0; j < m->slide_y1; ++j) {
#ifdef DEBUGGING
                                    kDebug() << "[undo] moving up tile" << j + 1;
#endif
                                    setField(i, j, getField(i, j + 1));
                                    updateField(i, j);
                                }
#ifdef DEBUGGING
                                kDebug() << "[undo] clearing last tile" << m->slide_y1;
#endif
                                setField(i, m->slide_y1, EMPTY);
                                updateField(i, m->slide_y1);
                            }
                        }
                    } else { // slide to the left
                        if (m->slide_y1 > 0) {
                            for (int i = m->x1 - dx; i <= m->x1; ++i) {
#ifdef DEBUGGING
                                kDebug() << "[undo] moving up column" << i << "until" << m->slide_y1;
#endif
                                for (int j = 0; j < m->slide_y1; ++j) {
#ifdef DEBUGGING
                                    kDebug() << "[undo] moving up tile" << j + 1;
#endif
                                    setField(i, j, getField(i, j + 1));
                                    updateField(i, j);
                                }
#ifdef DEBUGGING
                                kDebug() << "[undo] clearing last tile" << m->slide_y1;
#endif
                                setField(i, m->slide_y1, EMPTY);
                                updateField(i, m->slide_y1);
                            }
                        }
                    }

                    // then undo the slide to put the tiles back to their original location
#ifdef DEBUGGING
                    kDebug() << "[undo] reversing slide";
#endif
                    reverseSlide(m->x1, m->y1, m->slide_x1, m->slide_y1, m->slide_x2, m->slide_y2);

                } else {
                    // vertical slide, in fact nothing special is necessary
                    // the default implementation works because it only affects
                    // the two columns were tiles were taken
#ifdef DEBUGGING
                    kDebug() << "[undo] gravity from vertical slide";
#endif

                    // move tiles from the first column up
                    for (y = 0; y < m->y1; ++y) {
                        setField(m->x1, y, getField(m->x1, y + 1));
                        updateField(m->x1, y);
                    }

                    // move tiles from the second column up
                    for (y = 0; y < m->y2; ++y) {
                        setField(m->x2, y, getField(m->x2, y + 1));
                        updateField(m->x2, y);
                    }
                }
            }
        } else { // no gravity
            // undo slide if any
            if (m->hasSlide) {
                // perform the slide in reverse
                reverseSlide(m->x1, m->y1, m->slide_x1, m->slide_y1, m->slide_x2, m->slide_y2);
            }
        }

        // replace taken tiles
        setField(m->x1, m->y1, m->tile1);
        setField(m->x2, m->y2, m->tile2);
        updateField(m->x1, m->y1);
        updateField(m->x2, m->y2);

        m_redo.prepend(m);
        emit changed();
    }
}

void Board::redo()
{
    if (canRedo()) {
        clearHighlight();
        undrawConnection();
        Move* m = m_redo.takeFirst();
        // redo the slide if any
        if (m->hasSlide) {
            Path s;
            s.append(Position(m->slide_x1, m->slide_y1));
            s.append(Position(m->slide_x2, m->slide_y2));
            performSlide(m->x1, m->y1, s);
        }
        setField(m->x1, m->y1, EMPTY);
        setField(m->x2, m->y2, EMPTY);
        updateField(m->x1, m->y1);
        updateField(m->x2, m->y2);
        gravity(true);
        m_undo.append(m);
        emit changed();
    }
}

void Board::showHint()
{
    undrawConnection();

    if (getHint_I(m_possibleMoves)) {
        m_connection = m_possibleMoves.first().path;
        drawConnection(1000);
    }
}


#ifdef DEBUGGING
void Board::makeHintMove()
{
    PossibleMoves p;

    if (getHint_I(p)) {
        m_markX = -1;
        m_markY = -1;
        marked(p.first().path.first().x, p.first().path.first().y);
        marked(p.first().path.last().x,  p.first().path.last().y);
    }
}

void Board::finish()
{
    // broken ..
    /*PossibleMoves p;
      bool ready=false;

      while(!ready && getHint_I(p))
      {
      m_markX = -1;
      m_markY = -1;
      if(tilesLeft() == 2)
      ready = true;
      marked(p.first().path.first().x, p.first().path.first().y);
      marked(p.first().path.last().x,  p.first().path.last().y);
      qApp->processEvents();
      usleep(250*1000);
      }*/
}

void Board::dumpBoard() const
{
    kDebug() << "Board contents:";
    for (int y = 0; y < y_tiles(); ++y) {
        QString row;
        for (int x = 0; x < x_tiles(); ++x) {
            int tile = getField(x, y);
            if (tile == EMPTY) {
                row += " --";
            } else {
                row += QString("%1").arg(tile, 3);
            }
        }
        kDebug() << row;
    }
}

void Board::dumpBoard(const int* board) const
{
    kDebug() << "Board contents:";
    for (int y = 0; y < y_tiles(); ++y) {
        QString row;
        for (int x = 0; x < x_tiles(); ++x) {
            int tile = board[y * x_tiles() + x];
            if (tile == EMPTY) {
                row += " --";
            } else {
                row += QString("%1").arg(tile, 3);
            }
        }
        kDebug() << row;
    }
}
#endif

int Board::lineWidth()
{
    int width = qRound(m_tiles.height() / 10.0);
    if (width < 3) {
        width = 3;
    }

    return width;
}

bool Board::getHint_I(PossibleMoves& p) const
{
    //dumpBoard();
    short done[Board::nTiles];
    for (short index = 0; index < Board::nTiles; ++index) {
        done[index] = 0;
    }

    for (int x = 0; x < x_tiles(); ++x) {
        for (int y = 0; y < y_tiles(); ++y) {
            int tile = getField(x, y);
            if (tile != EMPTY && done[tile - 1] != 4) {
                // for all these types of tile search path's
                for (int xx = 0; xx < x_tiles(); ++xx) {
                    for (int yy = 0; yy < y_tiles(); ++yy) {
                        if (xx != x || yy != y) {
                            if (tilesMatch(getField(xx, yy), tile)) {
                                if (findPath(x, y, xx, yy, p)) {
                                    //kDebug() << "path.size() ==" << p.size();
                                    //for(Path::const_iterator i = p.begin(); i != p.end(); ++i)
                                    //    kDebug() << "pathEntry: (" << i->x << "," << i->y
                                    //        << ") => " << getField(i->x, i->y);
                                    return true;
                                }
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
    if (newvalue != m_shuffle) {
        m_shuffle = newvalue;
        newGame();
    }
}

int Board::getShuffle() const
{
    return m_shuffle;
}

int Board::tilesLeft() const
{
    int left = 0;

    for (int i = 0; i < x_tiles(); ++i) {
        for (int j = 0; j < y_tiles(); ++j) {
            if (getField(i, j) != EMPTY) {
                ++left;
            }
        }
    }

    return left;
}

int Board::getCurrentTime() const
{
    return (int)difftime(time((time_t *)0), m_startTime);
}

int Board::getTimeForGame() const
{
    if (tilesLeft() == 0) {
        return m_timeForGame;
    } else {
        if (m_isPaused) {
            return (int)difftime(m_pauseStart, m_startTime);
        } else {
            return (int)difftime(time((time_t *)0), m_startTime);
        }
    }
}

bool Board::solvable(bool norestore)
{
    int *oldfield = 0;

    if (!norestore) {
        oldfield = new int [x_tiles() * y_tiles()];
        memcpy(oldfield, m_field, x_tiles() * y_tiles() * sizeof(int));
    }

    PossibleMoves p;
    while (getHint_I(p)) {
        kFatal(!tilesMatch(getField(p.first().path.first().x, p.first().path.first().y), getField(p.first().path.last().x, p.first().path.last().y)))
        << "Removing unmatched tiles: (" << p.first().path.first().x << "," << p.first().path.first().y << ") => "
        << getField(p.first().path.first().x, p.first().path.first().y) << " (" << p.first().path.last().x << "," << p.first().path.last().y << ") => "
        << getField(p.first().path.last().x, p.first().path.last().y);
        setField(p.first().path.first().x, p.first().path.first().y, EMPTY);
        setField(p.first().path.last().x, p.first().path.last().y, EMPTY);
        //if(gravityFlag())
        //{
        //  gravity(p.first().x, false);
        //  gravity(p.last().x, false);
        //}
    }

    int left = tilesLeft();

    if (!norestore) {
        memcpy(m_field, oldfield, x_tiles() * y_tiles() * sizeof(int));
        delete [] oldfield;
    }

    return left == 0;
}

bool Board::getSolvableFlag() const
{
    return m_solvableFlag;
}

void Board::setSolvableFlag(bool value)
{
    if (value && !m_solvableFlag && !solvable()) {
        m_solvableFlag = value;
        newGame();
    } else {
        m_solvableFlag = value;
    }
}

bool Board::gravityFlag() const
{
    return m_gravityFlag;
}

void Board::setGravityFlag(bool b)
{
    if (m_gravityFlag != b) {
        if (canUndo() || canRedo()) {
            newGame();
        }
        m_gravityFlag = b;
    }
}

void Board::setChineseStyleFlag(bool b)
{
    if (m_chineseStyleFlag != b) {
        // we need to force a newGame because board generation is different
        m_chineseStyleFlag = b;
        newGame();
    }
}

void Board::setTilesCanSlideFlag(bool b)
{
    if (m_tilesCanSlideFlag != b) {
        if (canUndo() || canRedo()) {
            newGame();
        }
        m_tilesCanSlideFlag = b;
    }
}

bool Board::pause()
{
    m_isPaused = !m_isPaused;
    if (m_isPaused) {
        m_pauseStart = time((time_t *)0);
    } else {
        m_startTime += (time_t)difftime(time((time_t *)0), m_pauseStart);
    }
    update();

    return m_isPaused;
}

QSize Board::sizeHint() const
{
    int dpi = logicalDpiX();
    if (dpi < 75) {
        dpi = 75;
    }
    return QSize(9*dpi, 7*dpi);
}

void Board::resetTimer()
{
    m_startTime = time((time_t *)0);
}

void Board::resetUndo()
{
    qDeleteAll(m_undo);
    m_undo.clear();
}

void Board::resetRedo()
{
    qDeleteAll(m_redo);
    m_redo.clear();
}

#include "board.moc"

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
