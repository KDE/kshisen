/*
    KShisen - A japanese game similar to Mahjongg
    SPDX-FileCopyrightText: 2016 Frederik Schwarzer <schwarzer@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    PossibleMove(Path & path, Slide & slide);

    bool isInPath(TilePos tilePos) const;

    void Debug() const;

    Path path() const;
    bool hasSlide() const;
    Slide slide() const;
    void prependTile(TilePos const tilePos);

private:
    Path m_path; ///< path used to connect the two tiles
    bool m_hasSlide; ///< flag set if the move requires a slide
    Slide m_slide; ///< representing the movement of the last sliding tile
};
} // namespace KShisen

#endif // KSHISEN_POSSIBLEMOVE_H

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
