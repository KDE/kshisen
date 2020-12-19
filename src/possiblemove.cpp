/*
    KShisen - A japanese game similar to Mahjongg
    SPDX-FileCopyrightText: 2016 Frederik Schwarzer <schwarzer@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// own
#include "possiblemove.h"

// KShisen
#include "debug.h"

namespace KShisen
{
PossibleMove::PossibleMove(Path & path)
    : m_path(path)
    , m_hasSlide(false)
    , m_slide()
{
}

PossibleMove::PossibleMove(Path & path, Slide & slide)
    : m_path(path)
    , m_hasSlide(true)
    , m_slide(slide)
{
}

bool PossibleMove::isInPath(TilePos tilePos) const
{
    if (tilePos.x() == m_path.back().x() && tilePos.y() == m_path.back().y()) {
        return false;
    }
    qCDebug(KSHISEN_General) << "isInPath:" << tilePos.x() << "," << tilePos.y();
    Debug();

    // a path has at least 2 positions
    auto iter = m_path.cbegin();
    auto pathX = iter->x();
    auto pathY = iter->y();
    ++iter;
    for (; iter != m_path.cend(); ++iter) {
        // to fix
        if ((tilePos.x() == iter->x() && ((tilePos.y() > pathY && tilePos.y() <= iter->y())
                                          || (tilePos.y() < pathY && tilePos.y() >= iter->y())))
            || (tilePos.y() == iter->y() && ((tilePos.x() > pathX && tilePos.x() <= iter->x())
                                             || (tilePos.x() < pathX && tilePos.x() >= iter->x())))) {
            qCDebug(KSHISEN_General) << "isInPath:" << tilePos.x() << "," << tilePos.y() << "found in path" << pathX << "," << pathY << " => " << iter->x() << "," << iter->y();
            return true;
        }
        pathX = iter->x();
        pathY = iter->y();
    }
    return false;
}

void PossibleMove::Debug() const
{
    qCDebug(KSHISEN_General) << "PossibleMove";

    for (auto const &iter : qAsConst(m_path)) {
        qCDebug(KSHISEN_General) << "    Path:" << iter.x() << "," << iter.y();
    }

    if (m_hasSlide) {
        qCDebug(KSHISEN_General) << "   hasSlide";
        for (auto const &iter : qAsConst(m_slide)) {
            qCDebug(KSHISEN_General) << "    Slide:" << iter.x() << "," << iter.y();
        }
    }
}

Path PossibleMove::path() const
{
    return m_path;
}

bool PossibleMove::hasSlide() const
{
    return m_hasSlide;
}

Slide PossibleMove::slide() const
{
    return m_slide;
}
void PossibleMove::prependTile(TilePos const tilePos)
{
    m_path.prepend(tilePos);
}
} // namespace KShisen

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
