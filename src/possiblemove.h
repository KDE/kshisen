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

#ifndef KSHISEN_POSSIBLEMOVE_H
#define KSHISEN_POSSIBLEMOVE_H

#include "types.h"

namespace KShisen
{
/**
 * @brief Class holding a possible move and its functions
 *
 * A PossibleMove is a connection Path between two tiles
 * and optionally a slide Path.
 * Sometimes for a couple of tiles to match there may be multiple
 * possible moves for the player to choose between.
 */
class PossibleMove
{
public:
    explicit PossibleMove(Path & path);
    PossibleMove(Path & path, Path & slide);

    bool isInPath(TilePos const & tilePos) const;

    void Debug() const;

    Path path() const;
    bool hasSlide() const;
    Path slide() const;

private:
    Path m_path; ///< path used to connect the two tiles
    bool m_hasSlide; ///< flag set if the move requires a slide
    Path m_slide; ///< path representing the movement of the last sliding tile
};
}

#endif // KSHISEN_POSSIBLEMOVE_H

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
