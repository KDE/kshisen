/* Yo Emacs, this is -*- C++ -*-
 *******************************************************************
 *******************************************************************
 *
 *
 * KSHISEN
 *
 *
 *******************************************************************
 *
 * A japanese game similar to mahjongg
 *
 *******************************************************************
 *
 * created 1997 by Mario Weilguni <mweilguni@sime.com>
 *
 *******************************************************************
 *
 * This file is part of the KDE project "KSHISEN"
 *
 * KSHISEN is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * KSHISEN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with KSHISEN; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *******************************************************************
 */

#include "board.h"
#include <stdlib.h>
#include <stdio.h>
#include <qpainter.h>
#include <kapp.h>
#include <qimage.h>
#include <qtimer.h>
#include <qlist.h>
#include <sys/times.h>
#include <qbitarry.h>
#include <kmsgbox.h>
#include <debug.h>

#ifdef DEBUGGING
#include <unistd.h>
#endif

#define EMPTY		0
#define XBORDER		20
#define YBORDER		20
#define DEFAULTDELAY	500
#define DEFAULTSHUFFLE	4

extern QString PICDIR;

Board::Board(QWidget *parent) : QWidget(parent) {
  trying = FALSE;
  _solvable_flag = TRUE;

  // randomze
  setShuffle(DEFAULTSHUFFLE);
  clock_t t;
  struct tms dummy;
  t = times(&dummy); 
  srandom((int)t);

  starttime = time((time_t *)0);

  for(int i = 0; i < 45; i++)
    pm_tile[i] = 0;

  setDelay(DEFAULTDELAY);
  _redo.setAutoDelete(TRUE);
  _undo.setAutoDelete(TRUE);

  loadTiles(1);
  field = 0;
  QPixmap bg((PICDIR + "/kshisen_bgnd.xpm").data());
  setBackgroundPixmap(bg);
  connect(this, SIGNAL(fieldClicked(int, int)),
	  this, SLOT(marked(int, int)));
  connect(this, SIGNAL(madeMove(int, int, int, int)),
	  this, SLOT(slotMadeMove(int, int, int, int)));  

  setShuffle(0);
  setSize(18, 8);

  highlighted_tile = -1;
}

Board::~Board() {
  for(int i = 0; i < 36; i++)
    if(pm_tile[i])
      delete pm_tile[i];  
}

int Board::x_tiles() {
  return _x_tiles;
}

int Board::y_tiles() {
  return _y_tiles;
}

void Board::setField(int x, int y, int value) {
  if(x < x_tiles() && y < y_tiles())
    field[y * x_tiles() + x] = value;
  else {
    fprintf(stderr, "write access to invalid field (%d,%d)\n", x, y);
    exit(1);
  }
}

int Board::getField(int x, int y) {
  if(x == x_tiles() || x == -1)
    return EMPTY;
  else if(y == y_tiles() || y == -1)
    return EMPTY;
  else if(x < -1 || x > x_tiles() || y < -1 || y > y_tiles()) {
    return EMPTY;
    fprintf(stderr, "read access to invalid field (%d,%d)\n", x, y);
    exit(1);
    return 0; // makes gcc happy
  } else
    return field[y * x_tiles() + x];
}

void Board::mousePressEvent(QMouseEvent *e) {
    // calculate position
    int pos_x = (e->pos().x() - XBORDER <0)?-1:
                              (e->pos().x() - XBORDER) / pm_tile[0]->width();
    int pos_y = (e->pos().y() - YBORDER <0)?-1:
                              (e->pos().y() - YBORDER) / pm_tile[0]->height();

    // Mark tile
    if(e->button() == LeftButton) {
      if(highlighted_tile != -1) {
	int oldmarkx = mark_x;
	int oldmarky = mark_y;
	
	mark_x=-1; mark_y=-1;
	for(int i = 0; i < x_tiles(); i++)
	  for(int j = 0; j < y_tiles(); j++){
	    if( highlighted_tile == getField(i, j))
	      updateField(i, j);	      
	  }
	mark_x = oldmarkx; 
	mark_y = oldmarky;   // no tile selected
	highlighted_tile = -1;
      }
      
      if(pos_x >= 0 && pos_x < x_tiles() && pos_y >= 0 && pos_y < y_tiles())
	emit fieldClicked(pos_x, pos_y);  
    }

    // Assist by lighting all tiles of same type
    if(e->button() == RightButton) {
      int field = getField(pos_x,pos_y);
      highlighted_tile = field;
      
      for(int i = 0; i < x_tiles(); i++)
	for(int j = 0; j < y_tiles(); j++){
	  if( field == getField(i, j)){
	    mark_x=i; mark_y=j;
	  }
	  else{
	    mark_x=-1; mark_y=-1;
	  }
	  updateField(i, j);
	}
      mark_x=-1; mark_y=-1;   // no tile selected
    }
}

void Board::setSize(int x, int y) {
  if(x == x_tiles() && y == y_tiles())
    return;

  if(field != 0)
    free(field);

  field = (int*)malloc(sizeof(int) * x * y);
  _x_tiles = x;
  _y_tiles = y;
  for(int i = 0; i < x; i++)
    for(int j = 0; j < y; j++)
      setField(i, j, EMPTY);

  loadTiles();
  double scaler = 1.0;
  while(sizeHint().width() > kapp->desktop()->width() - 2*XBORDER ||
     sizeHint().height() > kapp->desktop()->height() - 2*YBORDER) {
    scaler -= 0.2;
    loadTiles(scaler);
  }

  newGame();
  emit changed();
  emit sizeChange();
}

bool Board::loadTiles(float scale) {
  int i, j, x, y;

  // delete old tiles
  for(i = 0; i < 45; i++)
    if(pm_tile[i] != 0) {
      delete pm_tile[i];
      pm_tile[i] = 0;
    }

  QPixmap pm((PICDIR + "/kshisen.xpm").data());
  if(pm.width() == 0 || pm.height() == 0) {
    KMsgBox::message(0, klocale->translate("Shisen-Sho"), 
		     klocale->translate("Cannot load pixmaps!"));
    exit(1);
  }
  
  if(pm.width() == 0 || pm.height() == 0)
    return FALSE;

  x = pm.width() / 9;
  y = pm.height() / 5;
  for(i = 0; i < 9; i++)
    for(j = 0; j < 5; j++) {
	pm_tile[i + j*9] = new QPixmap(x,y);
	bitBlt(pm_tile[i + j*9], 0, 0, &pm, x * i, y * j, x, y, CopyROP);
	if(scale != 1.0) {
	  QWMatrix wm;
	  wm.scale(scale, scale);
	  *pm_tile[i + j*9] = pm_tile[i + j*9]->xForm(wm);
	}
    }
  
  return TRUE;
}

void Board::newGame() {
  int i, j, x, y, k;

  mark_x = -1;
  mark_y = -1;

  while(_undo.count())
    _undo.removeFirst();
  while(_redo.count())
    _redo.removeFirst();

  clearHistory();

  for(i = 0; i < x_tiles(); i++)
    for(j = 0; j < y_tiles(); j++)
      setField(i, j, EMPTY);

  // distribute all tiles on board
  int cur_tile = 0;
  for(i = 0; i < x_tiles() * y_tiles() * 12; i++) {
    // map the tileindex to a tile
    // not all tiles from the pixmap are really used, only
    // 36 out of 45 are used. This maps and index to
    // the "real" index.
    int tile;
    if(cur_tile == 28)
      tile = 30;
    else if(cur_tile >= 29 && cur_tile <= 35)
      tile = cur_tile + 7;
    else
      tile = cur_tile;
    
    cur_tile++;
    if(cur_tile == 36)
      cur_tile = 0;

    x = i % x_tiles();
    y = i / x_tiles() * 4;

    tile++;
    for(k = 0; k < 4 && k + y < y_tiles(); k++)
      setField(x, y+k, tile);
  }

  if(getShuffle() == 0) {
    if(!trying) {
      update();
      starttime = time((time_t *)0);
      emit changed();
    }    
    return;
  }

  // shuffle the field
  int tx = x_tiles();
  int ty = y_tiles();
  for(i = 0; i < x_tiles() * y_tiles() * getShuffle(); i++) {
    int x1 = random(tx);
    int y1 = random(ty);
    int x2 = random(tx);
    int y2 = random(ty);
    int t  = getField(x1, y1);
    setField(x1, y1, getField(x2, y2));
    setField(x2, y2, t);
  }

  // do not make solvable if _solvable_flag is FALSE
  if(!_solvable_flag) {
    if(!trying) {
      update();
      starttime = time((time_t *)0);
      emit changed();
    }    
    return;
  }

  
  int fsize = x_tiles() * y_tiles() * sizeof(int);
  int *oldfield = new int[x_tiles() * y_tiles()];
  memcpy(oldfield, field, fsize);			// save field
  int *tiles = new int[x_tiles() * y_tiles()];
  int *pos = new int[x_tiles() * y_tiles()];

  while(!solvable(TRUE)) {
    // generate a list of free tiles and positions
    int num_tiles = 0;
    for(i = 0; i < x_tiles() * y_tiles(); i++)
      if(field[i] != EMPTY) {
	pos[num_tiles] = i;
	tiles[num_tiles] = field[i];
	num_tiles++;
      }

    // restore field
    memcpy(field, oldfield, fsize);
    
    // redistribute unsolved tiles
    while(num_tiles > 0) {
      // get a random tile
      int r1 = random(num_tiles);
      int r2 = random(num_tiles);
      int tile = tiles[r1];
      int apos = pos[r2];
      
      // truncate list
      tiles[r1] = tiles[num_tiles-1];
      pos[r2] = pos[num_tiles-1];
      num_tiles--;

      // put this tile on the new position
      field[apos] = tile;
    }

    // remember field
    memcpy(oldfield, field, fsize);
  }


  // restore field
  memcpy(field, oldfield, fsize);  
  delete tiles;
  delete pos;
  delete oldfield;

  if(!trying) {
    update();
    starttime = time((time_t *)0);
    emit changed();
  }
}

void Board::updateField(int x, int y) {
  if(trying)
    return;

  QRect r(XBORDER + x * pm_tile[0]->width(),
	  YBORDER + y * pm_tile[0]->height(),
	  pm_tile[0]->width(),
	  pm_tile[0]->height());
  if(isUpdatesEnabled())
    repaint(r, TRUE);
}

int MIN(int a, int b) {
  if(a < b)
    return a;
  else
    return b;
}

int MAX(int a, int b) {
  if(a > b)
    return a;
  else
    return b;
}

QPixmap *Board::lighten(QPixmap *src) {
  const float FACTOR = 1.3;

  QImage img = src->convertToImage();
  if(img.depth() > 8) { // at least high-color
    for(int y = 0; y < src->height(); y++) {
      uchar *p = (uchar *)img.scanLine(y);
      for(int x = 0; x < src->width() * 4; x++) {
	*p = (unsigned char)MIN(255, (int)(FACTOR * (*p)));
	p++;
      }
    }
  } else { // image has a palette
    // get background color index
    int idx = img.pixelIndex(8, 1); // should work for all tiles
    img.setColor(idx, QColor(img.color(idx)).light().rgb());
  }
 
  QPixmap *pm = new QPixmap();
  pm->convertFromImage(img);
  return pm;
}

void Board::paintEvent(QPaintEvent *e) {
  QPainter p;
  p.begin(this);
  for(int i = 0; i < x_tiles(); i++)
    for(int j = 0; j < y_tiles(); j++) {
      if(getField(i, j) == EMPTY)
	continue;

      int xpos = XBORDER + i * pm_tile[1]->width();
      int ypos = YBORDER + j * pm_tile[1]->height();
      QRect r(xpos, ypos, pm_tile[1]->width(), pm_tile[1]->height());
      if(e->rect().intersects(r)) {
	// check if it is a marked piece
	if(i == mark_x && j == mark_y) {
	    QPixmap *lpm = lighten(pm_tile[getField(i, j)-1]);
	    p.drawPixmap(xpos, ypos, *lpm);
	    delete lpm;
	} else
	  p.drawPixmap(xpos, ypos, *pm_tile[getField(i, j)-1]);
      }
    }
  p.end();
}

// returns a random number < max
int Board::random(int max) {
  //return (int)((float)(max) * rand() / (RAND_MAX+1.0));
  return ::random() % max; // don't depend on RAND_MAX...
}

void Board::marked(int x, int y) {
  // make sure that the last arrow is correctly undrawn
  undrawArrow();

  if(getField(x, y) == EMPTY)
    return;
  
  if(x == mark_x && y == mark_y) {
    // unmark the piece
    mark_x = -1;
    mark_y = -1;
    updateField(x, y);
    return;
  }

  if(mark_x == -1) {
    mark_x = x;
    mark_y = y;
    updateField(x, y);
    return;
  } else {
    int fld1 = getField(mark_x, mark_y);
    int fld2 = getField(x, y);
    
    // both field same?
    if(fld1 != fld2) {
      emit markError();
      return;
    }
    
    // trace    
    if(findPath(mark_x, mark_y, x, y)) {
      emit madeMove(mark_x, mark_y, x, y);
      drawArrow(mark_x, mark_y, x, y);
      setField(mark_x, mark_y, EMPTY);
      setField(x, y, EMPTY);
      mark_x = -1;
      mark_y = -1;

      int dummyx;
      History dummyh[4];

      // game is over?      
      if(!getHint_I(dummyx,dummyx,dummyx,dummyx,dummyh)) {
	time_for_game = (int)time((time_t)NULL) - starttime;
	emit endOfGame();
      }
      
    } else {
      clearHistory();
      emit markError();
    }
  }
}

bool Board::canMakePath(int x1, int y1, int x2, int y2) {
  int i;

  if(x1 == x2) {
    for(i = MIN(y1, y2)+1; i < MAX(y1, y2); i++) 
      if(getField(x1, i) != EMPTY)
	return FALSE;
  
    return TRUE;
  } 

  if(y1 == y2) {
    for(i = MIN(x1, x2)+1; i < MAX(x1, x2); i++)
      if(getField(i, y1) != EMPTY)
	return FALSE;

    return TRUE;
  }

  return FALSE;
}

bool Board::findPath(int x1, int y1, int x2, int y2) {
 clearHistory();

  if(findSimplePath(x1, y1, x2, y2))
     return TRUE;
  else {
    // find 3-way path
    int dx[4] = {1, 0, -1, 0};
    int dy[4] = {0, 1, 0, -1};
    int i;

    for(i = 0; i < 4; i++) {
      int newx = x1 + dx[i], newy = y1 + dy[i];
      while(getField(newx, newy) == EMPTY && 
	    newx >= -1 && newx <= x_tiles() &&
	    newy >= -1 && newy <= y_tiles()) {
	if(findSimplePath(newx, newy, x2, y2)) {
	  // make place for history point
	  for(int j = 3; j > 0; j--)
	    history[j] = history[j-1];

	  // insert history point
	  history[0].x = x1;
	  history[0].y = y1;
	  return TRUE;	 
	}

	newx += dx[i];
	newy += dy[i];
      }
    }

    clearHistory();
    return FALSE;
  }

  return FALSE;
}

bool Board::findSimplePath(int x1, int y1, int x2, int y2) {
  bool r = FALSE;

  // find direct line
  if(canMakePath(x1, y1, x2, y2)) {
    history[0].x = x1;
    history[0].y = y1;
    history[1].x = x2;
    history[1].y = y2;
    r = TRUE;
  } else {
    if(!(x1 == x2 || y1 == y2)) // requires complex path
      if(getField(x2, y1) == EMPTY &&
	 canMakePath(x1, y1, x2, y1) && canMakePath(x2, y1, x2, y2)) {
	history[0].x = x1;
	history[0].y = y1;
	history[1].x = x2;
	history[1].y = y1;
	history[2].x = x2;
	history[2].y = y2;
	r = TRUE;
      } else if(getField(x1, y2) == EMPTY &&
		canMakePath(x1, y1, x1, y2) && canMakePath(x1, y2, x2, y2)) {
	history[0].x = x1;
	history[0].y = y1;
	history[1].x = x1;
	history[1].y = y2;
	history[2].x = x2;
	history[2].y = y2;
	r = TRUE;
      }
  }
  
  return r;
}

void Board::drawArrow(int x1, int y1, int x2, int y2) {
  if(trying)
    return;

  // find out number of array
  int num = 0;
  while(num < 4 && history[num].x != -2)
    num++;

  // lighten the fields
  // remember mark_x,mark_y
  int mx = mark_x, my = mark_y;
  mark_x = x1; mark_y = y1;
  updateField(x1, y1);
  mark_x = x2; mark_y = y2;
  updateField(x2, y2);

  // restore the mark
  mark_x = mx;
  mark_y = my;

  QPainter p;
  p.begin(this);
  p.setPen(QPen(QColor("red"), 6));
  num = 0;
  while(num < 3 && history[num+1].x != -2) {
    p.drawLine(midCoord(history[num].x, history[num].y),
	       midCoord(history[num+1].x, history[num+1].y));
    num++;    
  }
  p.flush();
  p.end();

  QTimer::singleShot(getDelay(), this, SLOT(undrawArrow()));
}

void Board::undrawArrow() {
  if(trying)
    return;

  // is already undrawn?
  if(history[0].x == -2)
    return;

  // redraw all affected fields
  int num = 0;
  while(num < 3 && history[num+1].x != -2) {
    if(history[num].y == history[num+1].y)
      for(int i = MIN(history[num].x, history[num+1].x); 
	  i <= MAX(history[num].x, history[num+1].x); i++)
	updateField(i, history[num].y);
    else 
      for(int i = MIN(history[num].y, history[num+1].y); 
	  i <= MAX(history[num].y, history[num+1].y); i++)
	updateField(history[num].x, i);
    num++;
  }

  clearHistory();
}

QPoint Board::midCoord(int x, int y) {
  QPoint p;
  int w = pm_tile[0]->width();
  int h = pm_tile[0]->height();

  if(x == -1) 
    p.setX(XBORDER/2 - w/2);
  else if(x == x_tiles())
    p.setX(XBORDER/2 + w * x_tiles());
  else 
    p.setX(XBORDER + w * x);

  if(y == -1) 
    p.setY(YBORDER/2 - h/2);
  else if(y == y_tiles())
    p.setY(YBORDER/2 + h * y_tiles());
  else 
    p.setY(YBORDER + h * y);

  p.setX(p.x() + w/2);
  p.setY(p.y() + h/2);
  return p;
} 

void Board::setDelay(int newvalue) {
  _delay = newvalue;
}

int Board::getDelay() {
  return _delay;
}

void Board::slotMadeMove(int x1, int y1, int x2, int y2) {
  Move *m = new Move(x1, y1, x2, y2, getField(x1, y1));
  _undo.append(m);
  while(_redo.count())
    _redo.removeFirst();
  emit changed();
}

bool Board::canUndo() {
  return (bool)(_undo.count() > 0);
}

bool Board::canRedo() {
  return (bool)(_redo.count() > 0);
}

void Board::undo() {
  if(canUndo()) {
    undrawArrow();
    Move *m = _undo.take(_undo.count() - 1);
    setField(m->x1, m->y1, m->tile);
    setField(m->x2, m->y2, m->tile);
    updateField(m->x1, m->y1);
    updateField(m->x2, m->y2);
    _redo.insert(0, m);
    emit changed();
  }
}

void Board::redo() {
  if(canRedo()) {
    undrawArrow();
    Move *m = _redo.take(0);
    setField(m->x1, m->y1, EMPTY);
    setField(m->x2, m->y2, EMPTY);
    updateField(m->x1, m->y1);
    updateField(m->x2, m->y2);
    _undo.append(m);
    emit changed();
  }
}

QSize Board::sizeHint() {
  return QSize(x_tiles() * pm_tile[0]->width() + 2 * XBORDER,
	       y_tiles() * pm_tile[0]->height() + 2 * YBORDER);
}

void Board::clearHistory() {
  // init history
  for(int i = 0; i < 4; i++) {
    history[i].x = -2;
    history[i].y = -2;
  }
}

void Board::getHint() {
  int x1, y1, x2, y2;
  History h[4];

  if(getHint_I(x1, y1, x2, y2, h)) {
    undrawArrow();
    for(int i = 0; i < 4; i++)
      history[i] = h[i];

    int old_delay = getDelay();
    setDelay(1000);
    drawArrow(x1, y1, x2, y2);
    setDelay(old_delay);
  }
}

#ifdef DEBUGGING
void Board::finish() {
  int x1, y1, x2, y2;
  History h[4];
  bool ready=FALSE;

  while(!ready && getHint_I(x1, y1, x2, y2, h)) {
    mark_x = -1;
    mark_y = -1;
    if(tilesLeft() == 2)
      ready = TRUE;
    marked(x1, y1);
    marked(x2, y2);
    kapp->processEvents();
    usleep(250*1000);
  }
}
#endif

bool Board::getHint_I(int &x1, int &y1, int &x2, int &y2, History h[4]) {
  short done[45];
  for( short index = 0; index < 45; index++ )
     done[index] = 0;

  // remember old history
  History old[4];
  for(int i = 0; i < 4; i++)
    old[i] = history[i];

  // initial no hint
  x1 = -1;
  x2 = -1;
  y1 = -1;
  y2 = -1;

  for(int x = 0; x < x_tiles(); x++)
    for(int y = 0; y < y_tiles(); y++)
      if(getField(x, y) != EMPTY && done[getField(x, y)] != 4) {
	int tile = getField(x, y);
	
	// for all these types of tile search path's
	for(int xx = 0; xx < x_tiles(); xx++)
	  for(int yy = 0; yy < y_tiles(); yy++)
	    if(xx != x || yy != y)
	      if(getField(xx, yy) == tile)
		if(findPath(x, y, xx, yy)) {
		  for(int i = 0; i < 4; i++)
		    h[i] = history[i];

		  x1 = x;
		  x2 = xx;
		  y1 = y;
		  y2 = yy;
		  for(int i = 0; i < 4; i++)
		    history[i] = old[i];
		  return TRUE;
		}
	
	clearHistory();
	done[tile]++;
      }

  for(int i = 0; i < 4; i++)
    history[i] = old[i];

  return FALSE;
}

void Board::setShuffle(int newvalue) {
  _shuffle = newvalue;
}

int Board::getShuffle() {
  return _shuffle;
}

int Board::tilesLeft() {
  int left = 0;

  for(int i = 0; i < x_tiles(); i++)
    for(int j = 0; j < x_tiles(); j++)
      if(getField(i, j) != EMPTY)
	left++;

  return left;
}

int Board::getCurrentTime() {
  return (int)(time((time_t *)0) - starttime);
}

int Board::getTimeForGame() {
  if(tilesLeft() == 0) 
    return time_for_game;
  else
    return (int)(time((time_t *)0) - starttime);
}

bool Board::solvable(bool norestore) {
  int x1, y1, x2, y2;
  History h[4];
  int *oldfield = 0;
 
  if(!norestore) {
    oldfield = (int *)malloc(x_tiles() * y_tiles() * sizeof(int));
    memcpy(oldfield, field, x_tiles() * y_tiles() * sizeof(int));
  }

  while(getHint_I(x1, y1, x2, y2, h)) {
    setField(x1, y1, EMPTY);
    setField(x2, y2, EMPTY);
  }
  
  int left = tilesLeft();

  if(!norestore) {
    memcpy(field, oldfield, x_tiles() * y_tiles() * sizeof(int));
    free(oldfield);  
  }

  return (bool)(left == 0);
}

bool Board::getSolvableFlag() {
  return _solvable_flag;
}

void Board::setSolvableFlag(bool value) {
  _solvable_flag = value;
}

#include "board.moc"
