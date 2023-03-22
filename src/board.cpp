/*
    KShisen - A japanese game similar to Mahjongg
    SPDX-FileCopyrightText: 1997 Mario Weilguni <mweilguni@sime.com>
    SPDX-FileCopyrightText: 2002-2004 Dave Corrie <kde@davecorrie.com>
    SPDX-FileCopyrightText: 2007 Mauricio Piacentini <mauricio@tabuleiro.com>
    SPDX-FileCopyrightText: 2009-2016 Frederik Schwarzer <schwarzer@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

// KF
#include <KLocalizedString>
#include <KMessageBox>

// KShisen
#include "prefs.h"

namespace KShisen
{
#define EMPTY 0
#define SEASONS_START 28
#define FLOWERS_START 39

static std::array constexpr s_delay {1000, 750, 500, 250, 125};
static std::array constexpr s_sizeX {14, 16, 18, 24, 26, 30};
static std::array constexpr s_sizeY {6, 9, 8, 12, 14, 16};

Board::Board(QWidget * parent)
    : QWidget(parent)
    , m_random(QRandomGenerator::global()->generate())
    , m_soundPick(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("sounds/kshisen/tile-touch.ogg")))
    , m_soundFall(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("sounds/kshisen/tile-fall-tile.ogg")))
{
    m_tileRemove1.setX(-1);

    resetTimer();

    QPalette palette;
    palette.setBrush(backgroundRole(), m_background.getBackground());
    setPalette(palette);

    loadSettings();
}

void Board::loadSettings()
{
    const QString tileSetBeforeLoad = Prefs::tileSet();
    if (!loadTileset(tileSetBeforeLoad)) {
        const QString tileSetAfterLoad = Prefs::tileSet();
        if (tileSetBeforeLoad.isEmpty()) {
            // This is when the user starts the app for the first time
            if (tileSetBeforeLoad != tileSetAfterLoad) {
                // we're good the default was loaded
            } else {
                KMessageBox::error(this
                    , i18nc("%1 is a path to a tile set file", "An error occurred when loading the default tile set.\nPlease install the KMahjongg library.")
                    , i18n("Error Loading Tiles")
                );
            }
        } else {
            // This is when there was a tile set and has disappear, maybe the user or someone else removed it
            if (tileSetBeforeLoad != tileSetAfterLoad) {
                // warn the user tile set could not be loaded and the default one was loaded
                KMessageBox::information(this
                           , i18nc("%1 is a path to a tile set file", "An error occurred when loading the tile set %1. The default tile set has been loaded.", tileSetBeforeLoad)
                           , i18n("Error Loading Tiles")
                          );
            } else {
                // neither the user tile set nor the default could be loaded
                KMessageBox::error(this
                           , i18nc("%1 is a path to a tile set file", "An error occurred when loading the tile set %1. The default tile set could also not be loaded.\nPlease install the KMahjongg library.", tileSetBeforeLoad)
                           , i18n("Error Loading Tiles")
                          );
            }
        }
    }

    // Load background
    const QString backgroundBeforeLoad = Prefs::background();
    if (!loadBackground(backgroundBeforeLoad)) {
        const QString backgroundAfterLoad = Prefs::background();
        if (backgroundBeforeLoad.isEmpty()) {
            // This is when the user starts the app for the first time
            if (backgroundBeforeLoad != backgroundAfterLoad) {
                // we're good the default was loaded
            } else {
                KMessageBox::error(this
                    , i18nc("%1 is a path to a background image file", "An error occurred when loading the default background.\nPlease install the KMahjongg library.")
                    , i18n("Error Loading Tiles")
                );
            }
        } else {
            // This is when there was a background and has disappear, maybe the user or someone else removed it
            if (backgroundBeforeLoad != backgroundAfterLoad) {
                // warn the user background could not be loaded and the default one was loaded
                KMessageBox::information(this
                           , i18nc("%1 is a path to a background image file", "An error occurred when loading the background %1. The default background has been loaded.", backgroundBeforeLoad)
                           , i18n("Error Loading Tiles")
                          );
            } else {
                // neither the user background nor the default could be loaded
                KMessageBox::error(this
                           , i18nc("%1 is a path to a background image  file", "An error occurred when loading the background %1. The default background could also not be loaded.\nPlease install the KMahjongg library.", backgroundBeforeLoad)
                           , i18n("Error Loading Tiles")
                          );
            }
        }
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
    setShowUnsolvableMessageFlag(Prefs::showUnsolvableMessage());
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
            return true;
        }
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

void Board::setField(TilePos tilePos, int value)
{
    if (!isValidPos(tilePos)) {
        qCCritical(KSHISEN_General) << "Attempted write to invalid field position:"
                                    << tilePos.x()
                                    << ","
                                    << tilePos.y();
    }

    m_field.at(tilePos.y() * xTiles() + tilePos.x()) = value;
}

int Board::field(TilePos tilePos) const
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

void Board::applyGravity()
{
    if (!m_gravityFlag) {
        return;
    }
    for (decltype(xTiles()) column = 0; column < xTiles(); ++column) {
        auto rptr = yTiles() - 1;
        auto wptr = yTiles() - 1;
        while (rptr >= 0) {
            auto wptrPos = TilePos(column, wptr);
            if (field(wptrPos) != EMPTY) {
                --rptr;
                --wptr;
            } else {
                auto rptrPos = TilePos(column, rptr);
                if (field(rptrPos) != EMPTY) {
                    setField(wptrPos, field(rptrPos));
                    setField(rptrPos, EMPTY);
                    repaintTile(rptrPos);
                    repaintTile(wptrPos);
                    --wptr;
                    --rptr;
                    if (Prefs::sounds()) {
                        m_soundFall.start();
                    }
                } else {
                    --rptr;
                }
            }
        }
    }
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
    auto const oldTilePos = TilePos(m_markX, m_markY);
    m_markX = -1;
    m_markY = -1;
    repaintTile(oldTilePos);
}

void Board::mousePressEvent(QMouseEvent * e)
{
    // Do not process mouse events while the connection is drawn.
    // Clicking on one of the already connected tiles would have selected
    // it before removing it. This is more a workaround than a proper fix
    // but I have to understand the usage of m_paintConnection first in
    // order to consider its reuse here. (schwarzer)
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
    auto posX = (e->pos().x() - xOffset()) / (m_tiles.qWidth() * 2);
    auto posY = (e->pos().y() - yOffset()) / (m_tiles.qHeight() * 2);

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
        auto const clickedTile = field(TilePos(posX, posY));

        // Clear marked tile
        if (m_markX != -1 && field(TilePos(m_markX, m_markY)) != clickedTile) {
            unmarkTile();
        } else {
            m_markX = -1;
            m_markY = -1;
        }

        // Perform highlighting
        if (clickedTile != m_highlightedTile) {
            auto const oldHighlighted = m_highlightedTile;
            m_highlightedTile = clickedTile;
            for (decltype(xTiles()) i = 0; i < xTiles(); ++i) {
                for (decltype(yTiles()) j = 0; j < yTiles(); ++j) {
                    auto const fieldTile = field(TilePos(i, j));
                    if (fieldTile != EMPTY) {
                        if (fieldTile == oldHighlighted) {
                            repaintTile(TilePos(i, j));
                        } else if (fieldTile == clickedTile) {
                            repaintTile(TilePos(i, j));
                        } else if (m_chineseStyleFlag) {
                            if (isTileSeason(clickedTile) && isTileSeason(fieldTile)) {
                                repaintTile(TilePos(i, j));
                            } else if (isTileFlower(clickedTile) && isTileFlower(fieldTile)) {
                                repaintTile(TilePos(i, j));
                            }
                            // oldHighlighted
                            if (isTileSeason(oldHighlighted) && isTileSeason(fieldTile)) {
                                repaintTile(TilePos(i, j));
                            } else if (isTileFlower(oldHighlighted) && isTileFlower(fieldTile)) {
                                repaintTile(TilePos(i, j));
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
    auto tw = m_tiles.qWidth() * 2;
    return (width() - (tw * xTiles())) / 2;
}

int Board::yOffset() const
{
    auto th = m_tiles.qHeight() * 2;
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
    auto constexpr minScale = 0.2;
    auto const w = qRound(m_tiles.qWidth() * 2.0 * minScale) * xTiles() + m_tiles.width();
    auto const h = qRound(m_tiles.qHeight() * 2.0 * minScale) * yTiles() + m_tiles.height();

    setMinimumSize(w, h);

    resizeBoard();
    newGame();
    Q_EMIT changed();
}

void Board::resizeEvent(QResizeEvent * e)
{
    qCDebug(KSHISEN_General) << "[resizeEvent]";
    if (e->spontaneous()) {
        qCDebug(KSHISEN_General) << "[resizeEvent] spontaneous";
    }
    resizeBoard();
    Q_EMIT resized();
}

void Board::resizeBoard()
{
    // calculate tile size required to fit all tiles in the window
    auto const newsize = m_tiles.preferredTileSize(QSize(width(), height()), xTiles(), yTiles());
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
    auto curTile = 1;
    auto tileCount = 0;

    /*
     * Note by jwickers: i changed the way to distribute tiles
     *  in chinese mahjongg there are 4 tiles of each
     *  except flowers and seasons (4 flowers and 4 seasons,
     *  but one unique tile of each, that is why they are
     *  the only ones numbered)
     * That uses the chineseStyle flag
     */
    for (decltype(yTiles()) y = 0; y < yTiles(); ++y) {
        for (decltype(xTiles()) x = 0; x < xTiles(); ++x) {
            // do not duplicate flowers or seasons
            if (!m_chineseStyleFlag || !(isTileSeason(curTile) || isTileFlower(curTile))) {
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
        Q_EMIT newGameStarted();
        Q_EMIT changed();
        return;
    }

    // shuffle the field
    auto const tx = xTiles();
    auto const ty = yTiles();
    for (decltype(tx * ty * m_shuffle) i = 0; i < tx * ty * m_shuffle; ++i) {
        auto const tilePos1 = TilePos(m_random.bounded(tx), m_random.bounded(ty));
        auto const tilePos2 = TilePos(m_random.bounded(tx), m_random.bounded(ty));
        // keep and use t, because the next setField() call changes what field() will return
        // so there would a significant impact on shuffling with the field() call put into the
        // place where 't' is used
        auto const t = field(tilePos1);
        setField(tilePos1, field(tilePos2));
        setField(tilePos2, t);
    }

    // if m_solvableFlag is false, the game does not need to be solvable; we can drop out here
    if (!m_solvableFlag) {
        update();
        resetTimer();
        Q_EMIT newGameStarted();
        Q_EMIT changed();
        return;
    }


    auto oldfield = m_field;
    decltype(m_field) tiles(m_field.size());
    decltype(m_field) pos(m_field.size());
    //jwickers: in case the game cannot made solvable we do not want to run an infinite loop
    auto maxAttempts = 200;

    while (!isSolvable(false) && maxAttempts > 0) {
        // generate a list of free tiles and positions
        auto numberOfTiles = 0;
        for (decltype(xTiles() * yTiles()) i = 0; i < xTiles() * yTiles(); ++i) {
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
            auto const r1 = m_random.bounded(numberOfTiles);
            auto const r2 = m_random.bounded(numberOfTiles);
            auto const tile = tiles.at(r1);
            auto const apos = pos.at(r2);

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
    Q_EMIT changed();
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
        if (isTileSeason(tile1) && isTileSeason(tile2)) {
            return true;
        }
        // if both tiles are flowers
        if (isTileFlower(tile1) && isTileFlower(tile2)) {
            return true;
        }
    }
    return false;
}

bool Board::isTileHighlighted(TilePos tilePos) const
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

void Board::repaintTile(TilePos tilePos)
{
    auto const r = QRect(xOffset() + tilePos.x() * m_tiles.qWidth() * 2,
                         yOffset() + tilePos.y() * m_tiles.qHeight() * 2,
                         m_tiles.width(),
                         m_tiles.height());

    update(r);
}

void Board::showInfoRect(QPainter & p, QString const & message)
{
    auto const boxWidth = width() * 0.6;
    auto const boxHeight = height() * 0.6;
    auto const contentsRect = QRect((width() - boxWidth) / 2, (height() - boxHeight) / 2, boxWidth, boxHeight);
    QFont font;
    auto const fontsize = static_cast<int>(boxHeight / 13);
    font.setPointSize(fontsize);
    p.setFont(font);
    p.setBrush(QBrush(QColor(100, 100, 100, 150)));
    p.setRenderHint(QPainter::Antialiasing);
    p.drawRoundedRect(contentsRect, 10, 10);

    p.drawText(contentsRect, Qt::AlignCenter | Qt::TextWordWrap, message);
}

void Board::drawTiles(QPainter & p, QPaintEvent * e)
{
    auto const w = m_tiles.width();
    auto const h = m_tiles.height();
    auto const fw = m_tiles.qWidth() * 2;
    auto const fh = m_tiles.qHeight() * 2;
    for (decltype(xTiles()) i = 0; i < xTiles(); ++i) {
        for (decltype(yTiles()) j = 0; j < yTiles(); ++j) {
            auto const tile = field(TilePos(i, j));
            if (tile == EMPTY) {
                continue;
            }

            auto const xpos = xOffset() + i * fw;
            auto const ypos = yOffset() + j * fh;
            auto const r = QRect(xpos, ypos, w, h);
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
    auto const ur = e->rect(); // rectangle to update
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
        for (auto const &move : std::as_const(m_possibleMoves)) {
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

void Board::reverseSlide(TilePos tilePos, Slide const & slide)
{
    // slide[XY]2 is the current location of the last tile to slide
    // slide[XY]1 is its destination
    // calculate the offset for the tiles to slide
    auto const dx = slide.front().x() - slide.back().x();
    auto const dy = slide.front().y() - slide.back().y();
    auto currentTile = 0;
    // move all tiles between slideX2, slideY2 and x, y to slide with that offset
    if (dx == 0) {
        if (tilePos.y() < slide.back().y()) {
            for (auto i = tilePos.y() + 1; i <= slide.back().y(); ++i) {
                currentTile = field(TilePos(tilePos.x(), i));
                if (currentTile == EMPTY) {
                    continue;
                }
                setField(TilePos(tilePos.x(), i), EMPTY);
                setField(TilePos(tilePos.x(), i + dy), currentTile);
                repaintTile(TilePos(tilePos.x(), i));
                repaintTile(TilePos(tilePos.x(), i + dy));
            }
        } else {
            for (auto i = tilePos.y() - 1; i >= slide.back().y(); --i) {
                currentTile = field(TilePos(tilePos.x(), i));
                if (currentTile == EMPTY) {
                    continue;
                }
                setField(TilePos(tilePos.x(), i), EMPTY);
                setField(TilePos(tilePos.x(), i + dy), currentTile);
                repaintTile(TilePos(tilePos.x(), i));
                repaintTile(TilePos(tilePos.x(), i + dy));
            }
        }
    } else if (dy == 0) {
        if (tilePos.x() < slide.back().x()) {
            for (auto i = tilePos.x() + 1; i <= slide.back().x(); ++i) {
                currentTile = field(TilePos(i, tilePos.y()));
                if (currentTile == EMPTY) {
                    continue;
                }
                setField(TilePos(i, tilePos.y()), EMPTY);
                setField(TilePos(i + dx, tilePos.y()), currentTile);
                repaintTile(TilePos(i, tilePos.y()));
                repaintTile(TilePos(i + dx, tilePos.y()));
            }
        } else {
            for (auto i = tilePos.x() - 1; i >= slide.back().x(); --i) {
                currentTile = field(TilePos(i, tilePos.y()));
                if (currentTile == EMPTY) {
                    continue;
                }
                setField(TilePos(i, tilePos.y()), EMPTY);
                setField(TilePos(i + dx, tilePos.y()), currentTile);
                repaintTile(TilePos(i, tilePos.y()));
                repaintTile(TilePos(i + dx, tilePos.y()));
            }
        }
    }
}

void Board::performSlide(TilePos tilePos, Slide const & slide)
{
    // check if there is something to slide
    if (slide.empty()) {
        return;
    }

    // slide.first is the current location of the last tile to slide
    // slide.last is its destination
    // calculate the offset for the tiles to slide
    auto const dx = slide.back().x() - slide.front().x();
    auto const dy = slide.back().y() - slide.front().y();
    auto currentTile = 0;
    // move all tiles between m_markX, m_markY and the last tile to slide with that offset
    if (dx == 0) {
        if (tilePos.y() < slide.front().y()) {
            for (auto i = slide.front().y(); i > tilePos.y(); --i) {
                currentTile = field(TilePos(tilePos.x(), i));
                setField(TilePos(tilePos.x(), i), EMPTY);
                setField(TilePos(tilePos.x(), i + dy), currentTile);
                repaintTile(TilePos(tilePos.x(), i));
                repaintTile(TilePos(tilePos.x(), i + dy));
            }
        } else {
            for (auto i = slide.front().y(); i < tilePos.y(); ++i) {
                currentTile = field(TilePos(tilePos.x(), i));
                setField(TilePos(tilePos.x(), i), EMPTY);
                setField(TilePos(tilePos.x(), i + dy), currentTile);
                repaintTile(TilePos(tilePos.x(), i));
                repaintTile(TilePos(tilePos.x(), i + dy));
            }
        }
    } else if (dy == 0) {
        if (tilePos.x() < slide.front().x()) {
            for (auto i = slide.front().x(); i > tilePos.x(); --i) {
                currentTile = field(TilePos(i, tilePos.y()));
                setField(TilePos(i, tilePos.y()), EMPTY);
                setField(TilePos(i + dx, tilePos.y()), currentTile);
                repaintTile(TilePos(i, tilePos.y()));
                repaintTile(TilePos(i + dx, tilePos.y()));
            }
        } else {
            for (auto i = slide.front().x(); i < tilePos.x(); ++i) {
                currentTile = field(TilePos(i, tilePos.y()));
                setField(TilePos(i, tilePos.y()), EMPTY);
                setField(TilePos(i + dx, tilePos.y()), currentTile);
                repaintTile(TilePos(i, tilePos.y()));
                repaintTile(TilePos(i + dx, tilePos.y()));
            }
        }
    }
}

void Board::performMove(PossibleMove & possibleMoves)
{
    m_connection = possibleMoves.path();
#ifdef DEBUGGING
    // DEBUG undo, save board state
    auto saved1 = m_field;
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
    auto saved2 = m_field;
    decltype(m_field) saved3; // after undo
    decltype(m_field) saved4; // after redo
    // DEBUG undo, undo move
    auto errorFound = false;
    if (canUndo()) {
        undo();
        // DEBUG undo, compare to saved board state
        for (decltype(xTiles() * yTiles()) i = 0; i < xTiles() * yTiles(); ++i) {
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
            for (decltype(xTiles() * yTiles()) i = 0; i < xTiles() * yTiles(); ++i) {
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

void Board::marked(TilePos tilePos)
{
    if (field(tilePos) == EMPTY) { // click on empty space on the board
        if (m_possibleMoves.size() > 1) { // if the click is on any of the current possible moves, make that move
            for (auto move : std::as_const(m_possibleMoves)) {
                if (move.isInPath(tilePos)) {
                    performMove(move);
                    Q_EMIT selectATile();
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
        Q_EMIT selectATile();
        return;
    }

    if (m_markX == -1) { // nothing is selected so far
        m_markX = tilePos.x();
        m_markY = tilePos.y();
        drawPossibleMoves(false);
        m_possibleMoves.clear();
        repaintTile(tilePos);
        Q_EMIT selectAMatchingTile();
        return;
    }
    if (m_possibleMoves.size() > 1) { // if the click is on any of the current possible moves, make that move

        for (auto move : std::as_const(m_possibleMoves)) {
            if (move.isInPath(tilePos)) {
                performMove(move);
                Q_EMIT selectATile();
                return;
            }
        }
    }

    auto const tile1 = field(TilePos(m_markX, m_markY));
    auto const tile2 = field(tilePos);

    // both tiles do not match
    if (!tilesMatch(tile1, tile2)) {
        unmarkTile();
        Q_EMIT tilesDoNotMatch();
        return;
    }

    // trace and perform the move and get the list of possible moves
    if (findPath(TilePos(m_markX, m_markY), tilePos, m_possibleMoves) > 0) {
        if (m_possibleMoves.size() > 1) {
            auto withSlide = 0;
            for (auto const &move : std::as_const(m_possibleMoves)) {
                move.Debug();
                if (move.hasSlide()) {
                    ++withSlide;
                }
            }
            // if all moves have no slide, it doesn't matter
            if (withSlide > 0) {
                drawPossibleMoves(true);
                Q_EMIT selectAMove();
                return;
            }
        }

        // only one move possible, perform it
        performMove(m_possibleMoves.front());
        Q_EMIT selectATile();
        // game is over?
        // Must delay until after tiles fall to make this test
        // See undrawConnection GP.
    } else {
        Q_EMIT invalidMove();
        m_connection.clear();
    }
}


void Board::clearHighlight()
{
    if (m_highlightedTile == -1) {
        return;
    }
    auto const oldHighlighted = m_highlightedTile;
    m_highlightedTile = -1;

    for (decltype(xTiles()) i = 0; i < xTiles(); ++i) {
        for (decltype(yTiles()) j = 0; j < yTiles(); ++j) {
            if (tilesMatch(oldHighlighted, field(TilePos(i, j)))) {
                repaintTile(TilePos(i, j));
            }
        }
    }
}

bool Board::canMakePath(TilePos tilePos1, TilePos tilePos2) const
{
    if (tilePos1.x() == tilePos2.x()) {
        for (auto i = qMin(tilePos1.y(), tilePos2.y()) + 1; i < qMax(tilePos1.y(), tilePos2.y()); ++i) {
            if (field(TilePos(tilePos1.x(), i)) != EMPTY) {
                return false;
            }
        }
        return true;
    }

    if (tilePos1.y() == tilePos2.y()) {
        for (auto i = qMin(tilePos1.x(), tilePos2.x()) + 1; i < qMax(tilePos1.x(), tilePos2.x()); ++i) {
            if (field(TilePos(i, tilePos1.y())) != EMPTY) {
                return false;
            }
        }
        return true;
    }

    return false;
}

bool Board::canSlideTiles(TilePos tilePos1, TilePos tilePos2, Slide & slide) const
{
    auto distance = -1;
    slide.clear();
    if (tilePos1.x() == tilePos2.x()) {
        if (tilePos1.y() > tilePos2.y()) {
            distance = tilePos1.y() - tilePos2.y();
            // count how much free space we have for sliding
            auto startFree = -1;
            auto endFree = -1;
            // find first tile empty
            for (auto i = tilePos1.y() - 1; i >= 0; --i) {
                if (field(TilePos(tilePos1.x(), i)) == EMPTY) {
                    startFree = i;
                    break;
                }
            }
            // if not found, cannot slide
            // if the first free tile is just next to the sliding tile, no slide (should be a normal move)
            if (startFree == -1 || startFree == (tilePos1.y() - 1)) {
                return false;
            }
            // find last tile empty
            for (auto i = startFree - 1; i >= 0; --i) {
                if (field(TilePos(tilePos1.x(), i)) != EMPTY) {
                    endFree = i;
                    break;
                }
            }
            // if not found, it is the border: 0

            // so we can slide of start_free - end_free, compare this to the distance
            if (distance <= (startFree - endFree)) {
                // first position of the last slided tile
                slide.push_back(TilePos(tilePos1.x(), startFree + 1));
                // final position of the last slided tile
                slide.push_back(TilePos(tilePos1.x(), startFree + 1 - distance));
                return true;
            }
            return false;

        } else if (tilePos2.y() > tilePos1.y()) {
            distance = tilePos2.y() - tilePos1.y();
            // count how much free space we have for sliding
            auto startFree = -1;
            auto endFree = yTiles();
            // find first tile empty
            for (auto i = tilePos1.y() + 1; i < yTiles(); ++i) {
                if (field(TilePos(tilePos1.x(), i)) == EMPTY) {
                    startFree = i;
                    break;
                }
            }
            // if not found, cannot slide
            // if the first free tile is just next to the sliding tile, no slide (should be a normal move)
            if (startFree == -1 || startFree == tilePos1.y() + 1) {
                return false;
            }
            // find last tile empty
            for (auto i = startFree + 1; i < yTiles(); ++i) {
                if (field(TilePos(tilePos1.x(), i)) != EMPTY) {
                    endFree = i;
                    break;
                }
            }
            // if not found, it is the border: yTiles()-1

            // so we can slide of end_free - start_free, compare this to the distance
            if (distance <= (endFree - startFree)) {
                // first position of the last slidden tile
                slide.push_back(TilePos(tilePos1.x(), startFree - 1));
                // final position of the last slidden tile
                slide.push_back(TilePos(tilePos1.x(), startFree - 1 + distance));
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
            auto startFree = -1;
            auto endFree = -1;
            // find first tile empty
            for (auto i = tilePos1.x() - 1; i >= 0; --i) {
                if (field(TilePos(i, tilePos1.y())) == EMPTY) {
                    startFree = i;
                    break;
                }
            }
            // if not found, cannot slide
            // if the first free tile is just next to the sliding tile, no slide (should be a normal move)
            if (startFree == -1 || startFree == tilePos1.x() - 1) {
                return false;
            }
            // find last tile empty
            for (auto i = startFree - 1; i >= 0; --i) {
                if (field(TilePos(i, tilePos1.y())) != EMPTY) {
                    endFree = i;
                    break;
                }
            }
            // if not found, it is the border: 0

            // so we can slide of start_free - end_free, compare this to the distance
            if (distance <= (startFree - endFree)) {
                // first position of the last slidden tile
                slide.push_back(TilePos(startFree + 1, tilePos1.y()));
                // final position of the last slidden tile
                slide.push_back(TilePos(startFree + 1 - distance, tilePos1.y()));
                return true;
            }
            return false;

        } else if (tilePos2.x() > tilePos1.x()) {
            distance = tilePos2.x() - tilePos1.x();
            // count how much free space we have for sliding
            auto startFree = -1;
            auto endFree = xTiles();
            // find first tile empty
            for (auto i = tilePos1.x() + 1; i < xTiles(); ++i) {
                if (field(TilePos(i, tilePos1.y())) == EMPTY) {
                    startFree = i;
                    break;
                }
            }
            // if not found, cannot slide
            // if the first free tile is just next to the sliding tile, no slide (should be a normal move)
            if (startFree == -1 || startFree == tilePos1.x() + 1) {
                return false;
            }
            // find last tile empty
            for (auto i = startFree + 1; i < xTiles(); ++i) {
                if (field(TilePos(i, tilePos1.y())) != EMPTY) {
                    endFree = i;
                    break;
                }
            }
            // if not found, it is the border: xTiles()-1

            // so we can slide of endFree - startFree, compare this to the distance
            if (distance <= (endFree - startFree)) {
                // first position of the last slidden tile
                slide.push_back(TilePos(startFree - 1, tilePos1.y()));
                // final position of the last slidden tile
                slide.push_back(TilePos(startFree - 1 + distance, tilePos1.y()));
                return true;
            }
            return false;
        }
        // x1 == x2 ?!
        return false;
    }
    return false;
}

int Board::findPath(TilePos tilePos1, TilePos tilePos2, PossibleMoves & possibleMoves) const
{
    possibleMoves.clear();

    auto simplePath = 0;

    // first find the simple paths
    auto numberOfPaths = findSimplePath(tilePos1, tilePos2, possibleMoves);

    // if the tiles can slide, 2 lines max is allowed
    if (m_tilesCanSlideFlag) {
        return numberOfPaths;
    }

    // Find paths of 3 segments
    std::array constexpr dx {1, 0, -1, 0};
    std::array constexpr dy {0, 1, 0, -1};

    for (auto i = 0; i < 4; ++i) {
        auto tempX = tilePos1.x() + dx.at(i);
        auto tempY = tilePos1.y() + dy.at(i);
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

int Board::findSimplePath(TilePos tilePos1, TilePos tilePos2, PossibleMoves & possibleMoves) const
{
    auto numberOfPaths = 0;
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
        Slide slide;
        // Find path of 2 segments (route A)
        if (canSlideTiles(tilePos1, TilePos(tilePos2.x(), tilePos1.y()), slide)
            && canMakePath(TilePos(tilePos2.x(), tilePos1.y()), tilePos2)) {
            path.clear();
            path.push_back(tilePos1);
            path.push_back(TilePos(tilePos2.x(), tilePos1.y()));
            path.push_back(tilePos2);
            possibleMoves.push_back(PossibleMove(path, slide));
            ++numberOfPaths;
        }

        // Find path of 2 segments (route B)
        if (canSlideTiles(tilePos1, TilePos(tilePos1.x(), tilePos2.y()), slide)
            && canMakePath(TilePos(tilePos1.x(), tilePos2.y()), tilePos2)) {
            path.clear();
            path.push_back(tilePos1);
            path.push_back(TilePos(tilePos1.x(), tilePos2.y()));
            path.push_back(tilePos2);
            possibleMoves.push_back(PossibleMove(path, slide));
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

    auto const tile1 = TilePos(m_connection.front().x(), m_connection.front().y());
    auto const tile2 = TilePos(m_connection.back().x(), m_connection.back().y());
    // lighten the fields
    repaintTile(tile1);
    repaintTile(tile2);

    m_paintConnection = true;
    update();
}

void Board::undrawConnection()
{
    if (m_tileRemove1.x() != -1) {
        setField(m_tileRemove1, EMPTY);
        setField(m_tileRemove2, EMPTY);
        applyGravity();
        m_tileRemove1.setX(-1);
        update();
        Q_EMIT tileCountChanged();
    }

    // is already undrawn?
    if (m_connection.empty()) {
        return;
    }

    // Redraw all affected fields
    auto const oldConnection = m_connection;
    m_connection.clear();
    m_paintConnection = false;

    auto pt1 = oldConnection.cbegin();
    auto pt2 = pt1 + 1; // TODO: std::next?
    while (pt2 != oldConnection.cend()) {
        if (pt1->y() == pt2->y()) {
            for (auto i = qMin(pt1->x(), pt2->x()); i <= qMax(pt1->x(), pt2->x()); ++i) {
                repaintTile(TilePos(i, pt1->y()));
            }
        } else {
            for (auto i = qMin(pt1->y(), pt2->y()); i <= qMax(pt1->y(), pt2->y()); ++i) {
                repaintTile(TilePos(pt1->x(), i));
            }
        }
        ++pt1;
        ++pt2;
    }

    PossibleMoves dummyPossibleMoves;
    // game is over?
    if ((!pathFoundBetweenMatchingTiles(dummyPossibleMoves)
         && m_showUnsolvableMessageFlag)
        || (tilesLeft() == 0)) {
        m_gameClock.pause();
        Q_EMIT endOfGame();
    }
    m_paintInProgress = false;
}

QPoint Board::midCoord(TilePos tilePos) const
{
    QPoint p;
    auto const w = m_tiles.qWidth() * 2;
    auto const h = m_tiles.qHeight() * 2;

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

void Board::madeMove(TilePos tilePos1, TilePos tilePos2, Slide slide)
{
    std::unique_ptr<Move> move;
    if (slide.empty()) {
        move = std::make_unique<Move>(tilePos1, tilePos2, field(tilePos1), field(tilePos2));
    } else {
        move = std::make_unique<Move>(tilePos1, tilePos2, field(tilePos1), field(tilePos2), slide);
    }
    m_undo.push_back(std::move(move));
    if (!m_redo.empty()) {
        m_redo.clear();
    }
    Q_EMIT changed();
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
    auto move = std::move(m_undo.back());
    m_undo.pop_back();
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
            for (decltype(move->y1()) y = 0; y < move->y1(); ++y) {
                setField(TilePos(move->x1(), y), field(TilePos(move->x1(), y + 1)));
                repaintTile(TilePos(move->x1(), y));
            }

            // move tiles from the second column up
            for (decltype(move->y2()) y = 0; y < move->y2(); ++y) {
                setField(TilePos(move->x2(), y), field(TilePos(move->x2(), y + 1)));
                repaintTile(TilePos(move->x2(), y));
            }
        } else { // else check all tiles from the slide that may have fallen down

            qCDebug(KSHISEN_General) << "[undo] gravity from slide s1(" << move->slideX1() << "," << move->slideY1() << ")=>s2(" << move->slideX2() << "," << move->slideY2() << ") matching (" << move->x1() << "," << move->y1() << ")=>(" << move->x2() << "," << move->y2() << ")";

            // horizontal slide
            // because tiles that slides horizontally may fall down
            // in columns different than the taken tiles columns
            // we need to take them back up then undo the slide
            if (move->slideY1() == move->slideY2()) {
                qCDebug(KSHISEN_General) << "[undo] gravity from horizontal slide";

                // last slide tile went from slideX1() -> slideX2()
                // the number of slidden tiles is n = abs(x1 - slideX1())
                auto n = move->x1() - move->slideX1();
                if (n < 0) {
                    n = -n;
                }
                // distance slidden is
                auto dx = move->slideX2() - move->slideX1();
                if (dx < 0) {
                    dx = -dx;
                }

                qCDebug(KSHISEN_General) << "[undo] n =" << n;

                // slidden tiles may fall down after the slide
                // so any tiles on top of the columns between
                // slideX2() -> slideX2() +/- n (excluded) should go up to slideY1()
                if (move->slideX2() > move->slideX1()) { // slide to the right

                    qCDebug(KSHISEN_General) << "[undo] slide right";

                    for (auto i = move->slideX2(); i > move->slideX2() - n; --i) {
                        // find top tile
                        decltype(yTiles()) j = 0;
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
                        repaintTile(TilePos(i, j));
                        repaintTile(TilePos(i, move->slideY1()));
                    }
                } else { // slide to the left

                    qCDebug(KSHISEN_General) << "[undo] slide left";

                    for (auto i = move->slideX2(); i < move->slideX2() + n; ++i) {
                        // find top tile
                        decltype(yTiles()) j = 0;
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
                        repaintTile(TilePos(i, j));
                        repaintTile(TilePos(i, move->slideY1()));
                    }
                }

                qCDebug(KSHISEN_General) << "[undo] moving up column x2" << move->x2();

                // move tiles from the second column up
                for (decltype(move->y2()) y = 0; y <= move->y2(); ++y) {
                    qCDebug(KSHISEN_General) << "[undo] moving up tile" << y + 1;

                    setField(TilePos(move->x2(), y), field(TilePos(move->x2(), y + 1)));
                    repaintTile(TilePos(move->x2(), y));
                }
                // and all columns that fell after the tiles slidden between
                // only if they were not replaced by a sliding tile !!
                // x1 -> x1+dx should go up one
                // if their height > slideY1()
                // because they have fallen after the slide
                if (move->slideX2() > move->slideX1()) { // slide to the right
                    if (move->slideY1() > 0) {
                        for (auto i = move->x1() + dx; i >= move->x1(); --i) {
                            qCDebug(KSHISEN_General) << "[undo] moving up column" << i << "until" << move->slideY1();

                            for (decltype(move->slideY1()) j = 0; j < move->slideY1(); ++j) {
                                qCDebug(KSHISEN_General) << "[undo] moving up tile" << j + 1;

                                setField(TilePos(i, j), field(TilePos(i, j + 1)));
                                repaintTile(TilePos(i, j));
                            }

                            qCDebug(KSHISEN_General) << "[undo] clearing last tile" << move->slideY1();

                            setField(TilePos(i, move->slideY1()), EMPTY);
                            repaintTile(TilePos(i, move->slideY1()));
                        }
                    }
                } else { // slide to the left
                    if (move->slideY1() > 0) {
                        for (auto i = move->x1() - dx; i <= move->x1(); ++i) {
                            qCDebug(KSHISEN_General) << "[undo] moving up column" << i << "until" << move->slideY1();

                            for (decltype(move->slideY1()) j = 0; j < move->slideY1(); ++j) {
                                qCDebug(KSHISEN_General) << "[undo] moving up tile" << j + 1;

                                setField(TilePos(i, j), field(TilePos(i, j + 1)));
                                repaintTile(TilePos(i, j));
                            }

                            qCDebug(KSHISEN_General) << "[undo] clearing last tile" << move->slideY1();

                            setField(TilePos(i, move->slideY1()), EMPTY);
                            repaintTile(TilePos(i, move->slideY1()));
                        }
                    }
                }

                qCDebug(KSHISEN_General) << "[undo] reversing slide";

                // then undo the slide to put the tiles back to their original location
                reverseSlide(TilePos(move->x1(), move->y1()), move->slide());

            } else {
                qCDebug(KSHISEN_General) << "[undo] gravity from vertical slide";

                // vertical slide, in fact nothing special is necessary
                // the default implementation works because it only affects
                // the two columns were tiles were taken

                // move tiles from the first column up
                for (decltype(move->y1()) y = 0; y < move->y1(); ++y) {
                    setField(TilePos(move->x1(), y), field(TilePos(move->x1(), y + 1)));
                    repaintTile(TilePos(move->x1(), y));
                }

                // move tiles from the second column up
                for (decltype(move->y2()) y = 0; y < move->y2(); ++y) {
                    setField(TilePos(move->x2(), y), field(TilePos(move->x2(), y + 1)));
                    repaintTile(TilePos(move->x2(), y));
                }
            }
        }
    } else { // no gravity
        // undo slide if any
        if (move->hasSlide()) {
            // perform the slide in reverse
            reverseSlide(TilePos(move->x1(), move->y1()), move->slide());
        }
    }

    // replace taken tiles
    setField(TilePos(move->x1(), move->y1()), move->tile1());
    setField(TilePos(move->x2(), move->y2()), move->tile2());
    repaintTile(TilePos(move->x1(), move->y1()));
    repaintTile(TilePos(move->x2(), move->y2()));

    m_redo.push_front(std::move(move));
    Q_EMIT changed();
}

void Board::redo()
{
    if (canRedo()) {
        clearHighlight();
        undrawConnection();
        auto move = std::move(m_redo.front());
        m_redo.pop_front();
        // redo the slide if any
        if (move->hasSlide()) {
            Slide s;
            s.push_back(TilePos(move->slideX1(), move->slideY1()));
            s.push_back(TilePos(move->slideX2(), move->slideY2()));
            performSlide(TilePos(move->x1(), move->y1()), s);
        }
        setField(TilePos(move->x1(), move->y1()), EMPTY);
        setField(TilePos(move->x2(), move->y2()), EMPTY);
        repaintTile(TilePos(move->x1(), move->y1()));
        repaintTile(TilePos(move->x2(), move->y2()));
        applyGravity();
        m_undo.push_back(std::move(move));
        Q_EMIT changed();
    }
}

void Board::showHint()
{
    undrawConnection();

    if (pathFoundBetweenMatchingTiles(m_possibleMoves)) {
        m_connection = m_possibleMoves.front().path();
        drawConnection();
    }
}

#ifdef DEBUGGING
void Board::makeHintMove()
{
    PossibleMoves possibleMoves;

    if (pathFoundBetweenMatchingTiles(possibleMoves)) {
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

void Board::dumpBoard(const std::vector<int> & field) const
{
    qCDebug(KSHISEN_General) << "Board contents:";
    for (decltype(yTiles()) y = 0; y < yTiles(); ++y) {
        QString row;
        for (decltype(xTiles()) x = 0; x < xTiles(); ++x) {
            auto tile = field.at(y * xTiles() + x);
            if (tile == EMPTY) {
                row += QLatin1String(" --");
            } else {
                row += QStringLiteral("%1").arg(tile, 3);
            }
        }
        qCDebug(KSHISEN_General) << row;
    }
}
#endif

int Board::lineWidth() const
{
    auto width = qRound(m_tiles.height() / 10.0);
    if (width < 3) {
        width = 3;
    }

    return width;
}

bool Board::pathFoundBetweenMatchingTiles(PossibleMoves & possibleMoves) const
{
    std::array<short, Board::nTiles> done{};

    for (decltype(xTiles()) x = 0; x < xTiles(); ++x) {
        for (decltype(yTiles()) y = 0; y < yTiles(); ++y) {
            auto const tile = field(TilePos(x, y));
            if (tile != EMPTY && done.at(tile - 1) != 4) {
                // for all these types of tile search paths
                for (decltype(xTiles()) xx = 0; xx < xTiles(); ++xx) {
                    for (decltype(yTiles()) yy = 0; yy < yTiles(); ++yy) {
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
    return std::count_if(m_field.begin(), m_field.end(), [](auto field) { return field != EMPTY; });
}

int Board::currentTime() const
{
    return m_gameClock.seconds();
}

bool Board::isSolvable(bool restore)
{
    decltype(m_field) oldField;

    if (!restore) {
        oldField = m_field;
    }

    PossibleMoves p;
    while (pathFoundBetweenMatchingTiles(p)) {
        auto const tile1 = TilePos(p.front().path().front().x(), p.front().path().front().y());
        auto const tile2 = TilePos(p.front().path().back().x(), p.front().path().back().y());
        if (!tilesMatch(field(tile1), field(tile2))) {
            auto const errMessage = QStringLiteral("Removing unmatched tiles: (%1,%2) => %3 (%4,%5) => %6")
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

    auto const left = tilesLeft();

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

bool Board::showUnsolvableMessageFlag() const
{
    return m_showUnsolvableMessageFlag;
}

void Board::setShowUnsolvableMessageFlag(bool enabled)
{
    if (m_showUnsolvableMessageFlag == enabled) {
        return;
    }
    m_showUnsolvableMessageFlag = enabled;
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
    Q_EMIT changed();
    update();
}

QSize Board::sizeHint() const
{
    auto dpi = logicalDpiX();
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
    m_undo.clear();
}

void Board::resetRedo()
{
    if (!canRedo()) {
        return;
    }
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
    Q_EMIT changed();
    update();
}

void Board::setGameOverEnabled(bool enabled)
{
    if (m_gameState == GameState::Over && enabled) {
        return;
    }
    m_gameState = GameState::Over;
    Q_EMIT changed();
    update();
}

void Board::setCheatModeEnabled(bool enabled)
{
    if (m_cheat == enabled) {
        return;
    }
    m_cheat = enabled;
    Q_EMIT cheatStatusChanged();
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

bool Board::isValidPos(TilePos tilePos) const
{
    return tilePos.x() >= 0
        && tilePos.y() >= 0
        && tilePos.x() < xTiles()
        && tilePos.y() < yTiles();
}

bool Board::isValidPosWithOutline(TilePos tilePos) const
{
    return tilePos.x() >= -1
        && tilePos.y() >= -1
        && tilePos.x() <= xTiles()
        && tilePos.y() <= yTiles();
}

bool Board::isTileFlower(int tile) const
{
    return tile >= FLOWERS_START && tile <= (FLOWERS_START + 3);
}

bool Board::isTileSeason(int tile) const
{
    return tile >= SEASONS_START && tile <= (SEASONS_START + 3);
}
} // namespace KShisen

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
