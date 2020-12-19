/*
    KShisen - A japanese game similar to Mahjongg
    SPDX-FileCopyrightText: 2016 Frederik Schwarzer <schwarzer@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KSHISEN_TYPES_H
#define KSHISEN_TYPES_H

#include <QList>
#include <QPoint>

namespace KShisen
{
using TilePos = QPoint;

/**
 * A list of positions (at least 2) makes a Path
 */
using Path = QList<TilePos>;

using Slide = QList<TilePos>;
}

#endif // KSHISEN_TYPES_H

// vim: expandtab:tabstop=4:shiftwidth=4
// kate: space-indent on; indent-width 4
