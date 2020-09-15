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

#ifndef MOVETEST_H
#define MOVETEST_H

#include <QTest>

#include "../move.h"
#include "../types.h"

class MoveTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void x1() const;
    void y1() const;
    void x2() const;
    void y2() const;
    void tile1() const;
    void tile2() const;
    void hasSlide() const;
    void slide() const;
    void slideX1() const;
    void slideY1() const;
    void slideX2() const;
    void slideY2() const;
    void swapTiles();

    void cleanupTestCase();

private:
    KShisen::TilePos m_tile1{1, 2};
    KShisen::TilePos m_tile2{3, 4};
    KShisen::TilePos m_tile3{5, 6};
    KShisen::TilePos m_tile4{7, 8};
    KShisen::TilePos m_slidePos1{15, 16};
    KShisen::TilePos m_slidePos2{17, 18};
    KShisen::Move m_moveWithoutSlide{m_tile1, m_tile2, 12, 34};
    KShisen::Move m_moveWithSlide{m_tile3, m_tile4, 56, 78, KShisen::Slide() << m_slidePos1 << m_slidePos2};
};

#endif // MOVETEST_H
