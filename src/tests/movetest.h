#include <QtTest>

#include "../move.h"
#include "../types.h"

class MoveTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void swap();

    void cleanupTestCase();

private:
    KShisen::TilePos m_tile1;
    KShisen::TilePos m_tile2;
};
