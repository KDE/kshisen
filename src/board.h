/*
    KShisen - A japanese game similar to Mahjongg
    SPDX-FileCopyrightText: 1997 Mario Weilguni <mweilguni@sime.com>
    SPDX-FileCopyrightText: 2002-2004 Dave Corrie <kde@davecorrie.com>
    SPDX-FileCopyrightText: 2007 Mauricio Piacentini <mauricio@tabuleiro.com>
    SPDX-FileCopyrightText: 2009-2016 Frederik Schwarzer <schwarzer@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// KMahjonggLib integration and SVG support for KDE 4: Mauricio Piacentini <mauricio@tabuleiro.com>

#ifndef KSHISEN_BOARD_H
#define KSHISEN_BOARD_H

// STL
#include <memory>
#include <vector>

// Qt
#include <QList>
#include <QSize>
#include <QWidget>
#include <QRandomGenerator>

// KDEGames
#include <KGameClock>
#include <KGameSound>

// LibKMahjongg
#include <KMahjonggBackground>
#include <KMahjonggTileset>

// KShisen
#include "debug.h"
#include "move.h"
#include "possiblemove.h"

namespace KShisen
{
/**
 * A list of possible moves the player has to choose between
 */
using PossibleMoves = QList<PossibleMove>;

/**
 * @brief Class holding the game board and its functions.
 */
class Board : public QWidget
{
    Q_OBJECT

public:
    explicit Board(QWidget * parent = nullptr);

    /// Number of different kinds of tiles in the game.
    static int constexpr nTiles = 42;

    void paintEvent(QPaintEvent * e) override;
    void mousePressEvent(QMouseEvent * e) override;
    void resizeEvent(QResizeEvent * e) override;

    void setDelay(int);
    int delay() const;

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

    void showHint();
    bool pathFoundBetweenMatchingTiles(PossibleMoves & possibleMoves) const;

#ifdef DEBUGGING
    void makeHintMove();
    void finish();
    void dumpBoard() const;
    void dumpBoard(const std::vector<int> & field) const;
#endif

    /// Returns the number of tiles left on the board
    int tilesLeft() const;
    /// Returns the current game time in seconds
    int currentTime() const;

    /// Returns whether the current game is solvable
    bool isSolvable(bool restore); // const?

    bool solvableFlag() const;
    void setSolvableFlag(bool enabled);
    bool showUnsolvableMessageFlag() const;
    void setShowUnsolvableMessageFlag(bool enabled);
    bool gravityFlag() const;
    void setGravityFlag(bool enabled);
    void setChineseStyleFlag(bool enabled);
    void setTilesCanSlideFlag(bool enabled);

    /// Returns possible number of tiles in X direction
    int xTiles() const;
    /// Returns possible number of tiles in Y direction
    int yTiles() const;
    /// Returns overall possible number of tiles in current game board size
    int tiles() const;

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

    /** Returns whether the game is over.
     * @return True if game is over, False if game is not over
     */
    bool isOver() const;

    /** Returns whether the game is in pause mode.
     * @return True if game is paused, False if game is not paused
     */
    bool isPaused() const;

    /** Returns whether there are still matching tiles left.
     * @return True if there are no matching tiles left, False if there are matching tiles left
     */
    bool isStuck() const;

    /** Returns whether player is in cheat mode.
    * @return True if the player is in cheat mode, False if not
    */
    bool hasCheated() const;

Q_SIGNALS:
    void markMatched(); // unused?
    void newGameStarted();
    void changed();
    void tileCountChanged();
    void endOfGame();
    void resized();
    void invalidMove();
    void tilesDoNotMatch();
    void selectATile();
    void selectAMove();
    void selectAMatchingTile();
    void cheatStatusChanged();

public Q_SLOTS:
    /** Does most of the newGame work.
     * This slot is called from the KShisen::invokeNewGame() signal from KShisen and
     * should call KShisen::newGame again to do the work that cannot be done
     * from Board.
     */
    void newGame();

    /// Controls the pause mode
    void setPauseEnabled(bool enabled);

    /** Enables / disables sounds.
     * @param enabled Whether sound shall be enabled
     */
    void setSoundsEnabled(bool enabled);
    /// Loads the game settings
    void loadSettings();
    /// Loads the given tileset
    bool loadTileset(QString const & pathToTileset);
    /// Loads the given background
    bool loadBackground(QString const & pathToBackground);

private Q_SLOTS:
    void undrawConnection();

protected:
    QSize sizeHint() const override;

private: // functions
    /** Calculates the board's offset.
     * The board is centred inside the main playing area. xOffset()/yOffset()
     * provide the coordinates of the top-left corner of the board.
     */
    int xOffset() const;
    int yOffset() const;

    /** Returns the line width to use.
     * The line width should be relative to the tile size, however, if the tile size is too small, keep a minimum line width.
     */
    int lineWidth() const;

    /** Puts a tile of type @p value to the given position @p tilePos.
     * @param tilePos Position to be modified.
     * @param value Type of the tile to place.
     */
    void setField(TilePos tilePos, int value);
    /** Returns the kind of tile residing at the given position @p tilePos
     * @param TilePos Position to look at.
     * @return Type of the tile.
     */
    int field(TilePos tilePos) const;
    /** Repaints the area of the given @p TilePos.
     * @param TilePos Position of the tile to repaint.
     */
    void repaintTile(TilePos tilePos);
    void showInfoRect(QPainter &, const QString & message);
    void drawTiles(QPainter &, QPaintEvent *);
    void clearHighlight();

    /** Checks if two tiles can match.
     * This is used for connecting them and for highlighting tiles of the same group.
     * @param tile1 type of the first tile
     * @param tile2 type of the second tile
     */
    bool tilesMatch(int tile1, int tile2) const;

    /** Checks if a path between two tiles can be made with a single line.
     * @param tilePos1 coordinates of the first tile
     * @param tilePos2 coordinates of the second tile
     */
    bool canMakePath(TilePos tilePos1, TilePos tilePos2) const;

    /** Checks if the tile at \p tilePos1 can be slid to \p tilePos2.
     * @param tilePos1 coordinates of the slide's initial position
     * @param tilePos2 coordinates of the slide's final position
     * @param slide The movement of the last tile slid will be stored in @p slide
     */
    bool canSlideTiles(TilePos tilePos1, TilePos tilePos2, Slide & slide) const;

    /** Checks if a path between two tiles can be made with 2 or 3 lines.
    * @param tilePos1 coordinates of the first tile
    * @param tilePos2 coordinates of the second tile
    * @param possibleMoves All the possible moves are stored here
    * @return The number of paths found
    */
    int findPath(TilePos tilePos1, TilePos tilePos2, PossibleMoves & possibleMoves) const;

    /** Find a path of 1 or 2 segments between tiles.
     * @param tilePos1 coordinates of the first tile
     * @param tilePos2 coordinates of the second tile
     * @param possibleMoves all the possible moves are stored here
     * @return The number of paths found
     */
    int findSimplePath(TilePos tilePos1, TilePos tilePos2, PossibleMoves & possibleMoves) const;
    void performMove(PossibleMove & possibleMoves);
    void performSlide(TilePos tilePos, Slide const & slide);
    void reverseSlide(TilePos tilePos, Slide const & slide);
    bool isTileHighlighted(TilePos tilePos) const;
    void drawConnection();
    void drawPossibleMoves(bool b);
    /** Calculated the middle coordinates of the given tile position.
     * @param tilePos tile position
     * @return The middle coordinates of the tile at \p tilePos
     */
    QPoint midCoord(TilePos tilePos) const;
    void unmarkTile();
    void marked(TilePos tilePos);
    void madeMove(TilePos tilePos1, TilePos tilePos2, Slide slide = Slide());

    /// Applies gravity to all columns.
    void applyGravity();

    /** Returns True if @p tilePos is a valid position on Board.
     * @return Whether @p tiePos is valid.
     */
    bool isValidPos(TilePos tilePos) const;

    /** Returns True if @p tilePos is a valid position on Board including outline.
     * @return Whether @p tiePos is valid.
     */
    bool isValidPosWithOutline(TilePos tilePos) const;

    /** Returns True if @p tile is of kind FLOWERS.
     * @return Whether @p tile is within the range of the Flower tiles.
     */
    bool isTileFlower(int tile) const;
    /** Returns True if @p tile is of kind SEASONS.
     * @return Whether @p tile is within the range of the Seasons tiles.
     */
    bool isTileSeason(int tile) const;

private:
    KGameClock m_gameClock{};

    KMahjonggTileset m_tiles{};
    KMahjonggBackground m_background{};

    QRandomGenerator m_random{};

    std::list<std::unique_ptr<Move>> m_undo{}; ///< Undo history
    std::list<std::unique_ptr<Move>> m_redo{}; ///< Redo history

    int m_markX{0};
    int m_markY{0};
    Path m_connection{};
    PossibleMoves m_possibleMoves{};
    std::vector<int> m_field{}; ///< Matrix holding the game board grid
    int m_xTiles{};
    int m_yTiles{};
    int m_delay{};
    int m_level{};
    int m_shuffle{};

    // The game can be in one of the following states.
    enum class GameState { Normal,
                           Paused,
                           Stuck,
                           Over };
    GameState m_gameState{GameState::Normal};
    bool m_cheat{}; ///< Whether the cheat mode is set

    bool m_gravityFlag{true}; ///< Whether game is played with gravity
    bool m_solvableFlag{}; ///< Whether game is solvable
    bool m_showUnsolvableMessageFlag{}; ///< Whether "game unsolvable" message is shown
    bool m_chineseStyleFlag{}; ///< Whether game follows Chinese rules
    bool m_tilesCanSlideFlag{}; ///< Whether tiles can slide when connecting

    int m_highlightedTile{-1};

    bool m_paintConnection{};
    bool m_paintPossibleMoves{};
    bool m_paintInProgress{};
    TilePos m_tileRemove1{};
    TilePos m_tileRemove2{};
    KGameSound m_soundPick; ///< Sound object to play when tile is selected
    KGameSound m_soundFall; ///< Sound object to play when tiles fall down in gravity mode
};
} // namespace KShisen

#endif // KSHISEN_BOARD_H

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
