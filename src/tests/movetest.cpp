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
