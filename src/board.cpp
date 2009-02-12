/***************************************************************************
 *   KShisen - A japanese game similar to mahjongg                         *
 *   Copyright 1997   Mario Weilguni <mweilguni@sime.com>                  *
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

#include "board.h"
#include "prefs.h"

#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

#include <kglobalsettings.h>
#include <klocale.h>
#include <kstandarddirs.h>

#define USE_UPDATE 1

#define EMPTY           0
#define DEFAULTDELAY    500
#define DEFAULTSHUFFLE  4
#define SEASONS_START   28
#define FLOWERS_START   39

static int sizeX[6] = {14, 16, 18, 24, 26, 30};
static int sizeY[6] = { 6,  9,  8, 12, 14, 16};
static int s_delay[5] = {1000, 750, 500, 250, 125};

bool PossibleMove::isInPath(int x, int y) const
{
    if (x == m_path.last().x && y == m_path.last().y) {
        return false;
    }
    kDebug() << "isInPath:" << x << "," << y;
    Debug();
    QList<Position>::const_iterator iter;
    // a path has at least 2 positions
    iter = m_path.constBegin();
    int pathX = iter->x;
    int pathY = iter->y;
    ++iter;
    for (; iter != m_path.constEnd(); ++iter) {
        // to fix
        if ((x == iter->x && ((y > pathY && y <= iter->y) || (y < pathY && y >= iter->y)))
                || (y == iter->y && ((x > pathX && x <= iter->x) || (x < pathX && x >= iter->x)))) {
            kDebug() << "isInPath:" << x << "," << y << "found in path" << pathX << "," << pathY << " => " << iter->x << "," << iter->y;
            return true;
        }
        pathX = iter->x;
        pathY = iter->y;
    }
    return false;
}

Board::Board(QWidget *parent) :
        QWidget(parent), m_field(0),
        m_xTiles(0), m_yTiles(0),
        m_delay(125), m_isPaused(false), m_isOver(false),
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
    setFocus();
}

Board::~Board()
{
    delete [] m_field;
}

void Board::focusOutEvent(QFocusEvent *)
{
    if (!m_isPaused && !m_isOver) {
        setPauseEnabled(true);
    }
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
    setSize(sizeX[index], sizeY[index]);
    setGravityFlag(Prefs::gravity());
    setDelay(s_delay[Prefs::speed()]);
}

bool Board::loadTileset(const QString &pathToTileset)
{
    if (m_tiles.loadTileset(pathToTileset)) {
        if (m_tiles.loadGraphics()) {
            Prefs::setTileSet(pathToTileset);
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

bool Board::loadBackground(const QString &pathToBackground)
{
    if (m_background.load(pathToBackground, width(), height())) {
        if (m_background.loadGraphics()) {
            Prefs::setBackground(pathToBackground);
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

int Board::xTiles() const
{
    return m_xTiles;
}

int Board::yTiles() const
{
    return m_yTiles;
}

void Board::setField(int x, int y, int value)
{
    if (x < 0 || y < 0 || x >= xTiles() || y >= yTiles()) {
        kFatal() << "Attempted write to invalid field position "
        "(" << x << "," << y << ")";
    }

    m_field[y * xTiles() + x] = value;
}

int Board::field(int x, int y) const
{
#ifdef DEBUGGING
    if (x < -1 || y < -1 || x > xTiles() || y > yTiles()) {
        kFatal() << "Attempted read from invalid field position "
        "(" << x << "," << y << ")";
    }
#endif

    if (x < 0 || y < 0 || x >= xTiles() || y >= yTiles()) {
        return EMPTY;
    }

    return m_field[y * xTiles() + x];
}

/**
 * @param update FIXME: What is it for?
 */
void Board::gravity(bool update)
{
    m_gravCols.clear();
    if (!m_gravityFlag) {
        return;
    }
    for (int i = 0; i < xTiles(); ++i) {
        if (gravity(i, update)) {
            m_gravCols.append(i);
        }
    }
}

/**
 * @param column The column to check
 * @param update FIXME: What is it for?
 */
bool Board::gravity(int column, bool update)
{
    bool affected = false;
    if (m_gravityFlag) {
        int rptr = yTiles() - 1;
        int wptr = yTiles() - 1;
        while (rptr >= 0) {
            if (field(column, wptr) != EMPTY) {
                --rptr;
                --wptr;
            } else {
                if (field(column, rptr) != EMPTY) {
                    setField(column, wptr, field(column, rptr));
                    setField(column, rptr, EMPTY);
                    affected = true;
                    if (update) {
                        updateField(column, rptr);
                        updateField(column, wptr);
                    }
                    --wptr;
                    --rptr;
                } else {
                    --rptr;
                }
            }
        }
    }
    return affected;
}

void Board::mousePressEvent(QMouseEvent *e)
{
    if (m_isOver) {
        newGame();
        return;
    }
    if (m_isPaused) {
        setPauseEnabled(false);
        return;
    }
    // Calculate field position
    int posX = (e->pos().x() - xOffset()) / (m_tiles.qWidth() * 2);
    int posY = (e->pos().y() - yOffset()) / (m_tiles.qHeight() * 2);

    if (e->pos().x() < xOffset() || e->pos().y() < yOffset() ||
            posX >= xTiles() || posY >= yTiles()) {
        posX = -1;
        posY = -1;
    }

    // Mark tile
    if (e->button() == Qt::LeftButton) {
        clearHighlight();

        if (posX != -1) {
            marked(posX, posY);
        }
    }

    // Assist by highlighting all tiles of same type
    if (e->button() == Qt::RightButton) {
        int clickedTile = field(posX, posY);

        // Clear marked tile
        if (m_markX != -1 && field(m_markX, m_markY) != clickedTile) {
            // We need to set m_markX and m_markY to -1 before calling
            // updateField() to ensure the tile is redrawn as unmarked.
            int oldMarkX = m_markX;
            int oldMarkY = m_markY;
            m_markX = -1;
            m_markY = -1;
            updateField(oldMarkX, oldMarkY);
        } else {
            m_markX = -1;
            m_markY = -1;
        }

        // Perform highlighting
        if (clickedTile != m_highlightedTile) {
            int oldHighlighted = m_highlightedTile;
            m_highlightedTile = clickedTile;
            for (int i = 0; i < xTiles(); ++i) {
                for (int j = 0; j < yTiles(); ++j) {
                    const int fieldTile = field(i, j);
                    if (fieldTile != EMPTY) {
                        if (fieldTile == oldHighlighted) {
                            updateField(i, j);
                        } else if (fieldTile == clickedTile) {
                            updateField(i, j);
                        } else if (m_chineseStyleFlag) {
                            if (clickedTile >= SEASONS_START && clickedTile <= (SEASONS_START + 3) && fieldTile >= SEASONS_START && fieldTile <= (SEASONS_START + 3)) {
                                updateField(i, j);
                            } else if (clickedTile >= FLOWERS_START && clickedTile <= (FLOWERS_START + 3) && fieldTile >= FLOWERS_START && fieldTile <= (FLOWERS_START + 3)) {
                                updateField(i, j);
                            }
                            // oldHighlighted
                            if (oldHighlighted >= SEASONS_START && oldHighlighted <= (SEASONS_START + 3) && fieldTile >= SEASONS_START && fieldTile <= (SEASONS_START + 3)) {
                                updateField(i, j);
                            } else if (oldHighlighted >= FLOWERS_START && oldHighlighted <= (FLOWERS_START + 3) && fieldTile >= FLOWERS_START && fieldTile <= (FLOWERS_START + 3)) {
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
int Board::xOffset() const
{
    int tw = m_tiles.qWidth() * 2;
    return (width() - (tw * xTiles())) / 2;
}

int Board::yOffset() const
{
    int th = m_tiles.qHeight() * 2;
    return (height() - (th * yTiles())) / 2;
}

void Board::setSize(int x, int y)
{
    if (x == xTiles() && y == yTiles()) {
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
    //int w = qRound(m_tiles.unscaledTileWidth() * MINIMUM_SCALE) * xTiles();
    //int h = qRound(m_tiles.unscaledTileHeight() * MINIMUM_SCALE) * yTiles();
    int w = qRound(m_tiles.qWidth() * 2.0 * MINIMUM_SCALE) * xTiles();
    int h = qRound(m_tiles.qHeight() * 2.0 * MINIMUM_SCALE) * yTiles();
    w += m_tiles.width();
    h += m_tiles.width();

    setMinimumSize(w, h);

    resizeBoard();
    newGame();
    emit changed();
}

void Board::resizeEvent(QResizeEvent *e)
{
    kDebug() << "[resizeEvent]";
    if (e->spontaneous()) {
        kDebug() << "[resizeEvent] spontaneous";
    }
    resizeBoard();
    emit resized();
}

void Board::resizeBoard()
{
    // calculate tile size required to fit all tiles in the window
    QSize newsize = m_tiles.preferredTileSize(QSize(width(), height()), xTiles(), yTiles());
    m_tiles.reloadTileset(newsize);
    //recalculate bg, if needed
    m_background.sizeChanged(width(), height());
    //reload our bg brush, using the cache in libkmahjongg if possible
    QPalette palette;
    palette.setBrush(backgroundRole(), m_background.getBackground());
    setPalette(palette);
}

QSize Board::unscaledSize() const
{
    int w = m_tiles.qWidth() * 2 * xTiles() + m_tiles.width();
    int h = m_tiles.qHeight() * 2 * yTiles() + m_tiles.width();
    return QSize(w, h);
}

void Board::newGame()
{
    kDebug() << "NewGame";

    m_isOver = false;
    m_isPaused = false;

    m_markX = -1;
    m_markY = -1;
    m_highlightedTile = -1; // will clear previous highlight

    resetUndo();
    resetRedo();
    m_connection.clear();
    m_possibleMoves.clear();

    // distribute all tiles on board
    int curTile = 1;
    int tileCount = 0;
    /*
     * Note by jwickers: i changed the way to distribute tiles
     *  in chinese mahjongg there are 4 tiles of each
     *  except flowers and seasons (4 flowers and 4 seasons,
     *  but one unique tile of each, that is why they are
     *  the only ones numbered)
     * That uses the chineseStyle flag
     */
    for (int y = 0; y < yTiles(); ++y) {
        for (int x = 0; x < xTiles(); ++x) {
            // do not duplicate flowers or seasons
            if (!m_chineseStyleFlag || !((curTile >= SEASONS_START && curTile <= (SEASONS_START + 3)) || (curTile >= FLOWERS_START && curTile <= (FLOWERS_START + 3)))) {
                setField(x, y, curTile);
                if (++tileCount >= 4) {
                    tileCount = 0;
                    ++curTile;
                }
            } else {
                tileCount = 0;
                setField(x, y, curTile++);
            }
            if (curTile > Board::nTiles) {
                curTile = 1;
            }
        }
    }

    if (shuffle() == 0) {
        update();
        resetTimer();
        emit changed();
        return;
    }

    // shuffle the field
    int tx = xTiles();
    int ty = yTiles();
    for (int i = 0; i < xTiles() * yTiles() * shuffle(); ++i) {
        int x1 = m_random.getLong(tx);
        int y1 = m_random.getLong(ty);
        int x2 = m_random.getLong(tx);
        int y2 = m_random.getLong(ty);
        int t  = field(x1, y1);
        setField(x1, y1, field(x2, y2));
        setField(x2, y2, t);
    }

    // do not make solvable if m_solvableFlag is false
    if (!m_solvableFlag) {
        update();
        resetTimer();
        emit changed();
        return;
    }


    int fsize = xTiles() * yTiles() * sizeof(int);
    int *oldfield = new int[xTiles() * yTiles()];
    memcpy(oldfield, m_field, fsize);   // save field
    int *tiles = new int[xTiles() * yTiles()];
    int *pos = new int[xTiles() * yTiles()];
    //jwickers: in case the game cannot make the game solvable we do not want to run an infinite loop
    int max_attempts = 200;

    while (!solvable(true) && max_attempts > 0) {
        //kDebug() << "Not solvable";
        //dumpBoard();

        // generate a list of free tiles and positions
        int num_tiles = 0;
        for (int i = 0; i < xTiles() * yTiles(); ++i) {
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
            --num_tiles;

            // put this tile on the new position
            m_field[apos] = tile;
        }

        // remember field
        memcpy(oldfield, m_field, fsize);
        --max_attempts;
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

    if (tilesMatch(m_highlightedTile, field(x, y))) {
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
        p.drawText(rect(), Qt::AlignCenter, i18n("Game Paused\nClick to resume game."));
    } else if (m_isOver) {
        p.setFont(KGlobalSettings::largeFont());
        p.drawText(rect(), Qt::AlignCenter, i18n("Game Over\nClick to start a new game."));
    } else {
        int w = m_tiles.width();
        int h = m_tiles.height();
        int fw = m_tiles.qWidth() * 2;
        int fh = m_tiles.qHeight() * 2;
        for (int i = 0; i < xTiles(); ++i) {
            for (int j = 0; j < yTiles(); ++j) {
                int tile = field(i, j);
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
        for (QList<PossibleMove>::iterator iter = m_possibleMoves.begin(); iter != m_possibleMoves.end(); ++iter) {
            // Path.size() will always be >= 2
            Path::const_iterator pathEnd = iter->m_path.constEnd();
            Path::const_iterator pt1 = iter->m_path.constBegin();
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

void Board::reverseSlide(int x, int y, int slideX1, int slideY1, int slideX2, int slideY2)
{
    //kDebug() << "reverseSlide" << x << " " << y;

    // s_x2 is the current location of the last tile to slide
    // s_x1 is its destination
    // calculate the offset for the tiles to slide
    int dx = slideX1 - slideX2;
    int dy = slideY1 - slideY2;
    /*kDebug() << "reverseSlide last to slide is x=" << s_x2 << ", y=" << s_y2;
      kDebug() << "reverseSlide slide to x=" << s_x1 << ", y=" << s_y1;
      kDebug() << "reverseSlide offset dx=" << dx << ", dy=" << dy;*/
    int current_tile;
    // move all tiles between s_x2, s_y2 and x, y to slide with that offset
    if (dx == 0) {
        if (y < slideY2) {
            for (int i = y + 1; i <= slideY2; ++i) {
                current_tile = field(x, i);
                if (current_tile == EMPTY) {
                    continue;
                }
                setField(x, i, EMPTY);
                setField(x, i + dy, current_tile);
                updateField(x, i);
                updateField(x, i + dy);
            }
        } else {
            for (int i = y - 1; i >= slideY2; --i) {
                current_tile = field(x, i);
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
        if (x < slideX2) {
            for (int i = x + 1; i <= slideX2; ++i) {
                current_tile = field(i, y);
                if (current_tile == EMPTY) {
                    continue;
                }
                setField(i, y, EMPTY);
                setField(i + dx, y, current_tile);
                updateField(i, y);
                updateField(i + dx, y);
            }
        } else {
            for (int i = x - 1; i >= slideX2; --i) {
                current_tile = field(i, y);
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

void Board::performSlide(int x, int y, Path &slide)
{
    //kDebug() << "performSlide" << x << " " << y;

    // check if there is something to slide
    if (slide.isEmpty()) {
        return;
    }

    // slide.first is the current location of the last tile to slide
    // slide.last is its destination
    // calculate the offset for the tiles to slide
    int dx = slide.last().x - slide.first().x;
    int dy = slide.last().y - slide.first().y;
    /*kDebug() << "performSlide last to slide is x=" << s.first().x << ", y=" << s.first().y;
      kDebug() << "performSlide slide to x=" << s.last().x << ", y=" << s.last().y;
      kDebug() << "performSlide offset dx=" << dx << ", dy=" << dy;*/
    int current_tile;
    // move all tiles between m_markX, m_markY and the last tile to slide with that offset
    if (dx == 0) {
        if (y < slide.first().y) {
            for (int i = slide.first().y; i > y; --i) {
                current_tile = field(x, i);
                setField(x, i, EMPTY);
                setField(x, i + dy, current_tile);
                updateField(x, i);
                updateField(x, i + dy);
            }
        } else {
            for (int i = slide.first().y; i < y; ++i) {
                current_tile = field(x, i);
                setField(x, i, EMPTY);
                setField(x, i + dy, current_tile);
                updateField(x, i);
                updateField(x, i + dy);
            }
        }
    } else if (dy == 0) {
        if (x < slide.first().x) {
            for (int i = slide.first().x; i > x; --i) {
                current_tile = field(i, y);
                setField(i, y, EMPTY);
                setField(i + dx, y, current_tile);
                updateField(i, y);
                updateField(i + dx, y);
            }
        } else {
            for (int i = slide.first().x; i < x; ++i) {
                current_tile = field(i, y);
                setField(i, y, EMPTY);
                setField(i + dx, y, current_tile);
                updateField(i, y);
                updateField(i + dx, y);
            }
        }
    }
}

void Board::performMove(PossibleMove &possibleMoves)
{
    m_connection = possibleMoves.m_path;
#ifdef DEBUGGING
    // DEBUG undo, save board state
    int fsize = xTiles() * yTiles() * sizeof(int);
    int *saved1 = new int[xTiles() * yTiles()];
    memcpy(saved1, m_field, fsize);
#endif
    // if the tiles can slide, we have to update the slided tiles too
    // and store the slide in a Move
    if (possibleMoves.m_hasSlide) {
        performSlide(m_markX, m_markY, possibleMoves.m_slide);
        madeMoveWithSlide(m_markX, m_markY, possibleMoves.m_path.last().x, possibleMoves.m_path.last().y, possibleMoves.m_slide);
    } else {
        madeMove(m_markX, m_markY, possibleMoves.m_path.last().x, possibleMoves.m_path.last().y);
    }
    undrawPossibleMoves();
    drawConnection(delay());
    m_tileRemove1 = QPair<int, int>(m_markX, m_markY);
    m_tileRemove2 = QPair<int, int>(possibleMoves.m_path.last().x, possibleMoves.m_path.last().y);
    m_markX = -1;
    m_markY = -1;
    m_possibleMoves.clear();
#ifdef DEBUGGING
    // DEBUG undo, force gravity
    undrawConnection();
    // DEBUG undo, save board2 state
    int *saved2 = new int[xTiles() * yTiles()];
    int *saved3 = new int[xTiles() * yTiles()]; // after undo
    int *saved4 = new int[xTiles() * yTiles()]; // after redo
    memcpy(saved2, m_field, fsize);
    // DEBUG undo, undo move
    bool errorFound = false;
    if (canUndo()) {
        undo();
        // DEBUG undo, compare to saved board state
        for (int i = 0; i < xTiles() * yTiles(); ++i) {
            if (saved1[i] != m_field[i]) {
                kDebug() << "[DEBUG Undo 1], tile (" << i << ") was" << saved1[i] << "before more, it is" << m_field[i] << "after undo.";
                errorFound = true;
            }
        }
        // DEBUG undo, save board state
        memcpy(saved3, m_field, fsize);
        // DEBUG undo, redo
        if (canRedo()) {
            redo();
            undrawConnection();
            // DEBUG undo, compare to saved board2 state
            for (int i = 0; i < xTiles() * yTiles(); ++i) {
                if (saved2[i] != m_field[i]) {
                    kDebug() << "[DEBUG Undo 2], tile (" << i << ") was" << saved2[i] << "after more, it is" << m_field[i] << "after redo.";
                    errorFound = true;
                }
            }
            // DEBUG undo, save board state
            memcpy(saved4, m_field, fsize);
        }
    }
    // dumpBoard on error
    if (errorFound) {
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
        if (field(x, y) == EMPTY) {
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

        for (QList<PossibleMove>::iterator iter = m_possibleMoves.begin(); iter != m_possibleMoves.end(); ++iter) {
            if (iter->isInPath(x, y)) {
                performMove(*iter);
                emit selectATile();
                return;
            }
        }
    }
    if (field(x, y) == EMPTY) {
        return;
    }

    int field1 = field(m_markX, m_markY);
    int field2 = field(x, y);

    // both field match
    if (!tilesMatch(field1, field2)) {
        emit tilesDontMatch();
        return;
    }

    // trace and perform the move and get the list of possible moves
    if (findPath(m_markX, m_markY, x, y, m_possibleMoves)) {
        if (m_possibleMoves.count() > 1) {
            //kDebug() << "marked: there was" << m_possibleMoves.count() << "moves possible for this";
            int withSlide = 0;
            for (QList<PossibleMove>::const_iterator iter = m_possibleMoves.constBegin(); iter != m_possibleMoves.constEnd(); ++iter) {
                iter->Debug();
                if (iter->m_hasSlide) {
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

        for (int i = 0; i < xTiles(); ++i) {
            for (int j = 0; j < yTiles(); ++j) {
                if (tilesMatch(oldHighlighted, field(i, j))) {
                    updateField(i, j);
                }
            }
        }
    }
}

/**
 * @param x1 x coordinate of the first tile
 * @param y1 y coordinate of the first tile
 * @param x2 x coordinate of the second tile
 * @param y2 y coordinate of the second tile
 */
bool Board::canMakePath(int x1, int y1, int x2, int y2) const
{
    if (x1 == x2) {
        for (int i = qMin(y1, y2) + 1; i < qMax(y1, y2); ++i) {
            if (field(x1, i) != EMPTY) {
                return false;
            }
        }
        return true;
    }

    if (y1 == y2) {
        for (int i = qMin(x1, x2) + 1; i < qMax(x1, x2); ++i) {
            if (field(i, y1) != EMPTY) {
                return false;
            }
        }
        return true;
    }

    return false;
}

/**
 * @param x1 x coordinate of the slide's initial position
 * @param y1 y coordinate of the slide's initial position
 * @param x2 x coordinate of the slide's final position
 * @param y2 y coordinate of the slide's final position
 * @param path The movement of the last tile slided will be stored in the path
 */
bool Board::canSlideTiles(int x1, int y1, int x2, int y2, Path &path) const
{
    int distance = -1;
    path.clear();
    if (x1 == x2) {
        if (y1 > y2) {
            distance = y1 - y2;
            // count how much free space we have for sliding
            int start_free = -1;
            int end_free = -1;
            // find first tile empty
            for (int i = y1 - 1; i >= 0; --i) {
                if (field(x1, i) == EMPTY) {
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
            for (int i = start_free - 1; i >= 0; --i) {
                if (field(x1, i) != EMPTY) {
                    end_free = i;
                    break;
                }
            }
            // if not found, it is the border: 0

            // so we can slide of start_free - end_free, compare this to the distance
            //kDebug() << "canSlideTiles distance=" << distance << "free=" << (start_free - end_free);
            if (distance <= (start_free - end_free)) {
                // first position of the last slided tile
                path.append(Position(x1, start_free + 1));
                // final position of the last slided tile
                path.append(Position(x1, start_free + 1 - distance));
                return true;
            } else {
                return false;
            }
        } else if (y2 > y1) {
            distance = y2 - y1;
            // count how much free space we have for sliding
            int start_free = -1;
            int end_free = yTiles();
            // find first tile empty
            for (int i = y1 + 1; i < yTiles(); ++i) {
                if (field(x1, i) == EMPTY) {
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
            for (int i = start_free + 1; i < yTiles(); ++i) {
                if (field(x1, i) != EMPTY) {
                    end_free = i;
                    break;
                }
            }
            // if not found, it is the border: yTiles()-1

            // so we can slide of end_free - start_free, compare this to the distance
            //kDebug() << "canSlideTiles distance=" << distance << "free=" << (end_free - start_free);
            if (distance <= (end_free - start_free)) {
                // first position of the last slided tile
                path.append(Position(x1, start_free - 1));
                // final position of the last slided tile
                path.append(Position(x1, start_free - 1 + distance));
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
            for (int i = x1 - 1; i >= 0; --i) {
                if (field(i, y1) == EMPTY) {
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
            for (int i = start_free - 1; i >= 0; --i) {
                if (field(i, y1) != EMPTY) {
                    end_free = i;
                    break;
                }
            }
            // if not found, it is the border: 0

            // so we can slide of start_free - end_free, compare this to the distance
            //kDebug() << "canSlideTiles distance=" << distance << "free=" << (start_free - end_free);
            if (distance <= (start_free - end_free)) {
                // first position of the last slided tile
                path.append(Position(start_free + 1, y1));
                // final position of the last slided tile
                path.append(Position(start_free + 1 - distance, y1));
                return true;
            } else {
                return false;
            }
        } else if (x2 > x1) {
            distance = x2 - x1;
            // count how much free space we have for sliding
            int start_free = -1;
            int end_free = xTiles();
            // find first tile empty
            for (int i = x1 + 1; i < xTiles(); ++i) {
                if (field(i, y1) == EMPTY) {
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
            for (int i = start_free + 1; i < xTiles(); ++i) {
                if (field(i, y1) != EMPTY) {
                    end_free = i;
                    break;
                }
            }
            // if not found, it is the border: xTiles()-1

            // so we can slide of end_free - start_free, compare this to the distance
            //kDebug() << "canSlideTiles distance=" << distance << "free=" << (end_free - start_free);
            if (distance <= (end_free - start_free)) {
                // first position of the last slided tile
                path.append(Position(start_free - 1, y1));
                // final position of the last slided tile
                path.append(Position(start_free - 1 + distance, y1));
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

/**
* @param x1 x coordinate of the first tile
* @param y1 y coordinate of the first tile
* @param x2 x coordinate of the second tile
* @param y2 y coordinate of the second tile
* @param possibleMoves All the possible moves are stored here
* @return The number of paths found
*/
int Board::findPath(int x1, int y1, int x2, int y2, PossibleMoves &possibleMoves) const
{
    possibleMoves.clear();

    int numberOfPaths = 0;
    int simplePath = 0;

    // first find the simple paths
    numberOfPaths = findSimplePath(x1, y1, x2, y2, possibleMoves);

    // if the tiles can slide, 2 lines max is allowed
    if (m_tilesCanSlideFlag) {
        return numberOfPaths;
    }

    // Find paths of 3 segments
    const int dx[4] = { 1, 0, -1, 0 };
    const int dy[4] = { 0, 1, 0, -1 };

    for (int i = 0; i < 4; ++i) {
        int newX = x1 + dx[i];
        int newY = y1 + dy[i];
        while (newX >= -1 && newX <= xTiles() &&
                newY >= -1 && newY <= yTiles() &&
                field(newX, newY) == EMPTY) {
            if ((simplePath = findSimplePath(newX, newY, x2, y2, possibleMoves))) {
                possibleMoves.last().m_path.prepend(Position(x1, y1));
                numberOfPaths += simplePath;
            }
            newX += dx[i];
            newY += dy[i];
        }
    }
    return numberOfPaths;
}

/**
 * @param x1 x coordinate of the first tile
 * @param y1 y coordinate of the first tile
 * @param x2 x coordinate of the second tile
 * @param y2 y coordinate of the second tile
 * @param possibleMoves All the possible moves are stored here
 * @return The number of paths found
 */
int Board::findSimplePath(int x1, int y1, int x2, int y2, PossibleMoves &possibleMoves) const
{
    int numberOfPaths = 0;
    Path path;
    // Find direct line (path of 1 segment)
    if (canMakePath(x1, y1, x2, y2)) {
        path.append(Position(x1, y1));
        path.append(Position(x2, y2));
        possibleMoves.append(PossibleMove(path));
        ++numberOfPaths;
    }

    // If the tiles are in the same row or column, then a
    // a 'simple path' cannot be found between them
    // That is, canMakePath should have returned true above if
    // that was possible
    if (x1 == x2 || y1 == y2) {
        return numberOfPaths;
    }

    // I isolate the special code when tiles can slide even if it duplicates code for now
    // Can we make a path sliding tiles ?, the slide move is always first, then a normal path
    if (m_tilesCanSlideFlag) {
        Path slidePath;
        // Find path of 2 segments (route A)
        if (canSlideTiles(x1, y1, x2, y1, slidePath) && canMakePath(x2, y1, x2, y2)) {
            path.clear();
            path.append(Position(x1, y1));
            path.append(Position(x2, y1));
            path.append(Position(x2, y2));
            possibleMoves.append(PossibleMove(path, slidePath));
            ++numberOfPaths;
        }

        // Find path of 2 segments (route B)
        if (canSlideTiles(x1, y1, x1, y2, slidePath) && canMakePath(x1, y2, x2, y2)) {
            path.clear();
            path.append(Position(x1, y1));
            path.append(Position(x1, y2));
            path.append(Position(x2, y2));
            possibleMoves.append(PossibleMove(path, slidePath));
            ++numberOfPaths;
        }
    }

    // Even if tiles can slide, a path could still be done without sliding

    // Find path of 2 segments (route A)
    if (field(x2, y1) == EMPTY && canMakePath(x1, y1, x2, y1) &&
            canMakePath(x2, y1, x2, y2)) {
        path.clear();
        path.append(Position(x1, y1));
        path.append(Position(x2, y1));
        path.append(Position(x2, y2));
        possibleMoves.append(PossibleMove(path));
        ++numberOfPaths;
    }

    // Find path of 2 segments (route B)
    if (field(x1, y2) == EMPTY && canMakePath(x1, y1, x1, y2) &&
            canMakePath(x1, y2, x2, y2)) {
        path.clear();
        path.append(Position(x1, y1));
        path.append(Position(x1, y2));
        path.append(Position(x2, y2));
        possibleMoves.append(PossibleMove(path));
        ++numberOfPaths;
    }

    return numberOfPaths;
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
    if (!hint_I(dummyPossibleMoves)) {
        m_timeForGame = currentTime();
        emit endOfGame();
    }
}

QPoint Board::midCoord(int x, int y) const
{
    QPoint p;
    int w = m_tiles.qWidth() * 2;
    int h = m_tiles.qHeight() * 2;

    if (x == -1) {
        p.setX(xOffset() - (w / 4));
    } else if (x == xTiles()) {
        p.setX(xOffset() + (w * xTiles()) + (w / 4));
    } else {
        p.setX(xOffset() + (w * x) + (w / 2));
    }

    if (y == -1) {
        p.setY(yOffset() - (w / 4));
    } else if (y == yTiles()) {
        p.setY(yOffset() + (h * yTiles()) + (w / 4));
    } else {
        p.setY(yOffset() + (h * y) + (h / 2));
    }

    return p;
}

void Board::setDelay(int newValue)
{
    m_delay = newValue;
}

int Board::delay() const
{
    return m_delay;
}

void Board::madeMove(int x1, int y1, int x2, int y2)
{
    Path slide;
    slide.clear();
    madeMoveWithSlide(x1, y1, x2, y2, slide);
}

void Board::madeMoveWithSlide(int x1, int y1, int x2, int y2, Path &slide)
{
    Move *move;
    if (slide.empty()) {
        move = new Move(x1, y1, x2, y2, field(x1, y1), field(x2, y2));
    } else {
        move = new Move(x1, y1, x2, y2, field(x1, y1), field(x2, y2), slide.first().x, slide.first().y, slide.last().x, slide.last().y);
    }
    m_undo.append(move);
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
        Move *move = m_undo.takeLast();
        if (gravityFlag()) {
            int y;

            // When both tiles reside in the same column, the order of undo is
            // significant (we must undo the lower tile first).
            // Also in that case there cannot be a slide
            if (move->m_x1 == move->m_x2 && move->m_y1 < move->m_y2) {
                qSwap(move->m_x1, move->m_x2);
                qSwap(move->m_y1, move->m_y2);
                qSwap(move->m_tile1, move->m_tile2);
            }

            // if there is no slide, keep previous implementation: move both column up
            if (!move->m_hasSlide) {
#ifdef DEBUGGING
                kDebug() << "[undo] gravity from a no slide move";
#endif
                // move tiles from the first column up
                for (y = 0; y < move->m_y1; ++y) {
                    setField(move->m_x1, y, field(move->m_x1, y + 1));
                    updateField(move->m_x1, y);
                }

                // move tiles from the second column up
                for (y = 0; y < move->m_y2; ++y) {
                    setField(move->m_x2, y, field(move->m_x2, y + 1));
                    updateField(move->m_x2, y);
                }
            } else { // else check all tiles from the slide that may have fallen down
#ifdef DEBUGGING
                kDebug() << "[undo] gravity from slide s1(" << move->slide_x1 << "," << move->slide_y1 << ")=>s2(" << move->slide_x2 << "," << move->slide_y2 << ") matching (" << move->x1 << "," << move->y1 << ")=>(" << move->x2 << "," << move->y2 << ")";
#endif
                // horizontal slide
                // because tiles that slides horizontaly may fall down
                // in columns different than the taken tiles columns
                // we need to take them back up then undo the slide
                if (move->m_slideY1 == move->m_slideY2) {
#ifdef DEBUGGING
                    kDebug() << "[undo] gravity from horizontal slide";
#endif
                    // last slide tile went from slide_x1 -> slide_x2
                    // the number of slided tiles is n = abs(x1 - slide_x1)
                    int n = move->m_x1 - move->m_slideX1;
                    if (n < 0) {
                        n = -n;
                    }
                    // distance slided is
                    int dx = move->m_slideX2 - move->m_slideX1;
                    if (dx < 0) {
                        dx = -dx;
                    }
#ifdef DEBUGGING
                    kDebug() << "[undo] n =" << n;
#endif
                    // slided tiles may fall down after the slide
                    // so any tiles on top of the columns between
                    // slide_x2 -> slide_x2 +/- n (excluded) should go up to slide_y1
                    if (move->m_slideX2 > move->m_slideX1) {  // slide to the right
#ifdef DEBUGGING
                        kDebug() << "[undo] slide right";
#endif
                        for (int i = move->m_slideX2; i > move->m_slideX2 - n; --i) {
                            // find top tile
                            int j;
                            for (j = 0; j < yTiles(); ++j) {
                                if (field(i, j) != EMPTY) {
                                    break;
                                }
                            }

                            // ignore if the tile did not fall
                            if (j <= move->m_slideY1) {
                                continue;
                            }
#ifdef DEBUGGING
                            kDebug() << "[undo] moving (" << i << "," << j << ") up to (" << i << "," << move->slide_y1 << ")";
#endif
                            // put it back up
                            setField(i, move->m_slideY1, field(i, j));
                            setField(i, j, EMPTY);
                            updateField(i, j);
                            updateField(i, move->m_slideY1);
                        }
                    } else { // slide to the left
#ifdef DEBUGGING
                        kDebug() << "[undo] slide left";
#endif
                        for (int i = move->m_slideX2; i < move->m_slideX2 + n; ++i) {
                            // find top tile
                            int j;
                            for (j = 0; j < yTiles(); ++j) {
                                if (field(i, j) != EMPTY) {
                                    break;
                                }
                            }

                            // ignore if the tile did not fall
                            if (j <= move->m_slideY1) {
                                continue;
                            }
#ifdef DEBUGGING
                            kDebug() << "[undo] moving (" << i << "," << j << ") up to (" << i << "," << move->slide_y1 << ")";
#endif
                            // put it back up
                            setField(i, move->m_slideY1, field(i, j));
                            setField(i, j, EMPTY);
                            updateField(i, j);
                            updateField(i, move->m_slideY1);
                        }
                    }
                    // move tiles from the second column up
#ifdef DEBUGGING
                    kDebug() << "[undo] moving up column x2" << m->x2;
#endif
                    for (y = 0; y <= move->m_y2; ++y) {
#ifdef DEBUGGING
                        kDebug() << "[undo] moving up tile" << y + 1;
#endif
                        setField(move->m_x2, y, field(move->m_x2, y + 1));
                        updateField(move->m_x2, y);
                    }
                    // and all columns that fell after the tiles slided between
                    // only if they were not replaced by a sliding tile !!
                    // x1 -> x1+dx should go up one
                    // if their height > slide_y1
                    // because they have fallen after the slide
                    if (move->m_slideX2 > move->m_slideX1) {  // slide to the right
                        if (move->m_slideY1 > 0) {
                            for (int i = move->m_x1 + dx; i >= move->m_x1; --i) {
#ifdef DEBUGGING
                                kDebug() << "[undo] moving up column" << i << "until" << move->slide_y1;
#endif
                                for (int j = 0; j < move->m_slideY1; ++j) {
#ifdef DEBUGGING
                                    kDebug() << "[undo] moving up tile" << j + 1;
#endif
                                    setField(i, j, field(i, j + 1));
                                    updateField(i, j);
                                }
#ifdef DEBUGGING
                                kDebug() << "[undo] clearing last tile" << move->slide_y1;
#endif
                                setField(i, move->m_slideY1, EMPTY);
                                updateField(i, move->m_slideY1);
                            }
                        }
                    } else { // slide to the left
                        if (move->m_slideY1 > 0) {
                            for (int i = move->m_x1 - dx; i <= move->m_x1; ++i) {
#ifdef DEBUGGING
                                kDebug() << "[undo] moving up column" << i << "until" << move->slide_y1;
#endif
                                for (int j = 0; j < move->m_slideY1; ++j) {
#ifdef DEBUGGING
                                    kDebug() << "[undo] moving up tile" << j + 1;
#endif
                                    setField(i, j, field(i, j + 1));
                                    updateField(i, j);
                                }
#ifdef DEBUGGING
                                kDebug() << "[undo] clearing last tile" << move->slide_y1;
#endif
                                setField(i, move->m_slideY1, EMPTY);
                                updateField(i, move->m_slideY1);
                            }
                        }
                    }

                    // then undo the slide to put the tiles back to their original location
#ifdef DEBUGGING
                    kDebug() << "[undo] reversing slide";
#endif
                    reverseSlide(move->m_x1, move->m_y1, move->m_slideX1, move->m_slideY1, move->m_slideX2, move->m_slideY2);

                } else {
                    // vertical slide, in fact nothing special is necessary
                    // the default implementation works because it only affects
                    // the two columns were tiles were taken
#ifdef DEBUGGING
                    kDebug() << "[undo] gravity from vertical slide";
#endif

                    // move tiles from the first column up
                    for (y = 0; y < move->m_y1; ++y) {
                        setField(move->m_x1, y, field(move->m_x1, y + 1));
                        updateField(move->m_x1, y);
                    }

                    // move tiles from the second column up
                    for (y = 0; y < move->m_y2; ++y) {
                        setField(move->m_x2, y, field(move->m_x2, y + 1));
                        updateField(move->m_x2, y);
                    }
                }
            }
        } else { // no gravity
            // undo slide if any
            if (move->m_hasSlide) {
                // perform the slide in reverse
                reverseSlide(move->m_x1, move->m_y1, move->m_slideX1, move->m_slideY1, move->m_slideX2, move->m_slideY2);
            }
        }

        // replace taken tiles
        setField(move->m_x1, move->m_y1, move->m_tile1);
        setField(move->m_x2, move->m_y2, move->m_tile2);
        updateField(move->m_x1, move->m_y1);
        updateField(move->m_x2, move->m_y2);

        m_redo.prepend(move);
        emit changed();
    }
}

void Board::redo()
{
    if (canRedo()) {
        clearHighlight();
        undrawConnection();
        Move *move = m_redo.takeFirst();
        // redo the slide if any
        if (move->m_hasSlide) {
            Path s;
            s.append(Position(move->m_slideX1, move->m_slideY1));
            s.append(Position(move->m_slideX2, move->m_slideY2));
            performSlide(move->m_x1, move->m_y1, s);
        }
        setField(move->m_x1, move->m_y1, EMPTY);
        setField(move->m_x2, move->m_y2, EMPTY);
        updateField(move->m_x1, move->m_y1);
        updateField(move->m_x2, move->m_y2);
        gravity(true);
        m_undo.append(move);
        emit changed();
    }
}

void Board::showHint()
{
    undrawConnection();

    if (hint_I(m_possibleMoves)) {
        m_connection = m_possibleMoves.first().m_path;
        drawConnection(1000);
    }
}


#ifdef DEBUGGING
void Board::makeHintMove()
{
    PossibleMoves possibleMoves;

    if (getHint_I(p)) {
        m_markX = -1;
        m_markY = -1;
        marked(possibleMoves.first().path.first().x, possibleMoves.first().path.first().y);
        marked(possibleMoves.first().path.last().x,  possibleMoves.first().path.last().y);
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
    for (int y = 0; y < yTiles(); ++y) {
        QString row;
        for (int x = 0; x < xTiles(); ++x) {
            if (getField(x, y) == EMPTY) {
                row += " --";
            } else {
                row += QString("%1").arg(tile, 3);
            }
        }
        kDebug() << row;
    }
}

void Board::dumpBoard(const int *board) const
{
    kDebug() << "Board contents:";
    for (int y = 0; y < yTiles(); ++y) {
        QString row;
        for (int x = 0; x < xTiles(); ++x) {
            if (board[y * xTiles() + x] == EMPTY) {
                row += " --";
            } else {
                row += QString("%1").arg(tile, 3);
            }
        }
        kDebug() << row;
    }
}
#endif

/**
 * The line width should be relative to the tile size, however, if the tile size is too small, keep a minimum line width.
 */
int Board::lineWidth() const
{
    int width = qRound(m_tiles.height() / 10.0);
    if (width < 3) {
        width = 3;
    }

    return width;
}

bool Board::hint_I(PossibleMoves &possibleMoves) const
{
    //dumpBoard();
    short done[Board::nTiles];
    for (short i = 0; i < Board::nTiles; ++i) {
        done[i] = 0;
    }

    for (int x = 0; x < xTiles(); ++x) {
        for (int y = 0; y < yTiles(); ++y) {
            int tile = field(x, y);
            if (tile != EMPTY && done[tile - 1] != 4) {
                // for all these types of tile search path's
                for (int xx = 0; xx < xTiles(); ++xx) {
                    for (int yy = 0; yy < yTiles(); ++yy) {
                        if (xx != x || yy != y) {
                            if (tilesMatch(field(xx, yy), tile)) {
                                if (findPath(x, y, xx, yy, possibleMoves)) {
                                    //kDebug() << "path.size() ==" << p.size();
                                    //for(Path::const_iterator i = p.constBegin(); i != p.constEnd(); ++i)
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

void Board::setShuffle(int newValue)
{
    if (newValue != m_shuffle) {
        m_shuffle = newValue;
        newGame();
    }
}

int Board::shuffle() const
{
    return m_shuffle;
}

int Board::tilesLeft() const
{
    int left = 0;

    for (int i = 0; i < xTiles(); ++i) {
        for (int j = 0; j < yTiles(); ++j) {
            if (field(i, j) != EMPTY) {
                ++left;
            }
        }
    }

    return left;
}

int Board::currentTime() const
{
    return static_cast<int>(difftime(time(NULL), m_startTime));
}

int Board::timeForGame() const
{
    if (tilesLeft() == 0) {
        return m_timeForGame;
    } else {
        if (m_isPaused) {
            return static_cast<int>(difftime(m_pauseStart, m_startTime));
        } else {
            return currentTime();
        }
    }
}

bool Board::solvable(bool noRestore)
{
    int *oldField = 0;

    if (!noRestore) {
        oldField = new int [xTiles() * yTiles()];
        memcpy(oldField, m_field, xTiles() * yTiles() * sizeof(int));
    }

    PossibleMoves p;
    while (hint_I(p)) {
        kFatal(!tilesMatch(field(p.first().m_path.first().x, p.first().m_path.first().y), field(p.first().m_path.last().x, p.first().m_path.last().y)))
        << "Removing unmatched tiles: (" << p.first().m_path.first().x << "," << p.first().m_path.first().y << ") => "
        << field(p.first().m_path.first().x, p.first().m_path.first().y) << " (" << p.first().m_path.last().x << "," << p.first().m_path.last().y << ") => "
        << field(p.first().m_path.last().x, p.first().m_path.last().y);
        setField(p.first().m_path.first().x, p.first().m_path.first().y, EMPTY);
        setField(p.first().m_path.last().x, p.first().m_path.last().y, EMPTY);
        //if(gravityFlag())
        //{
        //  gravity(p.first().x, false);
        //  gravity(p.last().x, false);
        //}
    }

    int left = tilesLeft();

    if (!noRestore) {
        memcpy(m_field, oldField, xTiles() * yTiles() * sizeof(int));
        delete [] oldField;
    }

    return left == 0;
}

bool Board::solvableFlag() const
{
    return m_solvableFlag;
}

void Board::setSolvableFlag(bool b)
{
    if (b && !m_solvableFlag && !solvable()) {
        m_solvableFlag = b;
        newGame();
    } else {
        m_solvableFlag = b;
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

void Board::setPauseEnabled(bool enabled)
{
    m_isPaused = enabled;
    if (m_isPaused) {
        m_pauseStart = time(NULL);
    } else {
        m_startTime += static_cast<time_t>(difftime(time(NULL), m_pauseStart));
    }
    update();
    emit changed();
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
    m_startTime = time(NULL);
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

void Board::gameStuck()
{
    m_isStuck = true;
}

void Board::gameOver()
{
    m_isOver = true;
// schwarzer: skip for now and observe if it is needed at all
//    emit changed();
//    update();
}

bool Board::isOver() const
{
    return m_isOver;
}

#include "board.moc"

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
