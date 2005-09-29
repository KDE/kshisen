/**
 * tileset.cpp
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

#include <kapplication.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <qimage.h>

#include <algorithm>

#include "tileset.h"

TileSet::TileSet() : scaledTiles(nTiles)
{
	//loadTiles
	QImage tileset(KGlobal::dirs()->findResource("appdata", "tileset.png"));
	if(tileset.isNull())
	{
		KMessageBox::sorry(0, i18n("Cannot load tiles pixmap!"));
		KApplication::exit(1);
	}

	// split into individual tiles
	const int TILES_X = 9;
	const int TILES_Y = 4;
	unscaledTiles.reserve(nTiles);

	int w = tileset.width() / TILES_X;
	int h = tileset.height() / TILES_Y;
	for(int row = 0; row < TILES_Y; row++)
	{
		for(int col = 0; col < TILES_X; col++)
			unscaledTiles.push_back(tileset.copy(col * w, row * h, w, h));
	}
}

TileSet::~TileSet()
{
}

void TileSet::resizeTiles(int maxWidth, int maxHeight)
{
	// calculate largest tile size that will fit in maxWidth/maxHeight
	// and maintain the tile's height-to-width ratio
	double ratio = static_cast<double>(unscaledTileHeight()) / unscaledTileWidth();
	if(maxWidth * ratio < maxHeight)
		maxHeight = qRound(maxWidth * ratio);
	else
		maxWidth = qRound(maxHeight / ratio);

	if(maxHeight == tileHeight() && maxWidth == tileWidth())
		return;

	//kdDebug() << "tile size: " << maxWidth << "x" << maxHeight << endl;

	QImage img;
	for(int i = 0; i < nTiles; i++)
	{
		if(maxHeight == unscaledTileHeight())
			img = unscaledTiles[i].copy();//.convertDepth(32);
		else
			img = unscaledTiles[i].smoothScale(maxWidth, maxHeight);

		scaledTiles[i].convertFromImage(img);
	}
}

const QPixmap &TileSet::tile(int n) const
{
	return scaledTiles[n];
}

QPixmap TileSet::highlightedTile(int n) const
{
	const double LIGHTEN_FACTOR = 1.3;

	// lighten the image
	QImage img = scaledTiles[n].convertToImage().convertDepth(32);

	for(int y = 0; y < img.height(); y++)
	{
		uchar* p = img.scanLine(y);
		for(int x = 0; x < img.width() * 4; x++)
		{
			*p = static_cast<uchar>(std::min(255, static_cast<int>(*p * LIGHTEN_FACTOR)));
			p++;
		}
	}

	QPixmap highlightedTile;
	highlightedTile.convertFromImage(img);

	return highlightedTile;
}

int TileSet::lineWidth() const
{
	int width = qRound(tileHeight() / 10.0);
	if(width < 3)
		width = 3;

	return width;
}

int TileSet:: tileWidth() const
{
	return scaledTiles[0].width();
}

int TileSet:: tileHeight() const
{
	return scaledTiles[0].height();
}

int TileSet:: unscaledTileHeight() const
{
	return unscaledTiles[0].height();
}

int TileSet:: unscaledTileWidth() const
{
	return unscaledTiles[0].width();
}

