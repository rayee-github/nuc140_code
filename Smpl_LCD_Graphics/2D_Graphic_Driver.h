#ifndef __2D_Graphic_Driver_H__
#define __2D_Graphic_Driver_H__

extern void draw_pixel(int x, int y, int color);
     
extern void LineBresenham(int x1, int y1, int x2, int y2, int color);
extern void LineOptimized(int x1, int y1, int x2, int y2, int color);

extern void CircleBresenham(int xc, int yc, int r, int color);
extern void CircleMidpoint(int xc, int yc, int r, int color);
extern void CircleOptimized(int xc, int yc, int r, int color);  

extern void RectangleDraw(int x0, int y0, int x1, int y1, int color);
extern void RectangleFill(int x0, int y0, int x1, int y1, int color);

extern void Triangle (int x0, int y0, int x1, int y1, int x2, int y2, int color);
#endif
