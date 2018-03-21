/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Sample Code : Smple_LCD_Graphics                                                                        */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>	
#include <string.h>																										 
#include "NUC1xx.h"
#include "LCD_Driver.h"
#include "2D_Graphic_Driver.h"

extern unsigned char DisplayBuf[8*128];

void draw_pixel(int x, int y, int color)
{
	if (color==0) {
		switch (x%8) {
		case 0: DisplayBuf[(x/8)+y*8] &= 0xFE;  break;
		case 1: DisplayBuf[(x/8)+y*8] &= 0xFD;  break;
		case 2: DisplayBuf[(x/8)+y*8] &= 0xFB;  break;
		case 3: DisplayBuf[(x/8)+y*8] &= 0xF7;  break;
		case 4: DisplayBuf[(x/8)+y*8] &= 0xEF;  break;
		case 5: DisplayBuf[(x/8)+y*8] &= 0xDF;  break;
		case 6: DisplayBuf[(x/8)+y*8] &= 0xBF;  break;
		case 7: DisplayBuf[(x/8)+y*8] &= 0x7F;  break;
		default: break; }
		}
	else if (color==1) {
		switch (x%8) {
		case 0: DisplayBuf[(x/8)+y*8] |= 0x01;  break;
		case 1: DisplayBuf[(x/8)+y*8] |= 0x02;  break;
		case 2: DisplayBuf[(x/8)+y*8] |= 0x04;  break;
		case 3: DisplayBuf[(x/8)+y*8] |= 0x08;  break;
		case 4: DisplayBuf[(x/8)+y*8] |= 0x10;  break;
		case 5: DisplayBuf[(x/8)+y*8] |= 0x20;  break;
		case 6: DisplayBuf[(x/8)+y*8] |= 0x40;  break;
		case 7: DisplayBuf[(x/8)+y*8] |= 0x80;  break;
		default: break;	}
		}
}

void LineBresenham(int x1, int y1, int x2, int y2, int color)
{
    int dy = y2 - y1;
    int dx = x2 - x1;
    int stepx, stepy;

    if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
    if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
    dy <<= 1;        // dy is now 2*dy
    dx <<= 1;        // dx is now 2*dx

    draw_pixel(x1,y1, color);
    if (dx > dy) 
    {
        int fraction = dy - (dx >> 1);  // same as 2*dy - dx
        while (x1 != x2) 
        {
           if (fraction >= 0) 
           {
               y1 += stepy;
               fraction -= dx;          // same as fraction -= 2*dx
           }
           x1 += stepx;
           fraction += dy;              // same as fraction -= 2*dy
           draw_pixel(x1, y1, color);
        }
     } else {
        int fraction = dx - (dy >> 1);
        while (y1 != y2) {
           if (fraction >= 0) {
               x1 += stepx;
               fraction -= dy;
           }
           y1 += stepy;
           fraction += dx;
           draw_pixel(x1, y1, color);
        }
     }
}

void LineOptimized(int x1, int y1, int x2, int y2, int color)
{
    int cx, cy,
        ix, iy,
        dx, dy, 
        ddx= x2-x1, ddy= y2-y1;
     
    if (!ddx) { //vertical line special case
        if (ddy > 0) {
            cy= y1;  
            while (cy<= y2) draw_pixel(x1, cy++, color);
        } else {
            cy= y2;
            while (cy <= y1) draw_pixel(x1, cy++, color);
        }
    }
    if (!ddy) { //horizontal line special case
        if (ddx > 0) {
            cx= x1;
            while (++cx <= x2) draw_pixel(cx, y1, color);
        } else {
            cx= x2; 
            while (++cx <= x1) draw_pixel(cx, y1, color);
        }
    }
    if (ddy < 0) { iy= -1; ddy= -ddy; }//pointing up
            else iy= 1;
    if (ddx < 0) { ix= -1; ddx= -ddx; }//pointing left
            else ix= 1;
    dx= dy= ddx*ddy;
    cy= y1, cx= x1; 
    if (ddx < ddy) { // < 45 degrees, a tall line    
        while (dx > 0) {
            dx-=ddy;
            while (dy >=dx) {
                draw_pixel(cx, cy, color);
                cy+=iy, dy-=ddx;
            };
            cx+=ix;
        };
    } else { // >= 45 degrees, a wide line
        while (dy > 0) {
            dy-=ddx;
            while (dx >=dy) {
                draw_pixel(cx, cy, color);
                cx+=ix, dx-=ddy;
            } ;
            cy+=iy;
        };
    }
}

void CircleBresenham(int xc, int yc, int r, int color)
{
    int x = 0; 
    int y = r; 
    int p = 3 - 2 * r;
    if (!r) return;     
    while (y >= x) // only formulate 1/8 of circle
    {
        draw_pixel(xc-x, yc-y, color);//upper left left
        draw_pixel(xc-y, yc-x, color);//upper upper left
        draw_pixel(xc+y, yc-x, color);//upper upper right
        draw_pixel(xc+x, yc-y, color);//upper right right
        draw_pixel(xc-x, yc+y, color);//lower left left
        draw_pixel(xc-y, yc+x, color);//lower lower left
        draw_pixel(xc+y, yc+x, color);//lower lower right
        draw_pixel(xc+x, yc+y, color);//lower right right
        if (p < 0) p += 4*x++ + 6; 
              else p += 4*(x++ - y--) + 10; 
     } 
}

void CircleMidpoint(int xc, int yc, int r, int color)
{
    int   x= 0, y= r;
    int   d= 1-r;
    int   dE= 3;
    int   dSE= 5 - 2*r;

    if (!r) return;
    draw_pixel(xc-r, yc, color);
    draw_pixel(xc+r, yc, color);
    draw_pixel(xc, yc-r, color);
    draw_pixel(xc, yc+r, color);
      
    while (y > x)    //only formulate 1/8 of circle
    {
        if (d < 0) 
        {
            d+= dE;
            dE+=2, dSE+=2;
        } else {
            d+=dSE;
            dE+=2, dSE+=4;
            y--;
        }
        x++;
 
        draw_pixel(xc-x, yc-y, color);//upper left left
        draw_pixel(xc-y, yc-x, color);//upper upper left
        draw_pixel(xc+y, yc-x, color);//upper upper right
        draw_pixel(xc+x, yc-y, color);//upper right right
        draw_pixel(xc-x, yc+y, color);//lower left left
        draw_pixel(xc-y, yc+x, color);//lower lower left
        draw_pixel(xc+y, yc+x, color);//lower lower right
        draw_pixel(xc+x, yc+y, color);//lower right right
     }
}

void CircleOptimized(int xc, int yc, int r, int color)
{
    unsigned int x= r, y= 0;//local coords     
    int          cd2= 0;    //current distance squared - radius squared

    if (!r) return; 
    draw_pixel(xc-r, yc, color);
    draw_pixel(xc+r, yc, color);
    draw_pixel(xc, yc-r, color);
    draw_pixel(xc, yc+r, color);
 
    while (x > y)    //only formulate 1/8 of circle
    {
        cd2-= (--x) - (++y);
        if (cd2 < 0) cd2+=x++;

        draw_pixel(xc-x, yc-y, color);//upper left left
        draw_pixel(xc-y, yc-x, color);//upper upper left
        draw_pixel(xc+y, yc-x, color);//upper upper right
        draw_pixel(xc+x, yc-y, color);//upper right right
        draw_pixel(xc-x, yc+y, color);//lower left left
        draw_pixel(xc-y, yc+x, color);//lower lower left
        draw_pixel(xc+y, yc+x, color);//lower lower right
        draw_pixel(xc+x, yc+y, color);//lower right right
     } 
}

void RectangleDraw(int x0, int y0, int x1, int y1, int color)
{
	int x,y, tmp;
	if (x0>x1) { tmp = x1; x1 = x0; x0 = tmp; }
	if (y0>y1) { tmp = y1; y1 = y0; y0 = tmp; }
	if (x1 > 63) x1 = 63;
	if (y1 >127) y1 = 127;
    for (x=x0; x<=x1; x++) draw_pixel(x,y0,color);
	for (y=y0; y<=y1; y++) draw_pixel(x0,y,color);
	for (x=x0; x<=x1; x++) draw_pixel(x,y1,color);
 	for (y=y0; y<=y1; y++) draw_pixel(x1,y,color);		
}

void RectangleFill(int x0, int y0, int x1, int y1, int color)
{
	int x,y, tmp;
	if (x0>x1) { tmp = x1; x1 = x0; x0 = tmp; }
	if (y0>y1) { tmp = y1; y1 = y0; y0 = tmp; }
	if (x1 > 63) x1 = 63;
	if (y1 >127) y1 = 127; 
    for (x=x0; x<=x1; x++) {
		for (y=y0; y<=y1; y++) 	{
			draw_pixel(x,y,color);	
			}
		}
}

void Triangle (int x0, int y0, int x1, int y1, int x2, int y2, int color)
{
	LineOptimized(x0, y0, x1, y1, color);
	LineOptimized(x1, y1, x2, y2, color);
	LineOptimized(x0, y0, x2, y2, color);
}


