/**
 * tileset.h
 *
 * Copyright (c) 2002 Jason Lane <jglane@btopenworld.com>
 *           (c) 2002 Dave Corrie <kde@davecorrie.com>
 *
 *  This file is part of KShisen.
 *
 *  KMail is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __IMAGEDATA__H__
#define __IMAGEDATA__H__

#include <qvaluevector.h>

class TileSet
{

public:

	static const int nTiles = 36;

	TileSet();
	~TileSet();

	void resizeTiles(int maxWidth, int maxHeight);

	const QPixmap &tile(int n) const;
	QPixmap highlightedTile(int n) const;

	int lineWidth() const;

	int tileWidth() const;
	int tileHeight() const;
	int unscaledTileHeight() const;
	int unscaledTileWidth() const;

private:

	QValueVector<QPixmap> scaledTiles;
	QValueVector<QImage> unscaledTiles;

};

#endif

