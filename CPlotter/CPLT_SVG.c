/*
 * ADT: 'CPlotter'
 * CPlotter provides a basic, unified interface to different graphics
 * formats for simple 2D-drawings in C.
 *
 * This backend implements the vector-graphics format SVG, it writes
 * a SVG-(text)file (*.svg) according to SVG 1.1.
 *
 * Author and Copyright: Dipl.-Ing. Horst-W. Radners, Berlin, 2015-2016
 * License: LGPL 3.0, see http://www.gnu.org/licenses/lgpl-3.0.en.html
 */

#include <stdlib.h>
#include <stdio.h>

#include "CPLT_intern.h"

/* graphics context */
struct CPLT_gctx {
   CPLT_funcn_t *dispatch; /* functions dispatch table */
   FILE *fp;               /* filepointer */
   unsigned int pheight;   /* image's height [pix] */
   int curfontsize;        /* current font size [pix]*/
   int curlwd;             /* current linewidth [pix]*/
   char *curlsty;          /* current linestyle / dash array */
   int bgcol;              /* image's background color */
   int curcol[3];          /* current color, RGB [0,255]*/
};


/* global constants */
static char *fontface = "Verdana";  /* fontface used by string drawing */
static const float _EPS = 1.0E-5;   /* epsilon to 0 */


/* prototypes of internal helper functions */
CPLT_point_t _polar2cart_SVG(const float cx, const float cy,
                             const float radius, const float angle);
CPLT_funcn_t *_get_dispatchFuncs_SVG(void);


/*
 *******************************************************************************
 * API functions
 *******************************************************************************
 */

CPLT_gc_t CPLT_init_graphics_SVG(const unsigned int pwidth,
                                 const unsigned int pheight,
                                 char *plotfilename) {
   /* Initializes graphics of pwidth x pheight [pix] in graphics file
    * plotfilename, returns graphics-context pointer.
    * here SVG: opens plotfile, writes SVG-header */

   /* allocate memory for graphics context's data-struct */
   CPLT_gc_t gc = (CPLT_gc_t) malloc(sizeof(*gc));
   if (gc == NULL) {
      fprintf(stderr, " *** Not enough memory for graphics context!\n");
      return NULL;
   }

   /* open SVG-plotfile */
   if ((gc->fp = fopen(plotfilename, "w")) == NULL) {
      fprintf(stderr,
              " *** Can't open output plotfile '%s'!\n", plotfilename);
      return NULL;
   }

   /* write SVG-preamble */
   fprintf(gc->fp, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
   fprintf(gc->fp, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" "
           "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
   fprintf(gc->fp, "<svg xmlns=\"http://www.w3.org/2000/svg\" "
           "version=\"1.1\"\n");
   fprintf(gc->fp, "width=\"%dpx\" height=\"%dpx\">\n", pwidth, pheight);
   fprintf(gc->fp, "<title>%s</title>\n\n", plotfilename);

   /* defaults */
   gc->curcol[0] = 0;        /* current fg color: black: R */
   gc->curcol[1] = 0;        /* G */
   gc->curcol[2] = 0;        /* B */
   gc->pheight = pheight;     /* save image height for y-inversion */
   gc->curlwd = 1;            /* current linewidth [pix]*/
   gc->curlsty = "none";      /* current linestyle / dash array */
   gc->curfontsize = 12;      /* current fontsize [pix]*/

   /* fill whole canvas with white as background */
   fprintf(gc->fp, "<path d=\"\n");
   fprintf(gc->fp, "M %.2lf %.2lf\n",            0.,             0.);
   fprintf(gc->fp, "L %.2lf %.2lf\n",            0., (float)pheight);
   fprintf(gc->fp, "L %.2lf %.2lf\n", (float)pwidth, (float)pheight);
   fprintf(gc->fp, "L %.2lf %.2lf\n", (float)pwidth,             0.);
   fprintf(gc->fp, "z\"\n");
   fprintf(gc->fp, "fill=\"#%02X%02X%02X\" "
           "stroke=\"#%02X%02X%02X\"/>\n",
           255, 255, 255, 255, 255, 255);

   /* register dispatch table of our functions for generic callers */
   gc->dispatch = _get_dispatchFuncs_SVG();

   return gc;
}

/*
 *******************************************************************************
 */

void CPLT_draw_polyline_SVG(CPLT_gc_t gc, int numpts,
                            CPLT_point_t points[]) {
   /* Plots line through numpts 2D-points at given x/y-pairs in array points.
    * Draws line with current color and linewidth/style. */

   int i;

   if (gc == NULL) return;
   if (numpts <= 1) return;

   if (numpts == 2) {   /* line */
      fprintf(gc->fp,
              "<line x1=\"%.2lf\" y1=\"%.2lf\" x2=\"%.2lf\" y2=\"%.2lf\"\n",
              points[0].x, gc->pheight - points[0].y,
              points[1].x, gc->pheight - points[1].y);

   } else {             /* polyline */

      fprintf(gc->fp, "<polyline points=\"\n");
      for (i = 0; i < numpts; i++) {
         fprintf(gc->fp, "%.2lf %.2lf\n",
                 points[i].x, gc->pheight - points[i].y);
      }
      fprintf(gc->fp, "\" fill=\"none\" ");
   }
   fprintf(gc->fp, "stroke=\"#%02X%02X%02X\"",
           gc->curcol[0], gc->curcol[1], gc->curcol[2]);
   if (gc->curlwd != 1)
      fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
   if (strcmp(gc->curlsty, "none") != 0)
      fprintf(gc->fp, " stroke-dasharray=\"%s\"", gc->curlsty);
   fprintf(gc->fp, "/>\n");

}

/*
 *******************************************************************************
 */

void CPLT_draw_polygon_SVG(CPLT_gc_t gc, int numpts,
                           CPLT_point_t points[]) {
   /* Plots (automatically closed) 2D-polygon with numpts points at given
    * x/y-pairs in array points.
    * Draws outline of the polygon with current color and linewidth/style.
    * here SVG: strokes (closed) new path */

   int i;

   if (gc == NULL) return;
   if (numpts <= 1) return;

   fprintf(gc->fp, "<polygon points=\"\n");
   for (i = 0; i < numpts; i++) {
      fprintf(gc->fp, "%.2lf %.2lf\n",
              points[i].x, gc->pheight - points[i].y);
   }
   fprintf(gc->fp, "\" fill=\"none\"");
   fprintf(gc->fp, " stroke=\"#%02X%02X%02X\"",
           gc->curcol[0], gc->curcol[1], gc->curcol[2]);
   if (gc->curlwd != 1)
      fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
   if (strcmp(gc->curlsty, "none") != 0)
      fprintf(gc->fp, " stroke-dasharray=\"%s\"", gc->curlsty);
   fprintf(gc->fp, "/>\n");

}

/*
 *******************************************************************************
 */

void CPLT_draw_filledPolygon_SVG(CPLT_gc_t gc, int numpts,
                                 CPLT_point_t points[]) {
   /* Plots (automatically closed) 2D-polygon with numpts points at given
    * x/y-pairs in array points.
    * Fills and strokes the polygon with current color.
    * here SVG: fills + strokes (closed) new path */

   int i;

   if (gc == NULL) return;
   if (numpts <= 1) return;

   fprintf(gc->fp, "<polygon points=\"\n");
   for (i = 0; i < numpts; i++) {
      fprintf(gc->fp, "%.2lf %.2lf\n",
              points[i].x, gc->pheight - points[i].y);
   }
   fprintf(gc->fp, "\" fill=\"#%02X%02X%02X\"",
           gc->curcol[0], gc->curcol[1], gc->curcol[2]);
   fprintf(gc->fp, " stroke=\"#%02X%02X%02X\"",
           gc->curcol[0], gc->curcol[1], gc->curcol[2]);
   if (gc->curlwd != 1)
      fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
   if (strcmp(gc->curlsty, "none") != 0)
      fprintf(gc->fp, " stroke-dasharray=\"%s\"", gc->curlsty);
   fprintf(gc->fp, "/>\n");

}

/*
 *******************************************************************************
 */

void CPLT_draw_arc_SVG(CPLT_gc_t gc, const float cx, const float cy,
                       const float radius,
                       const float start, const float end) {
   /* Plots partial circle at given x/y-center with the specified radius.
    * The arc begins at the position in degrees specified by angle start
    * and ends at the position specified by angle end.
    * A full circle can be drawn by beginning from start=0 degrees and
    * ending at end=360 degrees.
    * Both angles turn counterclockwise, i.e. mathematically positive.
    * Draws outline of the arc with current color and linewidth/style. */

   CPLT_point_t start_pt, end_pt;
   int largeArc = 0, arcSweep = 1;
   float da, ciy;

   if (gc == NULL) return;

   ciy = gc->pheight - cy;

   if (start == 0 && end == 360) {     /* full circle */

      fprintf(gc->fp, "<circle cx=\"%d\" cy=\"%d\" r=\"%d\"\n",
              (int)cx, (int)ciy, (int)radius);

   } else {   /* partial arc */

      start_pt = _polar2cart_SVG(cx, cy, radius, end);
      end_pt   = _polar2cart_SVG(cx, cy, radius, start);

      da = end - start;
      if (da < 0) da += 360;
      largeArc = da > 180 ? 1 : 0;

      fprintf(gc->fp, "<path d=\"\n");
      fprintf(gc->fp, "M %.2lf %.2lf\n", start_pt.x, gc->pheight - start_pt.y);
      fprintf(gc->fp, "A %.2lf %.2lf %d %d %d %.2lf %.2lf\"\n",
              radius, radius, 0, largeArc, arcSweep,
              end_pt.x, gc->pheight - end_pt.y);

   }
   fprintf(gc->fp, "fill=\"none\" stroke=\"#%02X%02X%02X\"",
           gc->curcol[0], gc->curcol[1], gc->curcol[2]);
   if (gc->curlwd != 1)
      fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
   if (strcmp(gc->curlsty, "none") != 0)
      fprintf(gc->fp, " stroke-dasharray=\"%s\"", gc->curlsty);
   fprintf(gc->fp, "/>\n");

}

/*
 *******************************************************************************
 */

void CPLT_draw_filledArc_SVG(CPLT_gc_t gc, const float cx, const float cy,
                             const float radius,
                             const float start, const float end) {
   /* Plots partial circle at given x/y-center with the specified radius.
    * The arc begins at the position in degrees specified by angle start
    * and ends at the position specified by angle end.
    * A full circle can be drawn by beginning from start=0 degrees and
    * ending at end=360 degrees.
    * Both angles turn counterclockwise, i.e. mathematically positive.
    * Fills and strokes the arc/"pie slice" with current color. */

   CPLT_point_t start_pt, end_pt;
   int largeArc = 0, arcSweep = 1;
   float da, ciy;

   if (gc == NULL) return;

   ciy = gc->pheight - cy;

   if (start == 0 && end == 360) {     /* full circle */

      fprintf(gc->fp, "<circle cx=\"%d\" cy=\"%d\" r=\"%d\"\n",
              (int)cx, (int)ciy, (int)radius);

   } else {   /* partial arc */

      start_pt = _polar2cart_SVG(cx, cy, radius, end);
      end_pt   = _polar2cart_SVG(cx, cy, radius, start);

      da = end - start;
      if (da < 0) da += 360;
      largeArc = da > 180 ? 1 : 0;

      fprintf(gc->fp, "<path d=\"\n");
      fprintf(gc->fp, "M %.2lf %.2lf\n", start_pt.x, gc->pheight - start_pt.y);
      fprintf(gc->fp, "A %.2lf %.2lf %d %d %d %.2lf %.2lf\n",
              radius, radius, 0, largeArc, arcSweep,
              end_pt.x, gc->pheight - end_pt.y);
      fprintf(gc->fp, "L %.2lf %.2lf\n", cx, ciy);
      fprintf(gc->fp, "L %.2lf %.2lf\n", start_pt.x, gc->pheight - start_pt.y);
      fprintf(gc->fp, "z\"\n");

   }
   fprintf(gc->fp, "fill=\"#%02X%02X%02X\"",
           gc->curcol[0], gc->curcol[1], gc->curcol[2]);
   fprintf(gc->fp, " stroke=\"#%02X%02X%02X\"",
           gc->curcol[0], gc->curcol[1], gc->curcol[2]);
   if (gc->curlwd != 1)
      fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
   if (strcmp(gc->curlsty, "none") != 0)
      fprintf(gc->fp, " stroke-dasharray=\"%s\"", gc->curlsty);
   fprintf(gc->fp, "/>\n");

}

/*
 *******************************************************************************
 */

void CPLT_draw_curve_SVG(CPLT_gc_t gc, CPLT_point_t points[]) {
   /* Plots a Bezier curve segment by 4 given 2D-points as x/y-pairs
    * in array points. points[0] is start, points[3] end point of curve,
    * points[1] and points[2] are the Bezier control points.
    * Draws line with current color and linewidth/style. */

   int i;

   if (gc == NULL) return;

   fprintf(gc->fp, "<path d=\"\n");
   fprintf(gc->fp, "M %.2lf %.2lf\nC",
           points[0].x, gc->pheight - points[0].y);
   for (i = 1; i < 4; i++) {
      fprintf(gc->fp, " %.2lf %.2lf",
              points[i].x, gc->pheight - points[i].y);
   }
   fprintf(gc->fp, "\"\n");
   fprintf(gc->fp, "fill=\"none\" stroke=\"#%02X%02X%02X\"",
           gc->curcol[0], gc->curcol[1], gc->curcol[2]);
   if (gc->curlwd != 1)
      fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
   if (strcmp(gc->curlsty, "none") != 0)
      fprintf(gc->fp, " stroke-dasharray=\"%s\"", gc->curlsty);
   fprintf(gc->fp, "/>\n");

}

/*
 *******************************************************************************
 */

void CPLT_draw_marker_SVG(CPLT_gc_t gc, const float cx, const float cy,
                          const int wd, const int symbol) {
   /* Plots a marker of width/height wd [pix] centered at cx/cy, with
    * current linewidth and color.
    * symbol [0-7] enumerates the marker's form:
    * 0: X
    * 1: +
    * 2: star
    * 3: circle
    * 4: square
    * 5: square, turned 45°
    * 6: triangle, tip up
    * 7: triangle, tip down
    */

   float w, ciy;

   if (gc == NULL) return;

   w = 0.5 * wd;
   ciy = gc->pheight - cy;

   switch (symbol) {
      case 1:        /* + */
         fprintf(gc->fp, "<path d=\"\n");
         fprintf(gc->fp, "M %.2lf %.2lf\n", cx - w, ciy);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx + w, ciy);
         fprintf(gc->fp, "M %.2lf %.2lf\n", cx, ciy - w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx, ciy + w);
         fprintf(gc->fp, "\"\n");
         fprintf(gc->fp, "fill=\"none\" stroke=\"#%02X%02X%02X\"",
                 gc->curcol[0], gc->curcol[1], gc->curcol[2]);
         if (gc->curlwd != 1)
            fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
         fprintf(gc->fp, "/>\n");
         break;

      case 2:        /* star */
         fprintf(gc->fp, "<path d=\"\n");
         fprintf(gc->fp, "M %.2lf %.2lf\n", cx - w, ciy);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx + w, ciy);
         fprintf(gc->fp, "M %.2lf %.2lf\n", cx, ciy - w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx, ciy + w);
         fprintf(gc->fp, "M %.2lf %.2lf\n", cx - w, ciy - w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx + w, ciy + w);
         fprintf(gc->fp, "M %.2lf %.2lf\n", cx - w, ciy + w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx + w, ciy - w);
         fprintf(gc->fp, "\"\n");
         fprintf(gc->fp, "fill=\"none\" stroke=\"#%02X%02X%02X\"",
                 gc->curcol[0], gc->curcol[1], gc->curcol[2]);
         if (gc->curlwd != 1)
            fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
         fprintf(gc->fp, "/>\n");
         break;

      case 3:        /* circle */
         fprintf(gc->fp, "<circle cx=\"%d\" cy=\"%d\" r=\"%d\"\n",
                 (int)cx, (int)ciy, (int)w);
         fprintf(gc->fp, "fill=\"none\" stroke=\"#%02X%02X%02X\"",
                 gc->curcol[0], gc->curcol[1], gc->curcol[2]);
         if (gc->curlwd != 1)
            fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
         fprintf(gc->fp, "/>\n");
         fprintf(gc->fp, "<path d=\"\n");
         fprintf(gc->fp, "M %.2lf %.2lf\n", cx,     ciy + w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx,     ciy);
         fprintf(gc->fp, "\"\n");
         fprintf(gc->fp, "fill=\"none\" stroke=\"#%02X%02X%02X\"",
                 gc->curcol[0], gc->curcol[1], gc->curcol[2]);
         if (gc->curlwd != 1)
            fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
         fprintf(gc->fp, "/>\n");
         break;

      case 4:        /* square */
         fprintf(gc->fp, "<path d=\"\n");
         fprintf(gc->fp, "M %.2lf %.2lf\n", cx - w, ciy - w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx + w, ciy - w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx + w, ciy + w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx - w, ciy + w);
         fprintf(gc->fp, "z\"\n");
         fprintf(gc->fp, "fill=\"none\" stroke=\"#%02X%02X%02X\"",
                 gc->curcol[0], gc->curcol[1], gc->curcol[2]);
         if (gc->curlwd != 1)
            fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
         fprintf(gc->fp, "/>\n");
         fprintf(gc->fp, "<path d=\"\n");
         fprintf(gc->fp, "M %.2lf %.2lf\n", cx,     ciy + w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx,     ciy);
         fprintf(gc->fp, "\"\n");
         fprintf(gc->fp, "fill=\"none\" stroke=\"#%02X%02X%02X\"",
                 gc->curcol[0], gc->curcol[1], gc->curcol[2]);
         if (gc->curlwd != 1)
            fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
         fprintf(gc->fp, "/>\n");
         break;

      case 5:        /* square, turned 45° */
         fprintf(gc->fp, "<path d=\"\n");
         fprintf(gc->fp, "M %.2lf %.2lf\n", cx,     ciy + w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx + w, ciy);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx,     ciy - w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx - w, ciy);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx,     ciy + w);
         fprintf(gc->fp, "z\"\n");
         fprintf(gc->fp, "fill=\"none\" stroke=\"#%02X%02X%02X\"",
                 gc->curcol[0], gc->curcol[1], gc->curcol[2]);
         if (gc->curlwd != 1)
            fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
         fprintf(gc->fp, "/>\n");
         fprintf(gc->fp, "<path d=\"\n");
         fprintf(gc->fp, "M %.2lf %.2lf\n", cx,     ciy + w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx,     ciy);
         fprintf(gc->fp, "\"\n");
         fprintf(gc->fp, "fill=\"none\" stroke=\"#%02X%02X%02X\"",
                 gc->curcol[0], gc->curcol[1], gc->curcol[2]);
         if (gc->curlwd != 1)
            fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
         fprintf(gc->fp, "/>\n");
         break;

      case 6:        /* triangle, tip up */
         fprintf(gc->fp, "<path d=\"\n");
         fprintf(gc->fp, "M %.2lf %.2lf\n", cx,     ciy - w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx + w, ciy + w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx - w, ciy + w);
         fprintf(gc->fp, "z\"\n");
         fprintf(gc->fp, "fill=\"none\" stroke=\"#%02X%02X%02X\"",
                 gc->curcol[0], gc->curcol[1], gc->curcol[2]);
         if (gc->curlwd != 1)
            fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
         fprintf(gc->fp, "/>\n");
         fprintf(gc->fp, "<path d=\"\n");
         fprintf(gc->fp, "M %.2lf %.2lf\n", cx,     ciy + w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx,     ciy);
         fprintf(gc->fp, "\"\n");
         fprintf(gc->fp, "fill=\"none\" stroke=\"#%02X%02X%02X\"",
                 gc->curcol[0], gc->curcol[1], gc->curcol[2]);
         if (gc->curlwd != 1)
            fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
         fprintf(gc->fp, "/>\n");
         break;

      case 7:        /* triangle, tip down */
         fprintf(gc->fp, "<path d=\"\n");
         fprintf(gc->fp, "M %.2lf %.2lf\n", cx - w, ciy - w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx + w, ciy - w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx,     ciy + w);
         fprintf(gc->fp, "z\"\n");
         fprintf(gc->fp, "fill=\"none\" stroke=\"#%02X%02X%02X\"",
                 gc->curcol[0], gc->curcol[1], gc->curcol[2]);
         if (gc->curlwd != 1)
            fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
         fprintf(gc->fp, "/>\n");
         fprintf(gc->fp, "<path d=\"\n");
         fprintf(gc->fp, "M %.2lf %.2lf\n", cx,     ciy + w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx,     ciy);
         fprintf(gc->fp, "\"\n");
         fprintf(gc->fp, "fill=\"none\" stroke=\"#%02X%02X%02X\"",
                 gc->curcol[0], gc->curcol[1], gc->curcol[2]);
         if (gc->curlwd != 1)
            fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
         fprintf(gc->fp, "/>\n");
         break;

      default:       /* X */
         fprintf(gc->fp, "<path d=\"\n");
         fprintf(gc->fp, "M %.2lf %.2lf\n", cx - w, ciy - w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx + w, ciy + w);
         fprintf(gc->fp, "M %.2lf %.2lf\n", cx - w, ciy + w);
         fprintf(gc->fp, "L %.2lf %.2lf\n", cx + w, ciy - w);
         fprintf(gc->fp, "\"\n");
         fprintf(gc->fp, "fill=\"none\" stroke=\"#%02X%02X%02X\"",
                 gc->curcol[0], gc->curcol[1], gc->curcol[2]);
         if (gc->curlwd != 1)
            fprintf(gc->fp, " stroke-width=\"%d\"", gc->curlwd);
         fprintf(gc->fp, "/>\n");
         break;
   }

}

/*
 *******************************************************************************
 */

void CPLT_draw_text_SVG(CPLT_gc_t gc, const float x, const float y,
                        char *anchor, float angle, char *text) {
   /* Plots Latin-1 encoded text of current fontsize at angle degrees
    * with current color (although the currrent linewidth is ignored,
    * the actual strokewidth used depends on the current fontsize,
    * see CPLT_set_fontsize()).
    * anchor sets the reference point of the text enclosing
    * rectangle positioned at x/y:
    *
    *    nw--------n--------ne    For example,
    *    |         |         |    if anchor = "sw", the lower-left corner
    *    w---------c---------e    of text is positioned at x/y,
    *    |         |         |    if anchor = "c", the text is centered at x/y.
    *    sw--------s--------se
    */

   /* horizontal and vertical anchor positions */
   static char *tanchor[10] = {
      "",
      "start", "middle", "end",
      "start", "middle", "end",
      "start", "middle", "end"
   };
   static char *tbase[10] = {
      "",
      "text-after-edge",  "text-after-edge",  "text-after-edge",
      "middle",           "middle",           "middle",
      "text-before-edge", "text-before-edge", "text-before-edge"
   };
   int anchor_num;

   if (gc == NULL) return;
   anchor_num = _anchor_num_of(anchor);
   if (anchor_num == 0) anchor_num = 1;

   fprintf(gc->fp, "<text transform=\"translate(%.2lf,%.2lf)",
           x, gc->pheight - y);
   if (fabs(angle) > _EPS) fprintf(gc->fp, " rotate(%.2lf)", -angle);
   fprintf(gc->fp, "\"\n");
   fprintf(gc->fp, "font-family=\"%s\" font-size=\"%d\" "
           "fill=\"#%02X%02X%02X\"\n", fontface, gc->curfontsize,
           gc->curcol[0], gc->curcol[1], gc->curcol[2]);
   fprintf(gc->fp, "text-anchor=\"%s\" dominant-baseline=\"%s\">\n",
           tanchor[anchor_num], tbase[anchor_num]);
   fprintf(gc->fp, "%s\n", text);
   fprintf(gc->fp, "</text>\n");

}

/*
 *******************************************************************************
 */

void CPLT_set_fontsize_SVG(CPLT_gc_t gc, const float fontsize) {
   /* Sets current fontsize [pix] for CPLT_draw_text().
    * (Preset: fontsize=12.0) */

   if (gc == NULL) return;

   gc->curfontsize = (int)fontsize;

}

/*
 *******************************************************************************
 */

void CPLT_set_color_SVG(CPLT_gc_t gc, float r, float g, float b) {
   /* Sets current color of RGB values [0,1].
    * (Preset: r=0., g=0., b=0., i.e. black) */

   if (gc == NULL) return;

   r = r < 0. ? 0. : r > 1. ? 1. : r;
   g = g < 0. ? 0. : g > 1. ? 1. : g;
   b = b < 0. ? 0. : b > 1. ? 1. : b;

   /* internally saved as [0,255] */
   gc->curcol[0] = (int)(r * 255);
   gc->curcol[1] = (int)(g * 255);
   gc->curcol[2] = (int)(b * 255);

}

/*
 *******************************************************************************
 */

void CPLT_set_linewidth_SVG(CPLT_gc_t gc, const float w) {
   /* Sets current linewidth w [pix]. (Preset: w=1.0) */

   int p;

   if (gc == NULL) return;

   p = _rnd(w);
   gc->curlwd = p < 1 ? 1 : p;

}

/*
 *******************************************************************************
 */

void CPLT_set_linestyle_SVG(CPLT_gc_t gc, const CPLT_lnstyle_t s) {
   /* Sets current linestyle s [enumeration].
    * (Preset: s=CPLT_SolidLine) */

   if (gc == NULL) return;

   /* dash array values */
   switch (s) {
      case CPLT_SolidLine:
         gc->curlsty = "none";
         break;
      case CPLT_DashLine:
         gc->curlsty = "4 2";
         break;
      case CPLT_DotLine:
         gc->curlsty = "1 2";
         break;
      case CPLT_DashDotLine:
         gc->curlsty = "4 2 1 2";
         break;
      case CPLT_DashDotDotLine:
         gc->curlsty = "4 2 1 2 1 2";
         break;
      default:    /* solid */
         gc->curlsty = "none";
         break;
   }

}

/*
 *******************************************************************************
 */

void CPLT_finish_graphics_SVG(CPLT_gc_t gc) {
   /* Finishes graphics, closes plotfile, destroys graphics context.
    * here SVG: writes SVG-trailer to and closes plotfile */

   if (gc == NULL) return;

   fprintf(gc->fp, "\n</svg>\n");
   fclose(gc->fp);

   free(gc->dispatch);
   free(gc);

}

/*
 *******************************************************************************
 * internal helper functions
 *******************************************************************************
 */

CPLT_point_t _polar2cart_SVG(const float cx, const float cy,
                             const float radius, const float angle) {
   /* Internal helper func: converts polar coords (radius [pix], angle [deg])
    * to cartesian coords [pix] */

   CPLT_point_t p = {
      cx + radius * cos(angle * DEG2RAD),
      cy + radius * sin(angle * DEG2RAD)
   };
   return p;
}

/*
 *******************************************************************************
 * management function to assemble the dispatch table/struct
 * for the generic callers
 *******************************************************************************
 */

CPLT_funcn_t *_get_dispatchFuncs_SVG(void) {

   CPLT_funcn_t *dpt = (CPLT_funcn_t *) malloc(sizeof(*dpt));
   if (dpt == NULL) {
      fprintf(stderr, " *** Not enough memory for dispatch table!\n");
      return NULL;
   }

   dpt->INIT  = &CPLT_init_graphics_SVG;
   dpt->PLINE = &CPLT_draw_polyline_SVG;
   dpt->PGON  = &CPLT_draw_polygon_SVG;
   dpt->PGONF = &CPLT_draw_filledPolygon_SVG;
   dpt->ARC   = &CPLT_draw_arc_SVG;
   dpt->ARCF  = &CPLT_draw_filledArc_SVG;
   dpt->CURVE = &CPLT_draw_curve_SVG;
   dpt->MARK  = &CPLT_draw_marker_SVG;
   dpt->TEXT  = &CPLT_draw_text_SVG;
   dpt->FSIZE = &CPLT_set_fontsize_SVG;
   dpt->COLR  = &CPLT_set_color_SVG;
   dpt->LNWD  = &CPLT_set_linewidth_SVG;
   dpt->LNSTY = &CPLT_set_linestyle_SVG;
   dpt->FINI  = &CPLT_finish_graphics_SVG;

   return dpt;
}

/*
 *******************************************************************************
 */

