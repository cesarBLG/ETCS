//THIS FILE IS PART OF SDL_GFX LIBRARY, AND IT IS LICENSED UNDER ZLIB LICENSE
//VISIT THE AUTHOR'S WEBPAGE FOR MORE INFORMATION
#include "gfx_primitives.h"
#include <cmath>
using namespace std;
int pixelRGBA(SDL_Renderer * renderer, Sint16 x, Sint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	int result = 0;
	result |= SDL_SetRenderDrawBlendMode(renderer, (a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
	result |= SDL_SetRenderDrawColor(renderer, r, g, b, a);
	result |= SDL_RenderDrawPoint(renderer, x, y);
	return result;
}
int pixelRGBAWeight(SDL_Renderer * renderer, Sint16 x, Sint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a, Uint32 weight)
{
	/*
	* Modify Alpha by weight 
	*/
	Uint32 ax = a;
	ax = ((ax * weight) >> 8);
	if (ax > 255) {
		a = 255;
	} else {
		a = (Uint8)(ax & 0x000000ff);
	}

	return pixelRGBA(renderer, x, y, r, g, b, a);
}
int vline(SDL_Renderer * renderer, Sint16 x, Sint16 y1, Sint16 y2)
{
	return SDL_RenderDrawLine(renderer, x, y1, x, y2);;
}
int hline(SDL_Renderer * renderer, Sint16 x1, Sint16 x2, Sint16 y)
{
	return SDL_RenderDrawLine(renderer, x1, y, x2, y);;
}
int hlineRGBA(SDL_Renderer * renderer, Sint16 x1, Sint16 x2, Sint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	int result = 0;
	result |= SDL_SetRenderDrawBlendMode(renderer, (a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
	result |= SDL_SetRenderDrawColor(renderer, r, g, b, a);
	result |= SDL_RenderDrawLine(renderer, x1, y, x2, y);
	return result;
}
int vlineRGBA(SDL_Renderer * renderer, Sint16 x, Sint16 y1, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	int result = 0;
	result |= SDL_SetRenderDrawBlendMode(renderer, (a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
	result |= SDL_SetRenderDrawColor(renderer, r, g, b, a);
	result |= SDL_RenderDrawLine(renderer, x, y1, x, y2);
	return result;
}
int pixel(SDL_Renderer *renderer, Sint16 x, Sint16 y)
{
	return SDL_RenderDrawPoint(renderer, x, y);
}
int _drawQuadrants(SDL_Renderer * renderer,  Sint16 x, Sint16 y, Sint16 dx, Sint16 dy, Sint32 f)
{
	int result = 0;
	Sint16 xpdx, xmdx;
	Sint16 ypdy, ymdy;

	if (dx == 0) {
		if (dy == 0) {
			result |= pixel(renderer, x, y);
		} else {
			ypdy = y + dy;
			ymdy = y - dy;
			if (f) {
				result |= vline(renderer, x, ymdy, ypdy);
			} else {
				result |= pixel(renderer, x, ypdy);
				result |= pixel(renderer, x, ymdy);
			}
		}
	} else {	
		xpdx = x + dx;
		xmdx = x - dx;
		ypdy = y + dy;
		ymdy = y - dy;
		if (f) {
				result |= vline(renderer, xpdx, ymdy, ypdy);
				result |= vline(renderer, xmdx, ymdy, ypdy);
		} else {
				result |= pixel(renderer, xpdx, ypdy);
				result |= pixel(renderer, xmdx, ypdy);
				result |= pixel(renderer, xpdx, ymdy);
				result |= pixel(renderer, xmdx, ymdy);
		}
	}

	return result;
}
int _gfxPrimitivesCompareInt(const void *a, const void *b)
{
	return (*(const int *) a) - (*(const int *) b);
}
static int *gfxPrimitivesPolyIntsGlobal = NULL;
static int gfxPrimitivesPolyAllocatedGlobal = 0;
int filledPolygonRGBAMT(SDL_Renderer * renderer, const Sint16 * vx, const Sint16 * vy, int n, Uint8 r, Uint8 g, Uint8 b, Uint8 a, int **polyInts, int *polyAllocated)
{
	int result;
	int i;
	int y, xa, xb;
	int miny, maxy;
	int x1, y1;
	int x2, y2;
	int ind1, ind2;
	int ints;
	int *gfxPrimitivesPolyInts = NULL;
	int *gfxPrimitivesPolyIntsNew = NULL;
	int gfxPrimitivesPolyAllocated = 0;

	/*
	* Vertex array NULL check 
	*/
	if (vx == NULL) {
		return (-1);
	}
	if (vy == NULL) {
		return (-1);
	}

	/*
	* Sanity check number of edges
	*/
	if (n < 3) {
		return -1;
	}

	/*
	* Map polygon cache  
	*/
	if ((polyInts==NULL) || (polyAllocated==NULL)) {
		/* Use global cache */
		gfxPrimitivesPolyInts = gfxPrimitivesPolyIntsGlobal;
		gfxPrimitivesPolyAllocated = gfxPrimitivesPolyAllocatedGlobal;
	} else {
		/* Use local cache */
		gfxPrimitivesPolyInts = *polyInts;
		gfxPrimitivesPolyAllocated = *polyAllocated;
	}

	/*
	* Allocate temp array, only grow array 
	*/
	if (!gfxPrimitivesPolyAllocated) {
		gfxPrimitivesPolyInts = (int *) malloc(sizeof(int) * n);
		gfxPrimitivesPolyAllocated = n;
	} else {
		if (gfxPrimitivesPolyAllocated < n) {
			gfxPrimitivesPolyIntsNew = (int *) realloc(gfxPrimitivesPolyInts, sizeof(int) * n);
			if (!gfxPrimitivesPolyIntsNew) {
				if (!gfxPrimitivesPolyInts) {
					free(gfxPrimitivesPolyInts);
					gfxPrimitivesPolyInts = NULL;
				}
				gfxPrimitivesPolyAllocated = 0;
			} else {
				gfxPrimitivesPolyInts = gfxPrimitivesPolyIntsNew;
				gfxPrimitivesPolyAllocated = n;
			}
		}
	}

	/*
	* Check temp array
	*/
	if (gfxPrimitivesPolyInts==NULL) {        
		gfxPrimitivesPolyAllocated = 0;
	}

	/*
	* Update cache variables
	*/
	if ((polyInts==NULL) || (polyAllocated==NULL)) { 
		gfxPrimitivesPolyIntsGlobal =  gfxPrimitivesPolyInts;
		gfxPrimitivesPolyAllocatedGlobal = gfxPrimitivesPolyAllocated;
	} else {
		*polyInts = gfxPrimitivesPolyInts;
		*polyAllocated = gfxPrimitivesPolyAllocated;
	}

	/*
	* Check temp array again
	*/
	if (gfxPrimitivesPolyInts==NULL) {        
		return(-1);
	}

	/*
	* Determine Y maxima 
	*/
	miny = vy[0];
	maxy = vy[0];
	for (i = 1; (i < n); i++) {
		if (vy[i] < miny) {
			miny = vy[i];
		} else if (vy[i] > maxy) {
			maxy = vy[i];
		}
	}

	/*
	* Draw, scanning y 
	*/
	result = 0;
	for (y = miny; (y <= maxy); y++) {
		ints = 0;
		for (i = 0; (i < n); i++) {
			if (!i) {
				ind1 = n - 1;
				ind2 = 0;
			} else {
				ind1 = i - 1;
				ind2 = i;
			}
			y1 = vy[ind1];
			y2 = vy[ind2];
			if (y1 < y2) {
				x1 = vx[ind1];
				x2 = vx[ind2];
			} else if (y1 > y2) {
				y2 = vy[ind1];
				y1 = vy[ind2];
				x2 = vx[ind1];
				x1 = vx[ind2];
			} else {
				continue;
			}
			if ( ((y >= y1) && (y < y2)) || ((y == maxy) && (y > y1) && (y <= y2)) ) {
				gfxPrimitivesPolyInts[ints++] = ((65536 * (y - y1)) / (y2 - y1)) * (x2 - x1) + (65536 * x1);
			} 	    
		}

		qsort(gfxPrimitivesPolyInts, ints, sizeof(int), _gfxPrimitivesCompareInt);

		/*
		* Set color 
		*/
		result = 0;
	    result |= SDL_SetRenderDrawBlendMode(renderer, (a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
		result |= SDL_SetRenderDrawColor(renderer, r, g, b, a);	

		for (i = 0; (i < ints); i += 2) {
			xa = gfxPrimitivesPolyInts[i] + 1;
			xa = (xa >> 16) + ((xa & 32768) >> 15);
			xb = gfxPrimitivesPolyInts[i+1] - 1;
			xb = (xb >> 16) + ((xb & 32768) >> 15);
			result |= hline(renderer, xa, xb, y);
		}
	}

	return (result);
}
int lineRGBA(SDL_Renderer * renderer, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	/*
	* Draw
	*/
	int result = 0;
	result |= SDL_SetRenderDrawBlendMode(renderer, (a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
	result |= SDL_SetRenderDrawColor(renderer, r, g, b, a);	
	result |= SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	return result;
}
#define AAlevels 256
#define AAbits 8
int _aalineRGBA(SDL_Renderer * renderer, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, int draw_endpoint)
{
	Sint32 xx0, yy0, xx1, yy1;
	int result;
	Uint32 intshift, erracc, erradj;
	Uint32 erracctmp, wgt, wgtcompmask;
	int dx, dy, tmp, xdir, y0p1, x0pxdir;

	/*
	* Keep on working with 32bit numbers 
	*/
	xx0 = x1;
	yy0 = y1;
	xx1 = x2;
	yy1 = y2;

	/*
	* Reorder points to make dy positive 
	*/
	if (yy0 > yy1) {
		tmp = yy0;
		yy0 = yy1;
		yy1 = tmp;
		tmp = xx0;
		xx0 = xx1;
		xx1 = tmp;
	}

	/*
	* Calculate distance 
	*/
	dx = xx1 - xx0;
	dy = yy1 - yy0;

	/*
	* Adjust for negative dx and set xdir 
	*/
	if (dx >= 0) {
		xdir = 1;
	} else {
		xdir = -1;
		dx = (-dx);
	}
	
	/*
	* Check for special cases 
	*/
	if (dx == 0) {
		/*
		* Vertical line 
		*/
		if (draw_endpoint)
		{
			return (vlineRGBA(renderer, x1, y1, y2, r, g, b, a));
		} else {
			if (dy > 0) {
				return (vlineRGBA(renderer, x1, yy0, yy0+dy, r, g, b, a));
			} else {
				return (pixelRGBA(renderer, x1, y1, r, g, b, a));
			}
		}
	} else if (dy == 0) {
		/*
		* Horizontal line 
		*/
		if (draw_endpoint)
		{
			return (hlineRGBA(renderer, x1, x2, y1, r, g, b, a));
		} else {
			if (dx > 0) {
				return (hlineRGBA(renderer, xx0, xx0+(xdir*dx), y1, r, g, b, a));
			} else {
				return (pixelRGBA(renderer, x1, y1, r, g, b, a));
			}
		}
	} else if ((dx == dy) && (draw_endpoint)) {
		/*
		* Diagonal line (with endpoint)
		*/
		return (lineRGBA(renderer, x1, y1, x2, y2,  r, g, b, a));
	}


	/*
	* Line is not horizontal, vertical or diagonal (with endpoint)
	*/
	result = 0;

	/*
	* Zero accumulator 
	*/
	erracc = 0;

	/*
	* # of bits by which to shift erracc to get intensity level 
	*/
	intshift = 32 - AAbits;

	/*
	* Mask used to flip all bits in an intensity weighting 
	*/
	wgtcompmask = AAlevels - 1;

	/*
	* Draw the initial pixel in the foreground color 
	*/
	result |= pixelRGBA(renderer, x1, y1, r, g, b, a);

	/*
	* x-major or y-major? 
	*/
	if (dy > dx) {

		/*
		* y-major.  Calculate 16-bit fixed point fractional part of a pixel that
		* X advances every time Y advances 1 pixel, truncating the result so that
		* we won't overrun the endpoint along the X axis 
		*/
		/*
		* Not-so-portable version: erradj = ((Uint64)dx << 32) / (Uint64)dy; 
		*/
		erradj = ((dx << 16) / dy) << 16;

		/*
		* draw all pixels other than the first and last 
		*/
		x0pxdir = xx0 + xdir;
		while (--dy) {
			erracctmp = erracc;
			erracc += erradj;
			if (erracc <= erracctmp) {
				/*
				* rollover in error accumulator, x coord advances 
				*/
				xx0 = x0pxdir;
				x0pxdir += xdir;
			}
			yy0++;		/* y-major so always advance Y */

			/*
			* the AAbits most significant bits of erracc give us the intensity
			* weighting for this pixel, and the complement of the weighting for
			* the paired pixel. 
			*/
			wgt = (erracc >> intshift) & 255;
			result |= pixelRGBAWeight (renderer, xx0, yy0, r, g, b, a, 255 - wgt);
			result |= pixelRGBAWeight (renderer, x0pxdir, yy0, r, g, b, a, wgt);
		}

	} else {

		/*
		* x-major line.  Calculate 16-bit fixed-point fractional part of a pixel
		* that Y advances each time X advances 1 pixel, truncating the result so
		* that we won't overrun the endpoint along the X axis. 
		*/
		/*
		* Not-so-portable version: erradj = ((Uint64)dy << 32) / (Uint64)dx; 
		*/
		erradj = ((dy << 16) / dx) << 16;

		/*
		* draw all pixels other than the first and last 
		*/
		y0p1 = yy0 + 1;
		while (--dx) {

			erracctmp = erracc;
			erracc += erradj;
			if (erracc <= erracctmp) {
				/*
				* Accumulator turned over, advance y 
				*/
				yy0 = y0p1;
				y0p1++;
			}
			xx0 += xdir;	/* x-major so always advance X */
			/*
			* the AAbits most significant bits of erracc give us the intensity
			* weighting for this pixel, and the complement of the weighting for
			* the paired pixel. 
			*/
			wgt = (erracc >> intshift) & 255;
			result |= pixelRGBAWeight (renderer, xx0, yy0, r, g, b, a, 255 - wgt);
			result |= pixelRGBAWeight (renderer, xx0, y0p1, r, g, b, a, wgt);
		}
	}

	/*
	* Do we have to draw the endpoint 
	*/
	if (draw_endpoint) {
		/*
		* Draw final pixel, always exactly intersected by the line and doesn't
		* need to be weighted. 
		*/
		result |= pixelRGBA (renderer, x2, y2, r, g, b, a);
	}

	return (result);
}
int aacircleRGBA(SDL_Renderer * renderer, Sint16 x, Sint16 y, Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	/*
	* Draw 
	*/
	return aaellipseRGBA(renderer, x, y, rad, rad, r, g, b, a);
}
int aaellipseRGBA(SDL_Renderer * renderer, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	int result;
	int i;
	int a2, b2, ds, dt, dxt, t, s, d;
	Sint16 xp, yp, xs, ys, dyt, od, xx, yy, xc2, yc2;
	float cp;
	double sab;
	Uint8 weight, iweight;

	/*
	* Sanity check radii 
	*/
	if ((rx < 0) || (ry < 0)) {
		return (-1);
	}

	/*
	* Special cases for rx=0 and/or ry=0: draw a hline/vline/pixel 
	*/
	if (rx == 0) {
		if (ry == 0) {
			return (pixelRGBA(renderer, x, y, r, g, b, a));
		} else {
			return (vlineRGBA(renderer, x, y - ry, y + ry, r, g, b, a));
		}
	} else {
		if (ry == 0) {
			return (hlineRGBA(renderer, x - rx, x + rx, y, r, g, b, a));
		}
	}

	/* Variable setup */
	a2 = rx * rx;
	b2 = ry * ry;

	ds = 2 * a2;
	dt = 2 * b2;

	xc2 = 2 * x;
	yc2 = 2 * y;

	sab = sqrt((double)(a2 + b2));
	od = (Sint16)lrint(sab*0.01) + 1; /* introduce some overdraw */
	dxt = (Sint16)lrint((double)a2 / sab) + od;

	t = 0;
	s = -2 * a2 * ry;
	d = 0;

	xp = x;
	yp = y - ry;

	/* Draw */
	result = 0;
	result |= SDL_SetRenderDrawBlendMode(renderer, (a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);

	/* "End points" */
	result |= pixelRGBA(renderer, xp, yp, r, g, b, a);
	result |= pixelRGBA(renderer, xc2 - xp, yp, r, g, b, a);
	result |= pixelRGBA(renderer, xp, yc2 - yp, r, g, b, a);
	result |= pixelRGBA(renderer, xc2 - xp, yc2 - yp, r, g, b, a);

	for (i = 1; i <= dxt; i++) {
		xp--;
		d += t - b2;

		if (d >= 0)
			ys = yp - 1;
		else if ((d - s - a2) > 0) {
			if ((2 * d - s - a2) >= 0)
				ys = yp + 1;
			else {
				ys = yp;
				yp++;
				d -= s + a2;
				s += ds;
			}
		} else {
			yp++;
			ys = yp + 1;
			d -= s + a2;
			s += ds;
		}

		t -= dt;

		/* Calculate alpha */
		if (s != 0) {
			cp = (float) abs(d) / (float) abs(s);
			if (cp > 1.0) {
				cp = 1.0;
			}
		} else {
			cp = 1.0;
		}

		/* Calculate weights */
		weight = (Uint8) (cp * 255);
		iweight = 255 - weight;

		/* Upper half */
		xx = xc2 - xp;
		result |= pixelRGBAWeight(renderer, xp, yp, r, g, b, a, iweight);
		result |= pixelRGBAWeight(renderer, xx, yp, r, g, b, a, iweight);

		result |= pixelRGBAWeight(renderer, xp, ys, r, g, b, a, weight);
		result |= pixelRGBAWeight(renderer, xx, ys, r, g, b, a, weight);

		/* Lower half */
		yy = yc2 - yp;
		result |= pixelRGBAWeight(renderer, xp, yy, r, g, b, a, iweight);
		result |= pixelRGBAWeight(renderer, xx, yy, r, g, b, a, iweight);

		yy = yc2 - ys;
		result |= pixelRGBAWeight(renderer, xp, yy, r, g, b, a, weight);
		result |= pixelRGBAWeight(renderer, xx, yy, r, g, b, a, weight);
	}

	/* Replaces original approximation code dyt = abs(yp - yc); */
	dyt = (Sint16)lrint((double)b2 / sab ) + od;    

	for (i = 1; i <= dyt; i++) {
		yp++;
		d -= s + a2;

		if (d <= 0)
			xs = xp + 1;
		else if ((d + t - b2) < 0) {
			if ((2 * d + t - b2) <= 0)
				xs = xp - 1;
			else {
				xs = xp;
				xp--;
				d += t - b2;
				t -= dt;
			}
		} else {
			xp--;
			xs = xp - 1;
			d += t - b2;
			t -= dt;
		}

		s += ds;

		/* Calculate alpha */
		if (t != 0) {
			cp = (float) abs(d) / (float) abs(t);
			if (cp > 1.0) {
				cp = 1.0;
			}
		} else {
			cp = 1.0;
		}

		/* Calculate weight */
		weight = (Uint8) (cp * 255);
		iweight = 255 - weight;

		/* Left half */
		xx = xc2 - xp;
		yy = yc2 - yp;
		result |= pixelRGBAWeight(renderer, xp, yp, r, g, b, a, iweight);
		result |= pixelRGBAWeight(renderer, xx, yp, r, g, b, a, iweight);

		result |= pixelRGBAWeight(renderer, xp, yy, r, g, b, a, iweight);
		result |= pixelRGBAWeight(renderer, xx, yy, r, g, b, a, iweight);

		/* Right half */
		xx = xc2 - xs;
		result |= pixelRGBAWeight(renderer, xs, yp, r, g, b, a, weight);
		result |= pixelRGBAWeight(renderer, xx, yp, r, g, b, a, weight);

		result |= pixelRGBAWeight(renderer, xs, yy, r, g, b, a, weight);
		result |= pixelRGBAWeight(renderer, xx, yy, r, g, b, a, weight);		
	}

	return (result);
}
#define DEFAULT_ELLIPSE_OVERSCAN	4
int _ellipseRGBA(SDL_Renderer * renderer, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a, Sint32 f)
{
	int result;
	Sint32 rxi, ryi;
	Sint32 rx2, ry2, rx22, ry22; 
    Sint32 error;
    Sint32 curX, curY, curXp1, curYm1;
	Sint32 scrX, scrY, oldX, oldY;
    Sint32 deltaX, deltaY;
	Sint32 ellipseOverscan;

	/*
	* Sanity check radii 
	*/
	if ((rx < 0) || (ry < 0)) {
		return (-1);
	}

	/*
	* Set color
	*/
	result = 0;
	result |= SDL_SetRenderDrawBlendMode(renderer, (a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
	result |= SDL_SetRenderDrawColor(renderer, r, g, b, a);

	/*
	* Special cases for rx=0 and/or ry=0: draw a hline/vline/pixel 
	*/
	if (rx == 0) {
		if (ry == 0) {
			return (pixel(renderer, x, y));
		} else {
			return (vline(renderer, x, y - ry, y + ry));
		}
	} else {
		if (ry == 0) {
			return (hline(renderer, x - rx, x + rx, y));
		}
	}
	
	/*
 	 * Adjust overscan 
	 */
	rxi = rx;
	ryi = ry;
	if (rxi >= 512 || ryi >= 512)
	{
		ellipseOverscan = DEFAULT_ELLIPSE_OVERSCAN / 4;
	} 
	else if (rxi >= 256 || ryi >= 256)
	{
		ellipseOverscan = DEFAULT_ELLIPSE_OVERSCAN / 2;
	}
	else
	{
		ellipseOverscan = DEFAULT_ELLIPSE_OVERSCAN / 1;
	}

	/*
	 * Top/bottom center points.
	 */
	oldX = scrX = 0;
	oldY = scrY = ryi;
	result |= _drawQuadrants(renderer, x, y, 0, ry, f);

	/* Midpoint ellipse algorithm with overdraw */
	rxi *= ellipseOverscan;
	ryi *= ellipseOverscan;
	rx2 = rxi * rxi;
	rx22 = rx2 + rx2;
    ry2 = ryi * ryi;
	ry22 = ry2 + ry2;
    curX = 0;
    curY = ryi;
    deltaX = 0;
    deltaY = rx22 * curY;
 
	/* Points in segment 1 */ 
    error = ry2 - rx2 * ryi + rx2 / 4;
    while (deltaX <= deltaY)
    {
          curX++;
          deltaX += ry22;
 
          error +=  deltaX + ry2; 
          if (error >= 0)
          {
               curY--;
               deltaY -= rx22; 
               error -= deltaY;
          }

		  scrX = curX / ellipseOverscan;
		  scrY = curY / ellipseOverscan;
		  if ((scrX != oldX && scrY == oldY) || (scrX != oldX && scrY != oldY)) {
			result |= _drawQuadrants(renderer, x, y, scrX, scrY, f);
			oldX = scrX;
			oldY = scrY;
		  }
    }

	/* Points in segment 2 */
	if (curY > 0) 
	{
		curXp1 = curX + 1;
		curYm1 = curY - 1;
		error = ry2 * curX * curXp1 + ((ry2 + 3) / 4) + rx2 * curYm1 * curYm1 - rx2 * ry2;
		while (curY > 0)
		{
			curY--;
			deltaY -= rx22;

			error += rx2;
			error -= deltaY;
 
			if (error <= 0) 
			{
               curX++;
               deltaX += ry22;
               error += deltaX;
			}

		    scrX = curX / ellipseOverscan;
		    scrY = curY / ellipseOverscan;
		    if ((scrX != oldX && scrY == oldY) || (scrX != oldX && scrY != oldY)) {
				oldY--;
				for (;oldY >= scrY; oldY--) {
					result |= _drawQuadrants(renderer, x, y, scrX, oldY, f);
					/* prevent overdraw */
					if (f) {
						oldY = scrY - 1;
					}
				}
  				oldX = scrX;
				oldY = scrY;
		    }		
		}

		/* Remaining points in vertical */
		if (!f) {
			oldY--;
			for (;oldY >= 0; oldY--) {
				result |= _drawQuadrants(renderer, x, y, scrX, oldY, f);
			}
		}
	}

	return (result);
}
int filledCircleRGBA(SDL_Renderer * renderer, Sint16 x, Sint16 y, Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	return _ellipseRGBA(renderer, x, y, rad, rad, r, g ,b, a, 1);
}
int filledPolygonRGBA(SDL_Renderer * renderer, const Sint16 * vx, const Sint16 * vy, int n, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	return filledPolygonRGBAMT(renderer, vx, vy, n, r, g, b, a, NULL, NULL);
}
int aapolygonRGBA(SDL_Renderer * renderer, const Sint16 * vx, const Sint16 * vy, int n, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	int result;
	int i;
	const Sint16 *x1, *y1, *x2, *y2;

	/*
	* Vertex array NULL check 
	*/
	if (vx == NULL) {
		return (-1);
	}
	if (vy == NULL) {
		return (-1);
	}

	/*
	* Sanity check 
	*/
	if (n < 3) {
		return (-1);
	}

	/*
	* Pointer setup 
	*/
	x1 = x2 = vx;
	y1 = y2 = vy;
	x2++;
	y2++;

	/*
	* Draw 
	*/
	result = 0;
	for (i = 1; i < n; i++) {
		result |= _aalineRGBA(renderer, *x1, *y1, *x2, *y2, r, g, b, a, 0);
		x1 = x2;
		y1 = y2;
		x2++;
		y2++;
	}

	result |= _aalineRGBA(renderer, *x1, *y1, *vx, *vy, r, g, b, a, 0);

	return (result);
}
