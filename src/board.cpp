/***************************************************************************
 *   KShisen - A japanese game similar to mahjongg                         *
 *   Copyright 1997   Mario Weilguni <mweilguni@sime.com>                  *
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

// own
#include "board.h"

// STL
#include <algorithm>
#include <array>

// Qt
#include <QMouseEvent>
#include <QPainter>
#include <QStandardPaths>
#include <QTimer>

// KDE
#include <KLocalizedString>

// KShisen
#include "debug.h"
#include "prefs.h"

namespace KShisen
{
#define EMPTY 0
#define SEASONS_START 28
#define FLOWERS_START 39

static std::array<int, 5> const s_delay = {1000, 750, 500, 250, 125};
static std::array<int, 6> const s_sizeX = {14, 16, 18, 24, 26, 30};
static std::array<int, 6> const s_sizeY = {6, 9, 8, 12, 14, 16};

Board::Board(QWidget * parent)
    : QWidget(parent)
    , m_gameClock()
    , m_tiles()
    , m_background()
    , m_random()
    , m_undo()
    , m_redo()
    , m_markX(0)
    , m_markY(0)
    , m_connection()
    , m_possibleMoves()
    , m_field()
    , m_xTiles(0)
    , m_yTiles(0)
    , m_delay(0)
    , m_level(0)
    , m_shuffle(0)
    , m_gameState(GameState::Normal)
    , m_cheat(false)
    , m_gravityFlag(true)
    , m_solvableFlag(false)
    , m_chineseStyleFlag(false)
    , m_tilesCanSlideFlag(false)
    , m_gravCols()
    , m_highlightedTile(-1)
    , m_paintConnection(false)
    , m_paintPossibleMoves(false)
    , m_paintInProgress(false)
    , m_tileRemove1()
    , m_tileRemove2()
    , m_soundPick(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("sounds/kshisen/tile-touch.ogg")))
    , m_soundFall(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("sounds/kshisen/tile-fall-tile.ogg")))
{
    m_tileRemove1.setX(-1);

    m_random.setSeed(0);
    resetTimer();

    QPalette palette;
    palette.setBrush(backgroundRole(), m_background.getBackground());
    setPalette(palette);

    loadSettings();
}

void Board::loadSettings()
{
    if (!loadTileset(Prefs::tileSet())) {
        qCWarning(KSHISEN_General) << "An error occurred when loading the tileset" << Prefs::tileSet() << "KShisen will continue with the default tileset.";
    }

    // Load background
    if (!loadBackground(Prefs::background())) {
        qCWarning(KSHISEN_General) << "An error occurred when loading the background" << Prefs::background() << "KShisen will continue with the default background.";
    }

    // There are tile sets, that have only one tile for e.g. the flowers group.
    // If these tile sets are played in non-chineseStyle, this one tile face
    // appears too often and not every tile matches another one with the same
    // face because they are technically different (e.g different flowers).
    // The solution is to enforce chineseStyle gameplay for tile sets that are
    // known to be reduced. Those are Egypt and Alphabet for now.
    if (Prefs::tileSet().endsWith(QLatin1String("egypt.desktop")) || Prefs::tileSet().endsWith(QLatin1String("alphabet.desktop"))) {
        setChineseStyleFlag(true);
    } else {
        setChineseStyleFlag(Prefs::chineseStyle());
    }
    setTilesCanSlideFlag(Prefs::tilesCanSlide());
    // Need to load solvable before size because setSize calls newGame which
    // uses the solvable flag. Same with shuffle.
    setSolvableFlag(Prefs::solvable());
    m_shuffle = Prefs::level() * 4 + 1;
    setSize(s_sizeX.at(Prefs::size()), s_sizeY.at(Prefs::size()));
    setGravityFlag(Prefs::gravity());
    setDelay(s_delay.at(Prefs::speed()));
    setSoundsEnabled(Prefs::sounds());

    if (m_level != Prefs::level()) {
        newGame();
    }
    m_level = Prefs::level();
}

bool Board::loadTileset(QString const & pathToTileset)
{
    if (m_tiles.loadTileset(pathToTileset)) {
        if (m_tiles.loadGraphics()) {
            Prefs::setTileSet(pathToTileset);
            Prefs::self()->save();
            resizeBoard();
        }
        return true;
    }
    //Try default
    if (m_tiles.loadDefault()) {
        if (m_tiles.loadGraphics()) {
            Prefs::setTileSet(m_tiles.path());
            Prefs::self()->save();
            resizeBoard();
        }
    }
    return false;
}

bool Board::loadBackground(QString const & pathToBackground)
{
    if (m_background.load(pathToBackground, width(), height())) {
        if (m_background.loadGraphics()) {
            Prefs::setBackground(pathToBackground);
            Prefs::self()->save();
            resizeBoard();
            return true;
        }
    }
    //Try default
    if (m_background.loadDefault()) {
        if (m_background.loadGraphics()) {
            Prefs::setBackground(m_background.path());
            Prefs::self()->save();
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

int Board::tiles() const
{
    return m_field.size();
}

void Board::setField(TilePos const & tilePos, int value)
{
    if (!isValidPos(tilePos)) {
        qCCritical(KSHISEN_General) << "Attempted write to invalid field position:"
                                    << tilePos.x()
                                    << ","
                                    << tilePos.y();
    }

    m_field.at(tilePos.y() * xTiles() + tilePos.x()) = value;
}

int Board::field(TilePos const & tilePos) const
{
    if (!isValidPosWithOutline(tilePos)) {
        qCCritical(KSHISEN_General) << "Attempted read from invalid field position:"
                                    << tilePos.x()
                                    << ","
                                    << tilePos.y();
    }

    if (!isValidPos(tilePos)) {
        return EMPTY;
    }

    return m_field.at(tilePos.y() * xTiles() + tilePos.x());
}

void Board::gravity(bool update)
{
    m_gravCols.clear();
    if (!m_gravityFlag) {
        return;
    }
    bool fallingTiles = false;
    for (int i = 0; i < xTiles(); ++i) {
        if (gravity(i, update)) {
            fallingTiles = true;
            m_gravCols.push_back(i);
        }
    }
    if (Prefs::sounds() && fallingTiles) {
        m_soundFall.start();
    }
}

bool Board::gravity(int column, bool update)
{
    bool isAffected = false;
    if (m_gravityFlag) {
        int rptr = yTiles() - 1;
        int wptr = yTiles() - 1;
        while (rptr >= 0) {
            if (field(TilePos(column, wptr)) != EMPTY) {
                --rptr;
                --wptr;
            } else {
                if (field(TilePos(column, rptr)) != EMPTY) {
                    setField(TilePos(column, wptr), field(TilePos(column, rptr)));
                    setField(TilePos(column, rptr), EMPTY);
                    isAffected = true;
                    if (update) {
                        updateField(TilePos(column, rptr));
                        updateField(TilePos(column, wptr));
                    }
                    --wptr;
                    --rptr;
                } else {
                    --rptr;
                }
            }
        }
    }
    return isAffected;
}

void Board::unmarkTile()
{
    // if nothing is marked, nothing to do
    if (m_markX == -1 || m_markY == -1) {
        return;
    }
    drawPossibleMoves(false);
    m_possibleMoves.clear();
    // We need to set m_markX and m_markY to -1 before calling
    // updateField() to ensure the tile is redrawn as unmarked.
    TilePos const oldTilePos(m_markX, m_markY);
    m_markX = -1;
    m_markY = -1;
    updateField(oldTilePos);
}

void Board::mousePressEvent(QMouseEvent * e)
{
    // Do not process mouse events while the connection is drawn.
    // Clicking on one of the already connected tiles would have selected
    // it before removing it. This is more a workaround than a proper fix
    // but I have to understand the usage of m_paintConnection first in
    // order to consider its reusage here. (schwarzer)
    if (m_paintInProgress) {
        return;
    }
    switch (m_gameState) {
        case GameState::Normal:
            break;
        case GameState::Over:
            newGame();
            return;
        case GameState::Paused:
            setPauseEnabled(false);
            return;
        case GameState::Stuck:
            return;
    }
    // Calculate field position
    int posX = (e->pos().x() - xOffset()) / (m_tiles.qWidth() * 2);
    int posY = (e->pos().y() - yOffset()) / (m_tiles.qHeight() * 2);

    if (e->pos().x() < xOffset() || e->pos().y() < yOffset() || posX >= xTiles() || posY >= yTiles()) {
        posX = -1;
        posY = -1;
    }

    // Mark tile
    if (e->button() == Qt::LeftButton) {
        clearHighlight();

        if (posX != -1) {
            marked(TilePos(posX, posY));
        } else {
            // unmark when clicking outside the board
            unmarkTile();
        }
    }

    // Assist by highlighting all tiles of same type
    if (e->button() == Qt::RightButton) {
        int const clickedTile = field(TilePos(posX, posY));

        // Clear marked tile
        if (m_markX != -1 && field(TilePos(m_markX, m_markY)) != clickedTile) {
            unmarkTile();
        } else {
            m_markX = -1;
            m_markY = -1;
        }

        // Perform highlighting
        if (clickedTile != m_highlightedTile) {
            int const oldHighlighted = m_highlightedTile;
            m_highlightedTile = clickedTile;
            for (int i = 0; i < xTiles(); ++i) {
                for (int j = 0; j < yTiles(); ++j) {
                    int const fieldTile = field(TilePos(i, j));
                    if (fieldTile != EMPTY) {
                        if (fieldTile == oldHighlighted) {
                            updateField(TilePos(i, j));
                        } else if (fieldTile == clickedTile) {
                            updateField(TilePos(i, j));
                        } else if (m_chineseStyleFlag) {
                            if (clickedTile >= SEASONS_START && clickedTile <= (SEASONS_START + 3) && fieldTile >= SEASONS_START && fieldTile <= (SEASONS_START + 3)) {
                                updateField(TilePos(i, j));
                            } else if (clickedTile >= FLOWERS_START && clickedTile <= (FLOWERS_START + 3) && fieldTile >= FLOWERS_START && fieldTile <= (FLOWERS_START + 3)) {
                                updateField(TilePos(i, j));
                            }
                            // oldHighlighted
                            if (oldHighlighted >= SEASONS_START && oldHighlighted <= (SEASONS_START + 3) && fieldTile >= SEASONS_START && fieldTile <= (SEASONS_START + 3)) {
                                updateField(TilePos(i, j));
                            } else if (oldHighlighted >= FLOWERS_START && oldHighlighted <= (FLOWERS_START + 3) && fieldTile >= FLOWERS_START && fieldTile <= (FLOWERS_START + 3)) {
                                updateField(TilePos(i, j));
                            }
                        }
                    }
                }
            }
        }
    }
}

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

    m_field.resize(x * y);

    m_xTiles = x;
    m_yTiles = y;

    std::fill(m_field.begin(), m_field.end(), EMPTY);

    // set the minimum size of the scalable window
    double const MINIMUM_SCALE = 0.2;
    int const w = qRound(m_tiles.qWidth() * 2.0 * MINIMUM_SCALE) * xTiles() + m_tiles.width();
    int const h = qRound(m_tiles.qHeight() * 2.0 * MINIMUM_SCALE) * yTiles() + m_tiles.height();

    setMinimumSize(w, h);

    resizeBoard();
    newGame();
    emit changed();
}

void Board::resizeEvent(QResizeEvent * e)
{
    qCDebug(KSHISEN_General) << "[resizeEvent]";
    if (e->spontaneous()) {
        qCDebug(KSHISEN_General) << "[resizeEvent] spontaneous";
    }
    resizeBoard();
    emit resized();
}

void Board::resizeBoard()
{
    // calculate tile size required to fit all tiles in the window
    QSize const newsize = m_tiles.preferredTileSize(QSize(width(), height()), xTiles(), yTiles());
    m_tiles.reloadTileset(newsize);
    //recalculate bg, if needed
    m_background.sizeChanged(width(), height());
    //reload our bg brush, using the cache in libkmahjongg if possible
    QPalette palette;
    palette.setBrush(backgroundRole(), m_background.getBackground());
    setPalette(palette);
}


void Board::newGame()
{
    m_gameState = GameState::Normal;
    setCheatModeEnabled(false);

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
                setField(TilePos(x, y), curTile);
                if (++tileCount >= 4) {
                    tileCount = 0;
                    ++curTile;
                }
            } else {
                tileCount = 0;
                setField(TilePos(x, y), curTile++);
            }
            if (curTile > Board::nTiles) {
                curTile = 1;
            }
        }
    }

    if (m_shuffle == 0) {
        update();
        resetTimer();
        emit newGameStarted();
        emit changed();
        return;
    }

    // shuffle the field
    int const tx = xTiles();
    int const ty = yTiles();
    for (int i = 0; i < tx * ty * m_shuffle; ++i) {
        TilePos const tilePos1(m_random.getLong(tx), m_random.getLong(ty));
        TilePos const tilePos2(m_random.getLong(tx), m_random.getLong(ty));
        // keep and use t, because the next setField() call changes what field() will return
        // so there would a significant impact on shuffling with the field() call put into the
        // place where 't' is used
        int const t = field(tilePos1);
        setField(tilePos1, field(tilePos2));
        setField(tilePos2, t);
    }

    // if m_solvableFlag is false, the game does not need to be solvable; we can drop out here
    if (!m_solvableFlag) {
        update();
        resetTimer();
        emit newGameStarted();
        emit changed();
        return;
    }


    std::vector<int> oldfield = m_field;
    std::vector<int> tiles;
    std::vector<int> pos;
    //jwickers: in case the game cannot made solvable we do not want to run an infinite loop
    int maxAttempts = 200;

    while (!isSolvable(false) && maxAttempts > 0) {
        // generate a list of free tiles and positions
        int numberOfTiles = 0;
        for (int i = 0; i < xTiles() * yTiles(); ++i) {
            if (m_field.at(i) != EMPTY) {
                pos.at(numberOfTiles) = i;
                tiles.at(numberOfTiles) = m_field.at(i);
                ++numberOfTiles;
            }
        }

        // restore field
        m_field = oldfield;

        // redistribute unsolved tiles
        while (numberOfTiles > 0) {
            // get a random tile
            int const r1 = m_random.getLong(numberOfTiles);
            int const r2 = m_random.getLong(numberOfTiles);
            int const tile = tiles.at(r1);
            int const apos = pos.at(r2);

            // truncate list
            tiles.at(r1) = tiles.at(numberOfTiles - 1);
            pos.at(r2) = pos.at(numberOfTiles - 1);
            --numberOfTiles;

            // put this tile on the new position
            m_field.at(apos) = tile;
        }

        // remember field
        oldfield = m_field;
        --maxAttempts;
    }
    // debug, tell if make solvable failed
    if (maxAttempts == 0) {
        qCCritical(KSHISEN_General) << "NewGame make solvable failed";
    }


    // restore field
    m_field = oldfield;

    update();
    resetTimer();
    emit changed();
}

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

bool Board::isTileHighlighted(TilePos const & tilePos) const
{
    if (tilePos.x() == m_markX && tilePos.y() == m_markY) {
        return true;
    }

    if (tilesMatch(m_highlightedTile, field(tilePos))) {
        return true;
    }

    // m_tileRemove1.first != -1 is used because the repaint of the first if
    // on undrawConnection highlighted the tiles that fell because of gravity
    if (!m_connection.empty() && m_tileRemove1.x() != -1) {
        if (tilePos.x() == m_connection.front().x() && tilePos.y() == m_connection.front().y()) {
            return true;
        }

        if (tilePos.x() == m_connection.back().x() && tilePos.y() == m_connection.back().y()) {
            return true;
        }
    }

    return false;
}

void Board::updateField(TilePos const & tilePos)
{
    QRect const r(xOffset() + tilePos.x() * m_tiles.qWidth() * 2,
                  yOffset() + tilePos.y() * m_tiles.qHeight() * 2,
                  m_tiles.width(),
                  m_tiles.height());

    update(r);
}

void Board::showInfoRect(QPainter & p, QString const & message)
{
    int const boxWidth = width() * 0.6;
    int const boxHeight = height() * 0.6;
    QRect const contentsRect = QRect((width() - boxWidth) / 2, (height() - boxHeight) / 2, boxWidth, boxHeight);
    QFont font;
    int const fontsize = boxHeight / 13;
    font.setPointSize(fontsize);
    p.setFont(font);
    p.setBrush(QBrush(QColor(100, 100, 100, 150)));
    p.setRenderHint(QPainter::Antialiasing);
    p.drawRoundedRect(contentsRect, 10, 10);

    p.drawText(contentsRect, Qt::AlignCenter | Qt::TextWordWrap, message);
}

void Board::drawTiles(QPainter & p, QPaintEvent * e)
{
    int const w = m_tiles.width();
    int const h = m_tiles.height();
    int const fw = m_tiles.qWidth() * 2;
    int const fh = m_tiles.qHeight() * 2;
    for (int i = 0; i < xTiles(); ++i) {
        for (int j = 0; j < yTiles(); ++j) {
            int const tile = field(TilePos(i, j));
            if (tile == EMPTY) {
                continue;
            }

            int const xpos = xOffset() + i * fw;
            int const ypos = yOffset() + j * fh;
            QRect const r(xpos, ypos, w, h);
            if (e->rect().intersects(r)) {
                if (isTileHighlighted(TilePos(i, j))) {
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

void Board::paintEvent(QPaintEvent * e)
{
    QRect const ur = e->rect(); // rectangle to update
    QPainter p(this);
    p.fillRect(ur, m_background.getBackground());

    switch (m_gameState) {
        case GameState::Normal:
            drawTiles(p, e);
            break;
        case GameState::Paused:
            showInfoRect(p, i18n("Game Paused\nClick to resume game."));
            break;
        case GameState::Stuck:
            drawTiles(p, e);
            showInfoRect(p, i18n("Game Stuck\nNo more moves possible."));
            break;
        case GameState::Over:
            showInfoRect(p, i18n("Game Over\nClick to start a new game."));
            break;
    }

    if (m_paintConnection) {
        p.setPen(QPen(QColor("red"), lineWidth()));

        auto pt1 = m_connection.cbegin();
        auto pt2 = pt1 + 1;
        while (pt2 != m_connection.cend()) {
            p.drawLine(midCoord(*pt1), midCoord(*pt2));
            ++pt1;
            ++pt2;
        }
        QTimer::singleShot(delay(), this, &Board::undrawConnection);
        m_paintConnection = false;
    }
    if (m_paintPossibleMoves) {
        p.setPen(QPen(QColor("blue"), lineWidth()));
        // paint all possible moves
        foreach (auto const move, m_possibleMoves) {
            auto pt1 = move.path().cbegin();
            auto pt2 = pt1 + 1;
            while (pt2 != move.path().cend()) {
                p.drawLine(midCoord(*pt1), midCoord(*pt2));
                ++pt1;
                ++pt2;
            }
        }
        m_paintConnection = false;
    }
    p.end();
}

void Board::reverseSlide(TilePos const & tilePos, int slideX1, int slideY1, int slideX2, int slideY2)
{
    // slide[XY]2 is the current location of the last tile to slide
    // slide[XY]1 is its destination
    // calculate the offset for the tiles to slide
    int const dx = slideX1 - slideX2;
    int const dy = slideY1 - slideY2;
    int current_tile;
    // move all tiles between slideX2, slideY2 and x, y to slide with that offset
    if (dx == 0) {
        if (tilePos.y() < slideY2) {
            for (int i = tilePos.y() + 1; i <= slideY2; ++i) {
                current_tile = field(TilePos(tilePos.x(), i));
                if (current_tile == EMPTY) {
                    continue;
                }
                setField(TilePos(tilePos.x(), i), EMPTY);
                setField(TilePos(tilePos.x(), i + dy), current_tile);
                updateField(TilePos(tilePos.x(), i));
                updateField(TilePos(tilePos.x(), i + dy));
            }
        } else {
            for (int i = tilePos.y() - 1; i >= slideY2; --i) {
                current_tile = field(TilePos(tilePos.x(), i));
                if (current_tile == EMPTY) {
                    continue;
                }
                setField(TilePos(tilePos.x(), i), EMPTY);
                setField(TilePos(tilePos.x(), i + dy), current_tile);
                updateField(TilePos(tilePos.x(), i));
                updateField(TilePos(tilePos.x(), i + dy));
            }
        }
    } else if (dy == 0) {
        if (tilePos.x() < slideX2) {
            for (int i = tilePos.x() + 1; i <= slideX2; ++i) {
                current_tile = field(TilePos(i, tilePos.y()));
                if (current_tile == EMPTY) {
                    continue;
                }
                setField(TilePos(i, tilePos.y()), EMPTY);
                setField(TilePos(i + dx, tilePos.y()), current_tile);
                updateField(TilePos(i, tilePos.y()));
                updateField(TilePos(i + dx, tilePos.y()));
            }
        } else {
            for (int i = tilePos.x() - 1; i >= slideX2; --i) {
                current_tile = field(TilePos(i, tilePos.y()));
                if (current_tile == EMPTY) {
                    continue;
                }
                setField(TilePos(i, tilePos.y()), EMPTY);
                setField(TilePos(i + dx, tilePos.y()), current_tile);
                updateField(TilePos(i, tilePos.y()));
                updateField(TilePos(i + dx, tilePos.y()));
            }
        }
    }
}

void Board::performSlide(TilePos const & tilePos, Path const & slide)
{
    // check if there is something to slide
    if (slide.empty()) {
        return;
    }

    // slide.first is the current location of the last tile to slide
    // slide.last is its destination
    // calculate the offset for the tiles to slide
    int const dx = slide.back().x() - slide.front().x();
    int const dy = slide.back().y() - slide.front().y();
    int current_tile;
    // move all tiles between m_markX, m_markY and the last tile to slide with that offset
    if (dx == 0) {
        if (tilePos.y() < slide.front().y()) {
            for (int i = slide.front().y(); i > tilePos.y(); --i) {
                current_tile = field(TilePos(tilePos.x(), i));
                setField(TilePos(tilePos.x(), i), EMPTY);
                setField(TilePos(tilePos.x(), i + dy), current_tile);
                updateField(TilePos(tilePos.x(), i));
                updateField(TilePos(tilePos.x(), i + dy));
            }
        } else {
            for (int i = slide.front().y(); i < tilePos.y(); ++i) {
                current_tile = field(TilePos(tilePos.x(), i));
                setField(TilePos(tilePos.x(), i), EMPTY);
                setField(TilePos(tilePos.x(), i + dy), current_tile);
                updateField(TilePos(tilePos.x(), i));
                updateField(TilePos(tilePos.x(), i + dy));
            }
        }
    } else if (dy == 0) {
        if (tilePos.x() < slide.front().x()) {
            for (int i = slide.front().x(); i > tilePos.x(); --i) {
                current_tile = field(TilePos(i, tilePos.y()));
                setField(TilePos(i, tilePos.y()), EMPTY);
                setField(TilePos(i + dx, tilePos.y()), current_tile);
                updateField(TilePos(i, tilePos.y()));
                updateField(TilePos(i + dx, tilePos.y()));
            }
        } else {
            for (int i = slide.front().x(); i < tilePos.x(); ++i) {
                current_tile = field(TilePos(i, tilePos.y()));
                setField(TilePos(i, tilePos.y()), EMPTY);
                setField(TilePos(i + dx, tilePos.y()), current_tile);
                updateField(TilePos(i, tilePos.y()));
                updateField(TilePos(i + dx, tilePos.y()));
            }
        }
    }
}

void Board::performMove(PossibleMove & possibleMoves)
{
    m_connection = possibleMoves.path();
#ifdef DEBUGGING
    // DEBUG undo, save board state
    std::vector<int> saved1 = m_field;
#endif
    // if the tiles can slide, we have to update the slided tiles too
    // and store the slide in a Move
    if (possibleMoves.hasSlide()) {
        performSlide(TilePos(m_markX, m_markY), possibleMoves.slide());
        madeMove(TilePos(m_markX, m_markY), TilePos(possibleMoves.path().back().x(), possibleMoves.path().back().y()), possibleMoves.slide());
    } else {
        madeMove(TilePos(m_markX, m_markY), TilePos(possibleMoves.path().back().x(), possibleMoves.path().back().y()));
    }
    drawPossibleMoves(false);
    drawConnection();
    m_tileRemove1 = TilePos(m_markX, m_markY);
    m_tileRemove2 = TilePos(possibleMoves.path().back().x(), possibleMoves.path().back().y());
    m_markX = -1;
    m_markY = -1;
    m_possibleMoves.clear();
#ifdef DEBUGGING
    // DEBUG undo, force gravity
    undrawConnection();
    // DEBUG undo, save board2 state
    std::vector<int> saved2 = m_field;
    std::vector<int> saved3; // after undo
    std::vector<int> saved4; // after redo
    // DEBUG undo, undo move
    bool errorFound = false;
    if (canUndo()) {
        undo();
        // DEBUG undo, compare to saved board state
        for (int i = 0; i < xTiles() * yTiles(); ++i) {
            if (saved1.at(i) != m_field.at(i)) {
                qCDebug(KSHISEN_General) << "[DEBUG Undo 1], tile (" << i << ") was" << saved1.at(i) << "before move, it is" << m_field.at(i) << "after undo.";
                errorFound = true;
            }
        }
        // DEBUG undo, save board state
        saved3 = m_field;
        // DEBUG undo, redo
        if (canRedo()) {
            redo();
            undrawConnection();
            // DEBUG undo, compare to saved board2 state
            for (int i = 0; i < xTiles() * yTiles(); ++i) {
                if (saved2.at(i) != m_field.at(i)) {
                    qCDebug(KSHISEN_General) << "[DEBUG Undo 2], tile (" << i << ") was" << saved2.at(i) << "after move, it is" << m_field.at(i) << "after redo.";
                    errorFound = true;
                }
            }
            // DEBUG undo, save board state
            saved4 = m_field;
        }
    }
    // dumpBoard on error
    if (errorFound) {
        qCDebug(KSHISEN_General) << "[DEBUG] Before move";
        dumpBoard(saved1);
        qCDebug(KSHISEN_General) << "[DEBUG] After move";
        dumpBoard(saved2);
        qCDebug(KSHISEN_General) << "[DEBUG] Undo";
        dumpBoard(saved3);
        qCDebug(KSHISEN_General) << "[DEBUG] Redo";
        dumpBoard(saved4);
    }

#endif
}

void Board::marked(TilePos const & tilePos)
{
    if (field(tilePos) == EMPTY) { // click on empty space on the board
        if (m_possibleMoves.size() > 1) { // if the click is on any of the current possible moves, make that move
            foreach (auto move, m_possibleMoves) {
                if (move.isInPath(tilePos)) {
                    performMove(move);
                    emit selectATile();
                    return;
                }
            }
        } else {
            // unmark when not clicking on a tile
            unmarkTile();
            return;
        }
    }
    // make sure that the previous connection is correctly undrawn
    undrawConnection(); // is this still needed? (schwarzer)

    if (Prefs::sounds()) {
        m_soundPick.start();
    }

    if (tilePos.x() == m_markX && tilePos.y() == m_markY) { // the piece is already marked
        // unmark the piece
        unmarkTile();
        emit selectATile();
        return;
    }

    if (m_markX == -1) { // nothing is selected so far
        m_markX = tilePos.x();
        m_markY = tilePos.y();
        drawPossibleMoves(false);
        m_possibleMoves.clear();
        updateField(tilePos);
        emit selectAMatchingTile();
        return;
    }
    if (m_possibleMoves.size() > 1) { // if the click is on any of the current possible moves, make that move

        foreach (auto move, m_possibleMoves) {
            if (move.isInPath(tilePos)) {
                performMove(move);
                emit selectATile();
                return;
            }
        }
    }

    int const tile1 = field(TilePos(m_markX, m_markY));
    int const tile2 = field(tilePos);

    // both tiles do not match
    if (!tilesMatch(tile1, tile2)) {
        unmarkTile();
        emit tilesDoNotMatch();
        return;
    }

    // trace and perform the move and get the list of possible moves
    if (findPath(TilePos(m_markX, m_markY), tilePos, m_possibleMoves) > 0) {
        if (m_possibleMoves.size() > 1) {
            int withSlide = 0;
            foreach (auto const move, m_possibleMoves) {
                move.Debug();
                if (move.hasSlide()) {
                    ++withSlide;
                }
            }
            // if all moves have no slide, it doesn't matter
            if (withSlide > 0) {
                drawPossibleMoves(true);
                emit selectAMove();
                return;
            }
        }

        // only one move possible, perform it
        performMove(m_possibleMoves.front());
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
    if (m_highlightedTile == -1) {
        return;
    }
    int const oldHighlighted = m_highlightedTile;
    m_highlightedTile = -1;

    for (int i = 0; i < xTiles(); ++i) {
        for (int j = 0; j < yTiles(); ++j) {
            if (tilesMatch(oldHighlighted, field(TilePos(i, j)))) {
                updateField(TilePos(i, j));
            }
        }
    }
}

bool Board::canMakePath(TilePos const & tilePos1, TilePos const & tilePos2) const
{
    if (tilePos1.x() == tilePos2.x()) {
        for (int i = qMin(tilePos1.y(), tilePos2.y()) + 1; i < qMax(tilePos1.y(), tilePos2.y()); ++i) {
            if (field(TilePos(tilePos1.x(), i)) != EMPTY) {
                return false;
            }
        }
        return true;
    }

    if (tilePos1.y() == tilePos2.y()) {
        for (int i = qMin(tilePos1.x(), tilePos2.x()) + 1; i < qMax(tilePos1.x(), tilePos2.x()); ++i) {
            if (field(TilePos(i, tilePos1.y())) != EMPTY) {
                return false;
            }
        }
        return true;
    }

    return false;
}

bool Board::canSlideTiles(TilePos const & tilePos1, TilePos const & tilePos2, Path & path) const
{
    int distance = -1;
    path.clear();
    if (tilePos1.x() == tilePos2.x()) {
        if (tilePos1.y() > tilePos2.y()) {
            distance = tilePos1.y() - tilePos2.y();
            // count how much free space we have for sliding
            int start_free = -1;
            int end_free = -1;
            // find first tile empty
            for (int i = tilePos1.y() - 1; i >= 0; --i) {
                if (field(TilePos(tilePos1.x(), i)) == EMPTY) {
                    start_free = i;
                    break;
                }
            }
            // if not found, cannot slide
            // if the first free tile is just next to the sliding tile, no slide (should be a normal move)
            if (start_free == -1 || start_free == (tilePos1.y() - 1)) {
                return false;
            }
            // find last tile empty
            for (int i = start_free - 1; i >= 0; --i) {
                if (field(TilePos(tilePos1.x(), i)) != EMPTY) {
                    end_free = i;
                    break;
                }
            }
            // if not found, it is the border: 0

            // so we can slide of start_free - end_free, compare this to the distance
            if (distance <= (start_free - end_free)) {
                // first position of the last slided tile
                path.push_back(TilePos(tilePos1.x(), start_free + 1));
                // final position of the last slided tile
                path.push_back(TilePos(tilePos1.x(), start_free + 1 - distance));
                return true;
            }
            return false;

        } else if (tilePos2.y() > tilePos1.y()) {
            distance = tilePos2.y() - tilePos1.y();
            // count how much free space we have for sliding
            int start_free = -1;
            int end_free = yTiles();
            // find first tile empty
            for (int i = tilePos1.y() + 1; i < yTiles(); ++i) {
                if (field(TilePos(tilePos1.x(), i)) == EMPTY) {
                    start_free = i;
                    break;
                }
            }
            // if not found, cannot slide
            // if the first free tile is just next to the sliding tile, no slide (should be a normal move)
            if (start_free == -1 || start_free == tilePos1.y() + 1) {
                return false;
            }
            // find last tile empty
            for (int i = start_free + 1; i < yTiles(); ++i) {
                if (field(TilePos(tilePos1.x(), i)) != EMPTY) {
                    end_free = i;
                    break;
                }
            }
            // if not found, it is the border: yTiles()-1

            // so we can slide of end_free - start_free, compare this to the distance
            if (distance <= (end_free - start_free)) {
                // first position of the last slided tile
                path.push_back(TilePos(tilePos1.x(), start_free - 1));
                // final position of the last slided tile
                path.push_back(TilePos(tilePos1.x(), start_free - 1 + distance));
                return true;
            }
            return false;
        }
        // y1 == y2 ?!
        return false;
    }

    if (tilePos1.y() == tilePos2.y()) {
        if (tilePos1.x() > tilePos2.x()) {
            distance = tilePos1.x() - tilePos2.x();
            // count how much free space we have for sliding
            int start_free = -1;
            int end_free = -1;
            // find first tile empty
            for (int i = tilePos1.x() - 1; i >= 0; --i) {
                if (field(TilePos(i, tilePos1.y())) == EMPTY) {
                    start_free = i;
                    break;
                }
            }
            // if not found, cannot slide
            // if the first free tile is just next to the sliding tile, no slide (should be a normal move)
            if (start_free == -1 || start_free == tilePos1.x() - 1) {
                return false;
            }
            // find last tile empty
            for (int i = start_free - 1; i >= 0; --i) {
                if (field(TilePos(i, tilePos1.y())) != EMPTY) {
                    end_free = i;
                    break;
                }
            }
            // if not found, it is the border: 0

            // so we can slide of start_free - end_free, compare this to the distance
            if (distance <= (start_free - end_free)) {
                // first position of the last slided tile
                path.push_back(TilePos(start_free + 1, tilePos1.y()));
                // final position of the last slided tile
                path.push_back(TilePos(start_free + 1 - distance, tilePos1.y()));
                return true;
            }
            return false;

        } else if (tilePos2.x() > tilePos1.x()) {
            distance = tilePos2.x() - tilePos1.x();
            // count how much free space we have for sliding
            int start_free = -1;
            int end_free = xTiles();
            // find first tile empty
            for (int i = tilePos1.x() + 1; i < xTiles(); ++i) {
                if (field(TilePos(i, tilePos1.y())) == EMPTY) {
                    start_free = i;
                    break;
                }
            }
            // if not found, cannot slide
            // if the first free tile is just next to the sliding tile, no slide (should be a normal move)
            if (start_free == -1 || start_free == tilePos1.x() + 1) {
                return false;
            }
            // find last tile empty
            for (int i = start_free + 1; i < xTiles(); ++i) {
                if (field(TilePos(i, tilePos1.y())) != EMPTY) {
                    end_free = i;
                    break;
                }
            }
            // if not found, it is the border: xTiles()-1

            // so we can slide of end_free - start_free, compare this to the distance
            if (distance <= (end_free - start_free)) {
                // first position of the last slided tile
                path.push_back(TilePos(start_free - 1, tilePos1.y()));
                // final position of the last slided tile
                path.push_back(TilePos(start_free - 1 + distance, tilePos1.y()));
                return true;
            }
            return false;
        }
        // x1 == x2 ?!
        return false;
    }
    return false;
}

int Board::findPath(TilePos const & tilePos1, TilePos const & tilePos2, PossibleMoves & possibleMoves) const
{
    possibleMoves.clear();

    int simplePath = 0;

    // first find the simple paths
    int numberOfPaths = findSimplePath(tilePos1, tilePos2, possibleMoves);

    // if the tiles can slide, 2 lines max is allowed
    if (m_tilesCanSlideFlag) {
        return numberOfPaths;
    }

    // Find paths of 3 segments
    std::array<int, 4> const dx = {1, 0, -1, 0};
    std::array<int, 4> const dy = {0, 1, 0, -1};

    for (int i = 0; i < 4; ++i) {
        int tempX = tilePos1.x() + dx.at(i);
        int tempY = tilePos1.y() + dy.at(i);
        while (isValidPosWithOutline(TilePos(tempX, tempY)) && field(TilePos(tempX, tempY)) == EMPTY) {
            if ((simplePath = findSimplePath(TilePos(tempX, tempY), tilePos2, possibleMoves)) > 0) {
                possibleMoves.back().prependTile(tilePos1);
                numberOfPaths += simplePath;
            }
            tempX += dx.at(i);
            tempY += dy.at(i);
        }
    }
    return numberOfPaths;
}

int Board::findSimplePath(TilePos const & tilePos1, TilePos const & tilePos2, PossibleMoves & possibleMoves) const
{
    int numberOfPaths = 0;
    Path path;
    // Find direct line (path of 1 segment)
    if (canMakePath(tilePos1, tilePos2)) {
        path.push_back(tilePos1);
        path.push_back(tilePos2);
        possibleMoves.push_back(PossibleMove(path));
        ++numberOfPaths;
    }

    // If the tiles are in the same row or column, then a
    // a 'simple path' cannot be found between them
    // That is, canMakePath should have returned true above if
    // that was possible
    if (tilePos1.x() == tilePos2.x() || tilePos1.y() == tilePos2.y()) {
        return numberOfPaths;
    }

    // I isolate the special code when tiles can slide even if it duplicates code for now
    // Can we make a path sliding tiles ?, the slide move is always first, then a normal path
    if (m_tilesCanSlideFlag) {
        Path slidePath;
        // Find path of 2 segments (route A)
        if (canSlideTiles(tilePos1, TilePos(tilePos2.x(), tilePos1.y()), slidePath)
            && canMakePath(TilePos(tilePos2.x(), tilePos1.y()), tilePos2)) {
            path.clear();
            path.push_back(tilePos1);
            path.push_back(TilePos(tilePos2.x(), tilePos1.y()));
            path.push_back(tilePos2);
            possibleMoves.push_back(PossibleMove(path, slidePath));
            ++numberOfPaths;
        }

        // Find path of 2 segments (route B)
        if (canSlideTiles(tilePos1, TilePos(tilePos1.x(), tilePos2.y()), slidePath)
            && canMakePath(TilePos(tilePos1.x(), tilePos2.y()), tilePos2)) {
            path.clear();
            path.push_back(tilePos1);
            path.push_back(TilePos(tilePos1.x(), tilePos2.y()));
            path.push_back(tilePos2);
            possibleMoves.push_back(PossibleMove(path, slidePath));
            ++numberOfPaths;
        }
    }

    // Even if tiles can slide, a path could still be done without sliding

    // Find path of 2 segments (route A)
    if (field(TilePos(tilePos2.x(), tilePos1.y())) == EMPTY
        && canMakePath(tilePos1, TilePos(tilePos2.x(), tilePos1.y()))
        && canMakePath(TilePos(tilePos2.x(), tilePos1.y()), tilePos2)) {
        path.clear();
        path.push_back(tilePos1);
        path.push_back(TilePos(tilePos2.x(), tilePos1.y()));
        path.push_back(tilePos2);
        possibleMoves.push_back(PossibleMove(path));
        ++numberOfPaths;
    }

    // Find path of 2 segments (route B)
    if (field(TilePos(tilePos1.x(), tilePos2.y())) == EMPTY
        && canMakePath(tilePos1, TilePos(tilePos1.x(), tilePos2.y()))
        && canMakePath(TilePos(tilePos1.x(), tilePos2.y()), tilePos2)) {
        path.clear();
        path.push_back(tilePos1);
        path.push_back(TilePos(tilePos1.x(), tilePos2.y()));
        path.push_back(tilePos2);
        possibleMoves.push_back(PossibleMove(path));
        ++numberOfPaths;
    }

    return numberOfPaths;
}

void Board::drawPossibleMoves(bool b)
{
    if (m_possibleMoves.empty()) {
        return;
    }

    m_paintPossibleMoves = b;
    update();
}

void Board::drawConnection()
{
    m_paintInProgress = true;
    if (m_connection.empty()) {
        return;
    }

    TilePos const tile1(m_connection.front().x(), m_connection.front().y());
    TilePos const tile2(m_connection.back().x(), m_connection.back().y());
    // lighten the fields
    updateField(tile1);
    updateField(tile2);

    m_paintConnection = true;
    update();
}

void Board::undrawConnection()
{
    if (m_tileRemove1.x() != -1) {
        setField(m_tileRemove1, EMPTY);
        setField(m_tileRemove2, EMPTY);
        m_tileRemove1.setX(-1);
        update();
        emit tileCountChanged();
    }

    gravity(true); // why is this called here? (schwarzer)

    // is already undrawn?
    if (m_connection.empty()) {
        return;
    }

    // Redraw all affected fields
    Path const oldConnection = m_connection;
    m_connection.clear();
    m_paintConnection = false;

    auto pt1 = oldConnection.cbegin();
    auto pt2 = pt1 + 1;
    while (pt2 != oldConnection.cend()) {
        if (pt1->y() == pt2->y()) {
            for (int i = qMin(pt1->x(), pt2->x()); i <= qMax(pt1->x(), pt2->x()); ++i) {
                updateField(TilePos(i, pt1->y()));
            }
        } else {
            for (int i = qMin(pt1->y(), pt2->y()); i <= qMax(pt1->y(), pt2->y()); ++i) {
                updateField(TilePos(pt1->x(), i));
            }
        }
        ++pt1;
        ++pt2;
    }

    PossibleMoves dummyPossibleMoves;
    // game is over?
    if (!hint_I(dummyPossibleMoves)) {
        m_gameClock.pause();
        emit endOfGame();
    }
    m_paintInProgress = false;
}

QPoint Board::midCoord(TilePos const & tilePos) const
{
    QPoint p;
    int const w = m_tiles.qWidth() * 2;
    int const h = m_tiles.qHeight() * 2;

    if (tilePos.x() == -1) {
        p.setX(xOffset() - (w / 4));
    } else if (tilePos.x() == xTiles()) {
        p.setX(xOffset() + (w * xTiles()) + (w / 4));
    } else {
        p.setX(xOffset() + (w * tilePos.x()) + (w / 2));
    }

    if (tilePos.y() == -1) {
        p.setY(yOffset() - (w / 4));
    } else if (tilePos.y() == yTiles()) {
        p.setY(yOffset() + (h * yTiles()) + (w / 4));
    } else {
        p.setY(yOffset() + (h * tilePos.y()) + (h / 2));
    }

    return p;
}

void Board::setDelay(int newValue)
{
    if (m_delay == newValue) {
        return;
    }
    m_delay = newValue;
}

int Board::delay() const
{
    return m_delay;
}

void Board::madeMove(TilePos const & tilePos1, TilePos const & tilePos2, Path slide)
{
    Move * move;
    if (slide.empty()) {
        move = new Move(tilePos1, tilePos2, field(tilePos1), field(tilePos2));
    } else {
        move = new Move(tilePos1, tilePos2, field(tilePos1), field(tilePos2), slide.front().x(), slide.front().y(), slide.back().x(), slide.back().y());
    }
    m_undo.push_back(move);
    while (m_redo.size() != 0) {
        delete m_redo.front();
        m_redo.removeFirst();
    }
    emit changed();
}

bool Board::canUndo() const
{
    return !m_undo.empty();
}

bool Board::canRedo() const
{
    return !m_redo.empty();
}

void Board::undo()
{
    if (!canUndo()) {
        return;
    }

    clearHighlight();
    undrawConnection();
    Move * move = m_undo.takeLast();
    if (gravityFlag()) {
        // When both tiles reside in the same column, the order of undo is
        // significant (we must undo the lower tile first).
        // Also in that case there cannot be a slide
        if (move->x1() == move->x2() && move->y1() < move->y2()) {
            move->swapTiles();
        }

        // if there is no slide, keep previous implementation: move both column up
        if (!move->hasSlide()) {
            qCDebug(KSHISEN_General) << "[undo] gravity from a no slide move";

            // move tiles from the first column up
            for (int y = 0; y < move->y1(); ++y) {
                setField(TilePos(move->x1(), y), field(TilePos(move->x1(), y + 1)));
                updateField(TilePos(move->x1(), y));
            }

            // move tiles from the second column up
            for (int y = 0; y < move->y2(); ++y) {
                setField(TilePos(move->x2(), y), field(TilePos(move->x2(), y + 1)));
                updateField(TilePos(move->x2(), y));
            }
        } else { // else check all tiles from the slide that may have fallen down

            qCDebug(KSHISEN_General) << "[undo] gravity from slide s1(" << move->slideX1() << "," << move->slideY1() << ")=>s2(" << move->slideX2() << "," << move->slideY2() << ") matching (" << move->x1() << "," << move->y1() << ")=>(" << move->x2() << "," << move->y2() << ")";

            // horizontal slide
            // because tiles that slides horizontaly may fall down
            // in columns different than the taken tiles columns
            // we need to take them back up then undo the slide
            if (move->slideY1() == move->slideY2()) {
                qCDebug(KSHISEN_General) << "[undo] gravity from horizontal slide";

                // last slide tile went from slide_x1 -> slide_x2
                // the number of slided tiles is n = abs(x1 - slide_x1)
                int n = move->x1() - move->slideX1();
                if (n < 0) {
                    n = -n;
                }
                // distance slided is
                int dx = move->slideX2() - move->slideX1();
                if (dx < 0) {
                    dx = -dx;
                }

                qCDebug(KSHISEN_General) << "[undo] n =" << n;

                // slided tiles may fall down after the slide
                // so any tiles on top of the columns between
                // slide_x2 -> slide_x2 +/- n (excluded) should go up to slide_y1
                if (move->slideX2() > move->slideX1()) { // slide to the right

                    qCDebug(KSHISEN_General) << "[undo] slide right";

                    for (int i = move->slideX2(); i > move->slideX2() - n; --i) {
                        // find top tile
                        int j;
                        for (j = 0; j < yTiles(); ++j) {
                            if (field(TilePos(i, j)) != EMPTY) {
                                break;
                            }
                        }

                        // ignore if the tile did not fall
                        if (j <= move->slideY1()) {
                            continue;
                        }

                        qCDebug(KSHISEN_General) << "[undo] moving (" << i << "," << j << ") up to (" << i << "," << move->slideY1() << ")";

                        // put it back up
                        setField(TilePos(i, move->slideY1()), field(TilePos(i, j)));
                        setField(TilePos(i, j), EMPTY);
                        updateField(TilePos(i, j));
                        updateField(TilePos(i, move->slideY1()));
                    }
                } else { // slide to the left

                    qCDebug(KSHISEN_General) << "[undo] slide left";

                    for (int i = move->slideX2(); i < move->slideX2() + n; ++i) {
                        // find top tile
                        int j;
                        for (j = 0; j < yTiles(); ++j) {
                            if (field(TilePos(i, j)) != EMPTY) {
                                break;
                            }
                        }

                        // ignore if the tile did not fall
                        if (j <= move->slideY1()) {
                            continue;
                        }

                        qCDebug(KSHISEN_General) << "[undo] moving (" << i << "," << j << ") up to (" << i << "," << move->slideY1() << ")";

                        // put it back up
                        setField(TilePos(i, move->slideY1()), field(TilePos(i, j)));
                        setField(TilePos(i, j), EMPTY);
                        updateField(TilePos(i, j));
                        updateField(TilePos(i, move->slideY1()));
                    }
                }

                qCDebug(KSHISEN_General) << "[undo] moving up column x2" << move->x2();

                // move tiles from the second column up
                for (int y = 0; y <= move->y2(); ++y) {
                    qCDebug(KSHISEN_General) << "[undo] moving up tile" << y + 1;

                    setField(TilePos(move->x2(), y), field(TilePos(move->x2(), y + 1)));
                    updateField(TilePos(move->x2(), y));
                }
                // and all columns that fell after the tiles slided between
                // only if they were not replaced by a sliding tile !!
                // x1 -> x1+dx should go up one
                // if their height > slide_y1
                // because they have fallen after the slide
                if (move->slideX2() > move->slideX1()) { // slide to the right
                    if (move->slideY1() > 0) {
                        for (int i = move->x1() + dx; i >= move->x1(); --i) {
                            qCDebug(KSHISEN_General) << "[undo] moving up column" << i << "until" << move->slideY1();

                            for (int j = 0; j < move->slideY1(); ++j) {
                                qCDebug(KSHISEN_General) << "[undo] moving up tile" << j + 1;

                                setField(TilePos(i, j), field(TilePos(i, j + 1)));
                                updateField(TilePos(i, j));
                            }

                            qCDebug(KSHISEN_General) << "[undo] clearing last tile" << move->slideY1();

                            setField(TilePos(i, move->slideY1()), EMPTY);
                            updateField(TilePos(i, move->slideY1()));
                        }
                    }
                } else { // slide to the left
                    if (move->slideY1() > 0) {
                        for (int i = move->x1() - dx; i <= move->x1(); ++i) {
                            qCDebug(KSHISEN_General) << "[undo] moving up column" << i << "until" << move->slideY1();

                            for (int j = 0; j < move->slideY1(); ++j) {
                                qCDebug(KSHISEN_General) << "[undo] moving up tile" << j + 1;

                                setField(TilePos(i, j), field(TilePos(i, j + 1)));
                                updateField(TilePos(i, j));
                            }

                            qCDebug(KSHISEN_General) << "[undo] clearing last tile" << move->slideY1();

                            setField(TilePos(i, move->slideY1()), EMPTY);
                            updateField(TilePos(i, move->slideY1()));
                        }
                    }
                }

                qCDebug(KSHISEN_General) << "[undo] reversing slide";

                // then undo the slide to put the tiles back to their original location
                reverseSlide(TilePos(move->x1(), move->y1()), move->slideX1(), move->slideY1(), move->slideX2(), move->slideY2());

            } else {
                qCDebug(KSHISEN_General) << "[undo] gravity from vertical slide";

                // vertical slide, in fact nothing special is necessary
                // the default implementation works because it only affects
                // the two columns were tiles were taken

                // move tiles from the first column up
                for (int y = 0; y < move->y1(); ++y) {
                    setField(TilePos(move->x1(), y), field(TilePos(move->x1(), y + 1)));
                    updateField(TilePos(move->x1(), y));
                }

                // move tiles from the second column up
                for (int y = 0; y < move->y2(); ++y) {
                    setField(TilePos(move->x2(), y), field(TilePos(move->x2(), y + 1)));
                    updateField(TilePos(move->x2(), y));
                }
            }
        }
    } else { // no gravity
        // undo slide if any
        if (move->hasSlide()) {
            // perform the slide in reverse
            reverseSlide(TilePos(move->x1(), move->y1()), move->slideX1(), move->slideY1(), move->slideX2(), move->slideY2());
        }
    }

    // replace taken tiles
    setField(TilePos(move->x1(), move->y1()), move->tile1());
    setField(TilePos(move->x2(), move->y2()), move->tile2());
    updateField(TilePos(move->x1(), move->y1()));
    updateField(TilePos(move->x2(), move->y2()));

    m_redo.prepend(move);
    emit changed();
}

void Board::redo()
{
    if (canRedo()) {
        clearHighlight();
        undrawConnection();
        Move * move = m_redo.takeFirst();
        // redo the slide if any
        if (move->hasSlide()) {
            Path s;
            s.push_back(TilePos(move->slideX1(), move->slideY1()));
            s.push_back(TilePos(move->slideX2(), move->slideY2()));
            performSlide(TilePos(move->x1(), move->y1()), s);
        }
        setField(TilePos(move->x1(), move->y1()), EMPTY);
        setField(TilePos(move->x2(), move->y2()), EMPTY);
        updateField(TilePos(move->x1(), move->y1()));
        updateField(TilePos(move->x2(), move->y2()));
        gravity(true);
        m_undo.push_back(move);
        emit changed();
    }
}

void Board::showHint()
{
    undrawConnection();

    if (hint_I(m_possibleMoves)) {
        m_connection = m_possibleMoves.front().path();
        drawConnection();
    }
}


#ifdef DEBUGGING
void Board::makeHintMove()
{
    PossibleMoves possibleMoves;

    if (hint_I(possibleMoves)) {
        m_markX = -1;
        m_markY = -1;
        marked(TilePos(possibleMoves.front().path().front().x(), possibleMoves.front().path().front().y()));
        marked(TilePos(possibleMoves.front().path().back().x(), possibleMoves.front().path().back().y()));
    }
}


void Board::dumpBoard() const
{
    dumpBoard(m_field);
}

void Board::dumpBoard(const std::vector<int> & board) const
{
    qCDebug(KSHISEN_General) << "Board contents:";
    for (int y = 0; y < yTiles(); ++y) {
        QString row;
        for (int x = 0; x < xTiles(); ++x) {
            int tile = board.at(y * xTiles() + x);
            if (tile == EMPTY) {
                row += QLatin1String(" --");
            } else {
                row += QString(QLatin1String("%1")).arg(tile, 3);
            }
        }
        qCDebug(KSHISEN_General) << row;
    }
}
#endif

int Board::lineWidth() const
{
    int width = qRound(m_tiles.height() / 10.0);
    if (width < 3) {
        width = 3;
    }

    return width;
}

bool Board::hint_I(PossibleMoves & possibleMoves) const
{
    std::array<short, Board::nTiles> done{}; // Appended {} initialises with zeroes here.

    for (int x = 0; x < xTiles(); ++x) {
        for (int y = 0; y < yTiles(); ++y) {
            int const tile = field(TilePos(x, y));
            if (tile != EMPTY && done.at(tile - 1) != 4) {
                // for all these types of tile search paths
                for (int xx = 0; xx < xTiles(); ++xx) {
                    for (int yy = 0; yy < yTiles(); ++yy) {
                        if (xx != x || yy != y) {
                            if (tilesMatch(field(TilePos(xx, yy)), tile)) {
                                if (findPath(TilePos(x, y), TilePos(xx, yy), possibleMoves) > 0) {
                                    return true;
                                }
                            }
                        }
                    }
                }
                done.at(tile - 1)++;
            }
        }
    }
    return false;
}

int Board::tilesLeft() const
{
    return std::count_if(m_field.begin(), m_field.end(), [](int field) { return field != EMPTY; });
}

int Board::currentTime() const
{
    return m_gameClock.seconds();
}

bool Board::isSolvable(bool restore)
{
    std::vector<int> oldField;

    if (!restore) {
        oldField = m_field;
    }

    PossibleMoves p;
    while (hint_I(p)) {
        TilePos const tile1(p.front().path().front().x(), p.front().path().front().y());
        TilePos const tile2(p.front().path().back().x(), p.front().path().back().y());
        if (!tilesMatch(field(tile1), field(tile2))) {
            QString errMessage = QStringLiteral("Removing unmatched tiles: (%1,%2) => %3 (%4,%5) => %6")
                                     .arg(p.front().path().front().x())
                                     .arg(p.front().path().front().y())
                                     .arg(field(tile1))
                                     .arg(p.front().path().back().x())
                                     .arg(p.front().path().back().y())
                                     .arg(field(tile2));
            qCCritical(KSHISEN_General) << errMessage;
        }
        setField(tile1, EMPTY);
        setField(tile2, EMPTY);
    }

    int const left = tilesLeft();

    if (!restore) {
        m_field = oldField;
    }

    return left == 0;
}

bool Board::solvableFlag() const
{
    return m_solvableFlag;
}

void Board::setSolvableFlag(bool enabled)
{
    if (m_solvableFlag == enabled) {
        return;
    }
    m_solvableFlag = enabled;
    // if the solvable flag was set and the current game is not solvable, start a new game
    if (m_solvableFlag && !isSolvable(true)) {
        newGame();
    }
}

bool Board::gravityFlag() const
{
    return m_gravityFlag;
}

void Board::setGravityFlag(bool enabled)
{
    if (m_gravityFlag == enabled) {
        return;
    }
    m_gravityFlag = enabled;
    // start a new game if the player is in the middle of a game
    if (canUndo() || canRedo()) {
        newGame();
    }
}

void Board::setChineseStyleFlag(bool enabled)
{
    if (m_chineseStyleFlag == enabled) {
        return;
    }
    m_chineseStyleFlag = enabled;
    // we need to force a newGame() because board generation is different
    newGame();
}

void Board::setTilesCanSlideFlag(bool enabled)
{
    if (m_tilesCanSlideFlag == enabled) {
        return;
    }
    m_tilesCanSlideFlag = enabled;
    // start a new game if the player is in the middle of a game
    if (canUndo() || canRedo()) {
        newGame();
    }
}

void Board::setPauseEnabled(bool enabled)
{
    if ((m_gameState == GameState::Paused && enabled) || m_gameState == GameState::Stuck) {
        return;
    }
    if (enabled) {
        m_gameState = GameState::Paused;
        m_gameClock.pause();
    } else {
        m_gameState = GameState::Normal;
        m_gameClock.resume();
    }
    emit changed();
    update();
}

QSize Board::sizeHint() const
{
    int dpi = logicalDpiX();
    if (dpi < 75) {
        dpi = 75;
    }
    return QSize(9 * dpi, 7 * dpi);
}

void Board::resetTimer()
{
    m_gameClock.restart();
}

void Board::resetUndo()
{
    if (!canUndo()) {
        return;
    }
    qDeleteAll(m_undo);
    m_undo.clear();
}

void Board::resetRedo()
{
    if (!canRedo()) {
        return;
    }
    qDeleteAll(m_redo);
    m_redo.clear();
}

void Board::setGameStuckEnabled(bool enabled)
{
    if (m_gameState == GameState::Stuck && enabled) {
        return;
    }
    if (enabled) {
        m_gameState = GameState::Stuck;
        m_gameClock.pause();
    } else {
        m_gameState = GameState::Normal;
        m_gameClock.resume();
    }
    emit changed();
    update();
}

void Board::setGameOverEnabled(bool enabled)
{
    if (m_gameState == GameState::Over && enabled) {
        return;
    }
    m_gameState = GameState::Over;
    emit changed();
    update();
}

void Board::setCheatModeEnabled(bool enabled)
{
    if (m_cheat == enabled) {
        return;
    }
    m_cheat = enabled;
    emit cheatStatusChanged();
}

bool Board::isOver() const
{
    return m_gameState == GameState::Over;
}

bool Board::isPaused() const
{
    return m_gameState == GameState::Paused;
}

bool Board::isStuck() const
{
    return m_gameState == GameState::Stuck;
}

bool Board::hasCheated() const
{
    return m_cheat;
}

void Board::setSoundsEnabled(bool enabled)
{
    Prefs::setSounds(enabled);
    Prefs::self()->save();
}

bool Board::isValidPos(TilePos const & tilePos) const
{
    return tilePos.x() >= 0
        && tilePos.y() >= 0
        && tilePos.x() < xTiles()
        && tilePos.y() < yTiles();
}

bool Board::isValidPosWithOutline(TilePos const & tilePos) const
{
    return tilePos.x() >= -1
        && tilePos.y() >= -1
        && tilePos.x() <= xTiles()
        && tilePos.y() <= yTiles();
}
}

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
