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

#include "movetest.h"

void MoveTest::initTestCase()
{
}

void MoveTest::x1() const
{
    QCOMPARE(m_moveWithoutSlide.x1(), 1);
}

void MoveTest::y1() const
{
    QCOMPARE(m_moveWithoutSlide.y1(), 2);
}

void MoveTest::x2() const
{
    QCOMPARE(m_moveWithoutSlide.x2(), 3);
}

void MoveTest::y2() const
{
    QCOMPARE(m_moveWithoutSlide.y2(), 4);
}

void MoveTest::tile1() const
{
    QCOMPARE(m_moveWithoutSlide.tile1(), 12);
}

void MoveTest::tile2() const
{
    QCOMPARE(m_moveWithoutSlide.tile2(), 34);
}

void MoveTest::hasSlide() const
{
    QCOMPARE(m_moveWithoutSlide.hasSlide(), false);
    QCOMPARE(m_moveWithSlide.hasSlide(), true);
}

void MoveTest::slideX1() const
{
    QCOMPARE(m_moveWithSlide.slideX1(), 5);
}

void MoveTest::slideY1() const
{
    QCOMPARE(m_moveWithSlide.slideY1(), 6);
}

void MoveTest::slideX2() const
{
    QCOMPARE(m_moveWithSlide.slideX2(), 7);
}

void MoveTest::slideY2() const
{
    QCOMPARE(m_moveWithSlide.slideY2(), 8);
}

void MoveTest::swapTiles()
{
    auto move = KShisen::Move(m_tile1, m_tile2, 12, 34);
    move.swapTiles();

    QCOMPARE(move.x1(), 3);
    QCOMPARE(move.y1(), 4);
    QCOMPARE(move.tile1(), 34);

    QCOMPARE(move.x2(), 1);
    QCOMPARE(move.y2(), 2);
    QCOMPARE(move.tile2(), 12);
}

void MoveTest::cleanupTestCase()
{
}

QTEST_MAIN(MoveTest)
#include "movetest.moc"
