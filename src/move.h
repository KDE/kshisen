/***************************************************************************
 *   KShisen - A japanese game similar to mahjongg                         *
 *   Copyright 2016  Frederik Schwarzer <schwarzer@kde.org>                *
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

#include "types.h"

#ifndef KSHISEN_MOVE_H
#define KSHISEN_MOVE_H

namespace KShisen
{
/**
 * @brief Class holding a move on the board made by the player
 *
 * Contains all the information needed to undo or redo a move.
 */
class Move
{
public:
    Move(TilePos tilePos1, TilePos tilePos2, int tile1, int tile2);
    Move(TilePos tilePos1, TilePos tilePos2, int tile1, int tile2, Slide const & slide);

    int x1() const;
    int y1() const;
    int x2() const;
    int y2() const;
    int tile1() const;
    int tile2() const;
    bool hasSlide() const;
    Slide slide() const;
    int slideX1() const;
    int slideY1() const;
    int slideX2() const;
    int slideY2() const;

    /**
     * @brief Swaps the two tiles involved in a move.
     *
     * This is needed for undoing a move in case both tiles are in the same column.
     */
    void swapTiles();

private:
    TilePos m_tilePos1; ///< coordinates of the first tile that matched
    TilePos m_tilePos2; ///< coordinates of the second tile that matched
    int m_tile1; ///< type of tile at first set of coordinates
    int m_tile2; ///< type of tile at second set of coordinates
    bool m_hasSlide; ///< if we performed a slide during the move
    Slide m_slide; ///< original x coordinate of the last slided tile
};
} // namespace KShisen

#endif // KSHISEN_MOVE_H

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
