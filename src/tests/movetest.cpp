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
    qDebug("called before everything else");

    m_tile1 = KShisen::TilePos(1, 2);
    m_tile2 = KShisen::TilePos(3, 4);
}

void MoveTest::swap()
{
    auto move = KShisen::Move(m_tile1, m_tile2, 12, 34);
    move.swapTiles();

    QCOMPARE(move.tile1(), 34);
    QCOMPARE(move.tile2(), 12);
}

void MoveTest::cleanupTestCase()
{
    qDebug("called after myFirstTest and mySecondTest");
}

QTEST_MAIN(MoveTest)
#include "movetest.moc"
