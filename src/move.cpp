/*
    KShisen - A japanese game similar to Mahjongg
    SPDX-FileCopyrightText: 2016 Frederik Schwarzer <schwarzer@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// own
#include "move.h"

namespace KShisen
{
Move::Move(TilePos tilePos1, TilePos tilePos2, int tile1, int tile2)
    : m_tilePos1(tilePos1)
    , m_tilePos2(tilePos2)
    , m_tile1(tile1)
    , m_tile2(tile2)
    , m_hasSlide(false)
    , m_slide(Slide())
{
}

Move::Move(TilePos tilePos1, TilePos tilePos2, int tile1, int tile2, Slide const & slide)
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
