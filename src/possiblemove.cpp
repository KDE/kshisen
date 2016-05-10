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

PossibleMove::PossibleMove(Path & path, Path & slide)
    : m_path(path)
    , m_hasSlide(true)
    , m_slide(slide)
{
}

bool PossibleMove::isInPath(TilePos const & tilePos) const
{
    if (tilePos.x() == m_path.back().x() && tilePos.y() == m_path.back().y()) {
        return false;
    }
    qCDebug(KSHISEN_LOG) << "isInPath:" << tilePos.x() << "," << tilePos.y();
    Debug();

    // a path has at least 2 positions
    auto iter = m_path.cbegin();
    int pathX = iter->x();
    int pathY = iter->y();
    ++iter;
    for (; iter != m_path.cend(); ++iter) {
        // to fix
        if ((tilePos.x() == iter->x() && ((tilePos.y() > pathY && tilePos.y() <= iter->y())
                                          || (tilePos.y() < pathY && tilePos.y() >= iter->y())))
            || (tilePos.y() == iter->y() && ((tilePos.x() > pathX && tilePos.x() <= iter->x())
                                             || (tilePos.x() < pathX && tilePos.x() >= iter->x())))) {
            qCDebug(KSHISEN_LOG) << "isInPath:" << tilePos.x() << "," << tilePos.y() << "found in path" << pathX << "," << pathY << " => " << iter->x() << "," << iter->y();
            return true;
        }
        pathX = iter->x();
        pathY = iter->y();
    }
    return false;
}

void PossibleMove::Debug() const
{
    qCDebug(KSHISEN_LOG) << "PossibleMove";

    foreach (auto iter, m_path) {
        qCDebug(KSHISEN_LOG) << "    Path:" << iter.x() << "," << iter.y();
    }

    if (m_hasSlide) {
        qCDebug(KSHISEN_LOG) << "   hasSlide";
        foreach (auto iter, m_slide) {
            qCDebug(KSHISEN_LOG) << "    Slide:" << iter.x() << "," << iter.y();
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

Path PossibleMove::slide() const
{
    return m_slide;
}
void PossibleMove::prependTile(TilePos const tilePos)
{
    m_path.prepend(tilePos);
}
}

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
