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

// own
#include "move.h"

namespace KShisen
{
Move::Move(TilePos const & tilePos1, TilePos const & tilePos2, int tile1, int tile2)
    : m_tilePos1(tilePos1)
    , m_tilePos2(tilePos2)
    , m_tile1(tile1)
    , m_tile2(tile2)
    , m_hasSlide(false)
    , m_slide(Slide())
{
}

Move::Move(TilePos const & tilePos1, TilePos const & tilePos2, int tile1, int tile2, Slide const & slide)
    : m_tilePos1(tilePos1)
    , m_tilePos2(tilePos2)
    , m_tile1(tile1)
    , m_tile2(tile2)
    , m_hasSlide(true)
    , m_slide(slide)
{
}

int Move::x1() const
{
    return m_tilePos1.x();
}

int Move::y1() const
{
    return m_tilePos1.y();
}

int Move::x2() const
{
    return m_tilePos2.x();
}

int Move::y2() const
{
    return m_tilePos2.y();
}

int Move::tile1() const
{
    return m_tile1;
}

int Move::tile2() const
{
    return m_tile2;
}

bool Move::hasSlide() const
{
    return m_hasSlide;
}

Slide Move::slide() const
{
    return m_slide;
}

int Move::slideX1() const
{
    if (m_slide.empty()) {
        return 0;
    }
    return m_slide.front().x();
}

int Move::slideY1() const
{
    if (m_slide.empty()) {
        return 0;
    }
    return m_slide.front().y();
}

int Move::slideX2() const
{
    if (m_slide.empty()) {
        return 0;
    }
    return m_slide.back().x();
}

int Move::slideY2() const
{
    if (m_slide.empty()) {
        return 0;
    }
    return m_slide.back().y();
}

void Move::swapTiles()
{
    std::swap(m_tilePos1, m_tilePos2);
    std::swap(m_tile1, m_tile2);
}
} // namespace KShisen

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
