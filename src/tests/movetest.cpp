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
    QCOMPARE(m_moveWithSlide.x1(), 5);
}

void MoveTest::y1() const
{
    QCOMPARE(m_moveWithoutSlide.y1(), 2);
    QCOMPARE(m_moveWithSlide.y1(), 6);
}

void MoveTest::x2() const
{
    QCOMPARE(m_moveWithoutSlide.x2(), 3);
    QCOMPARE(m_moveWithSlide.x2(), 7);
}

void MoveTest::y2() const
{
    QCOMPARE(m_moveWithoutSlide.y2(), 4);
    QCOMPARE(m_moveWithSlide.y2(), 8);
}

void MoveTest::tile1() const
{
    QCOMPARE(m_moveWithoutSlide.tile1(), 12);
    QCOMPARE(m_moveWithSlide.tile1(), 56);
}

void MoveTest::tile2() const
{
    QCOMPARE(m_moveWithoutSlide.tile2(), 34);
    QCOMPARE(m_moveWithSlide.tile2(), 78);
}

void MoveTest::hasSlide() const
{
    QCOMPARE(m_moveWithoutSlide.hasSlide(), false);
    QCOMPARE(m_moveWithSlide.hasSlide(), true);
}

void MoveTest::slide() const
{
    QCOMPARE(m_moveWithoutSlide.slide(), KShisen::Slide());
    QCOMPARE(m_moveWithSlide.slide(), KShisen::Slide() << m_slidePos1 << m_slidePos2);
}

void MoveTest::slideX1() const
{
    QCOMPARE(m_moveWithoutSlide.slideX1(), 0);
    QCOMPARE(m_moveWithSlide.slideX1(), 15);
}

void MoveTest::slideY1() const
{
    QCOMPARE(m_moveWithoutSlide.slideY1(), 0);
    QCOMPARE(m_moveWithSlide.slideY1(), 16);
}

void MoveTest::slideX2() const
{
    QCOMPARE(m_moveWithoutSlide.slideX2(), 0);
    QCOMPARE(m_moveWithSlide.slideX2(), 17);
}

void MoveTest::slideY2() const
{
    QCOMPARE(m_moveWithoutSlide.slideY2(), 0);
    QCOMPARE(m_moveWithSlide.slideY2(), 18);
}

void MoveTest::swapTiles()
{
    m_moveWithoutSlide.swapTiles();

    QCOMPARE(m_moveWithoutSlide.x1(), 3);
    QCOMPARE(m_moveWithoutSlide.y1(), 4);
    QCOMPARE(m_moveWithoutSlide.tile1(), 34);
    QCOMPARE(m_moveWithoutSlide.x2(), 1);
    QCOMPARE(m_moveWithoutSlide.y2(), 2);
    QCOMPARE(m_moveWithoutSlide.tile2(), 12);

    m_moveWithSlide.swapTiles();

    QCOMPARE(m_moveWithSlide.x1(), 7);
    QCOMPARE(m_moveWithSlide.y1(), 8);
    QCOMPARE(m_moveWithSlide.tile1(), 78);
    QCOMPARE(m_moveWithSlide.x2(), 5);
    QCOMPARE(m_moveWithSlide.y2(), 6);
    QCOMPARE(m_moveWithSlide.tile2(), 56);
}

void MoveTest::cleanupTestCase()
{
}

QTEST_APPLESS_MAIN(MoveTest)
#include "movetest.moc"
