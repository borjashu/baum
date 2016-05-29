/*
 * ADT: 'CPlotter'
 * CPlotter provides a basic, unified interface to different graphics
 * formats for simple 2D-drawings in C.
 *
 * This backend implements the vector-graphics format EPS, it writes
 * an EPS-(text)file (*.eps) according to PS-Adobe-3.0 EPSF-3.0.
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
};


/* global constants */
/* Type1-fontface used by string drawing */
static char *fontface = "Helvetica";


/* prototypes of internal helper functions */
void _create_poly_EPS(CPLT_gc_t gc, int numpts, CPLT_point_t points[]);
CPLT_funcn_t *_get_dispatchFuncs_EPS(void);

/*
 *******************************************************************************
 * API functions
 *******************************************************************************
 */

CPLT_gc_t CPLT_init_graphics_EPS(const unsigned int pwidth,
                                 const unsigned int pheight,
                                 char *plotfilename) {
   /* Initializes graphics of pwidth x pheight [pix] in graphics file
    * plotfilename, returns graphics-context pointer.
    * here EPS: opens plotfile, writes EPS-header */

   /* allocate memory for graphics context's data-struct */
   CPLT_gc_t gc = (CPLT_gc_t) malloc(sizeof(*gc));
   if (gc == NULL) {
      fprintf(stderr, " *** Not enough memory for graphics context!\n");
      return NULL;
   }

   /* open EPS-plotfile */
   if ((gc->fp = fopen(plotfilename, "w")) == NULL) {
      fprintf(stderr,
              " *** Can't open output plotfile '%s'!\n", plotfilename);
      return NULL;
   }

   time_t now = time(NULL);

   /* write EPS-preamble, define some useful procedures */
   fprintf(gc->fp, "%%!PS-Adobe-3.0 EPSF-3.0\n");
   fprintf(gc->fp, "%%%%Title: %s\n", plotfilename);
   fprintf(gc->fp, "%%%%Creator: CPlotter\n");
   fprintf(gc->fp, "%%%%CreationDate: %s", ctime(&now));
   fprintf(gc->fp, "%%%%BoundingBox: 0 0 %u %u\n", pwidth, pheight);
   fprintf(gc->fp, "%%%%Pages: 1\n");
   fprintf(gc->fp, "%%%%EndComments\n");
   fprintf(gc->fp, "%%%%BeginProlog\n");
   fprintf(gc->fp, "/g {gsave} bind def\n");
   fprintf(gc->fp, "/G {grestore} bind def\n");
   fprintf(gc->fp, "/P {currentpoint} bind def\n");
   fprintf(gc->fp, "/a {arc} bind def\n");
   fprintf(gc->fp, "/C {curveto} bind def\n");
   fprintf(gc->fp, "/c {setrgbcolor} bind def\n");
   fprintf(gc->fp, "/d {0 setdash} bind def\n");
   fprintf(gc->fp, "/f {fill} bind def\n");
   fprintf(gc->fp, "/l {lineto} bind def\n");
   fprintf(gc->fp, "/m {moveto} bind def\n");
   fprintf(gc->fp, "/rm {rmoveto} bind def\n");
   fprintf(gc->fp, "/n {newpath} bind def\n");
   fprintf(gc->fp, "/p {closepath} bind def\n");
   fprintf(gc->fp, "/r {rotate} bind def\n");
   fprintf(gc->fp, "/s {stroke} bind def\n");
   fprintf(gc->fp, "/t {translate} bind def\n");
   fprintf(gc->fp, "/T {m show} bind def\n");
   fprintf(gc->fp, "/T1 {/A exch def m g P t A r show G} bind def\n");
   fprintf(gc->fp, "/T2 {/A exch def /Y exch def /X exch def "
           "/S exch def\n");
   fprintf(gc->fp, "     S stringwidth pop /B exch def\n");
   fprintf(gc->fp, "     X Y m g P t A r\n");
   fprintf(gc->fp, "     B 2 div neg 0 rm S show G} bind def\n");
   fprintf(gc->fp, "/T3 {/A exch def /Y exch def /X exch def "
           "/S exch def\n");
   fprintf(gc->fp, "     S stringwidth pop /B exch def\n");
   fprintf(gc->fp, "     X Y m g P t A r\n");
   fprintf(gc->fp, "     B neg 0 rm S show G} bind def\n");
   fprintf(gc->fp, "/T4 {/A exch def /Y exch def /X exch def "
           "/S exch def\n");
   fprintf(gc->fp, "     S stringwidth pop /B exch def\n");
   fprintf(gc->fp, "     X Y m g P t A r\n");
   fprintf(gc->fp, "     0 FH 2 div neg rm S show G} bind def\n");
   fprintf(gc->fp, "/T5 {/A exch def /Y exch def /X exch def "
           "/S exch def\n");
   fprintf(gc->fp, "     S stringwidth pop /B exch def\n");
   fprintf(gc->fp, "     X Y m g P t A r\n");
   fprintf(gc->fp, "     B 2 div neg FH 2 div neg rm S show G} bind def\n");
   fprintf(gc->fp, "/T6 {/A exch def /Y exch def /X exch def "
           "/S exch def\n");
   fprintf(gc->fp, "     S stringwidth pop /B exch def\n");
   fprintf(gc->fp, "     X Y m g P t A r\n");
   fprintf(gc->fp, "     B neg FH 2 div neg rm S show G} bind def\n");
   fprintf(gc->fp, "/T7 {/A exch def /Y exch def /X exch def "
           "/S exch def\n");
   fprintf(gc->fp, "     S stringwidth pop /B exch def\n");
   fprintf(gc->fp, "     X Y m g P t A r\n");
   fprintf(gc->fp, "     0 FH neg rm S show G} bind def\n");
   fprintf(gc->fp, "/T8 {/A exch def /Y exch def /X exch def "
           "/S exch def\n");
   fprintf(gc->fp, "     S stringwidth pop /B exch def\n");
   fprintf(gc->fp, "     X Y m g P t A r\n");
   fprintf(gc->fp, "     B 2 div neg FH neg rm S show G} bind def\n");
   fprintf(gc->fp, "/T9 {/A exch def /Y exch def /X exch def "
           "/S exch def\n");
   fprintf(gc->fp, "     S stringwidth pop /B exch def\n");
   fprintf(gc->fp, "     X Y m g P t A r\n");
   fprintf(gc->fp, "     B neg FH neg rm S show G} bind def\n");
   fprintf(gc->fp, "/w {setlinewidth} bind def\n");
   fprintf(gc->fp, "%%\n%% calculate character height FH of current font\n");
   fprintf(gc->fp, "/calc_FH {g n 0 0 m\n");
   fprintf(gc->fp, "   (M) true charpath flattenpath pathbbox\n");
   fprintf(gc->fp, "   ceiling /FH exch def pop pop pop\n");
   fprintf(gc->fp, "   G} def\n");
   fprintf(gc->fp, "%%\n%% change encoding to ISO8859-1\n");
   fprintf(gc->fp, "/ISOfindfont {\n");
   fprintf(gc->fp, "   dup 100 string cvs (ISO-) exch concatstrings cvn exch\n");
   fprintf(gc->fp, "   findfont dup maxlength dict begin\n");
   fprintf(gc->fp, "     { 1 index /FID ne {def}{pop pop} ifelse } forall\n");
   fprintf(gc->fp, "     /Encoding ISOLatin1Encoding def\n");
   fprintf(gc->fp, "     currentdict\n");
   fprintf(gc->fp, "   end definefont} def\n%%\n");
   fprintf(gc->fp, "%%%%EndProlog\n");
   fprintf(gc->fp, "%%%%BeginSetup\n");
   fprintf(gc->fp, "0.5 w\n");
   fprintf(gc->fp, "3 setmiterlimit\n");
   fprintf(gc->fp, "0 0 0 c\n");
   fprintf(gc->fp, "/%s ISOfindfont 12 scalefont setfont calc_FH\n", fontface);
   fprintf(gc->fp, "%%%%EndSetup\n\n");
   fprintf(gc->fp, "%%%%Page: 1 1\n");

   /* register dispatch table of our functions for generic callers */
   gc->dispatch = _get_dispatchFuncs_EPS();

   return gc;
}

/*
 *******************************************************************************
 */

void CPLT_draw_polyline_EPS(CPLT_gc_t gc, int numpts,
                            CPLT_point_t points[]) {
   /* Plots line through numpts 2D-points at given x/y-pairs in array points.
    * Draws line with current color and linewidth/style. */

   if (gc == NULL) return;
   if (numpts <= 1) return;

   fprintf(gc->fp, "n\n");
   _create_poly_EPS(gc, numpts, points);
   fprintf(gc->fp, "s\n");

}

/*
 *******************************************************************************
 */

void CPLT_draw_polygon_EPS(CPLT_gc_t gc, int numpts,
                           CPLT_point_t points[]) {
   /* Plots (automatically closed) 2D-polygon with numpts points at given
    * x/y-pairs in array points.
    * Draws outline of the polygon with current color and linewidth/style.
    * here EPS: strokes (closed) new path */

   if (gc == NULL) return;
   if (numpts <= 1) return;

   fprintf(gc->fp, "n\n");
   _create_poly_EPS(gc, numpts, points);
   fprintf(gc->fp, "p s\n");

}

/*
 *******************************************************************************
 */

void CPLT_draw_filledPolygon_EPS(CPLT_gc_t gc, int numpts,
                                 CPLT_point_t points[]) {
   /* Plots (automatically closed) 2D-polygon with numpts points at given
    * x/y-pairs in array points.
    * Fills + strokes the polygon with current color.
    * here EPS: fills + strokes (closed) new path */

   if (gc == NULL) return;
   if (numpts <= 1) return;

   fprintf(gc->fp, "n\n");
   _create_poly_EPS(gc, numpts, points);
   fprintf(gc->fp, "p g f G s\n");

}

/*
 *******************************************************************************
 */

void CPLT_draw_arc_EPS(CPLT_gc_t gc, const float cx, const float cy,
                       const float radius,
                       const float start, const float end) {
   /* Plots partial circle at given x/y-center with the specified radius.
    * The arc begins at the position in degrees specified by angle start
    * and ends at the position specified by angle end.
    * A full circle can be drawn by beginning from start=0 degrees and
    * ending at end=360 degrees.
    * Both angles turn counterclockwise, i.e. mathematically positive.
    * Draws outline of the arc with current color and linewidth/style. */

   if (gc == NULL) return;

   fprintf(gc->fp, "n %.2lf %.2lf %.2lf %.2lf %.2lf a s\n",
           cx, cy, radius, start, end);

}

/*
 *******************************************************************************
 */

void CPLT_draw_filledArc_EPS(CPLT_gc_t gc, const float cx, const float cy,
                             const float radius,
                             const float start, const float end) {
   /* Plots partial circle at given x/y-center with the specified radius.
    * The arc begins at the position in degrees specified by angle start
    * and ends at the position specified by angle end.
    * A full circle can be drawn by beginning from start=0 degrees and
    * ending at end=360 degrees.
    * Both angles turn counterclockwise, i.e. mathematically positive.
    * Fills + strokes the arc/"pie slice" with current color. */

   if (gc == NULL) return;

   fprintf(gc->fp, "n %.2lf %.2lf m\n", cx, cy);
   fprintf(gc->fp, "%.2lf %.2lf %.2lf %.2lf %.2lf a\n",
           cx, cy, radius, start, end);
   fprintf(gc->fp, "%.2lf %.2lf l\n", cx, cy);
   fprintf(gc->fp, "p g f G s\n");

}

/*
 *******************************************************************************
 */

void CPLT_draw_curve_EPS(CPLT_gc_t gc, CPLT_point_t points[]) {
   /* Plots a Bezier curve segment by 4 given 2D-points as x/y-pairs
    * in array points. points[0] is start, points[3] end point of curve,
    * points[1] and points[2] are the Bezier control points.
    * Draws line with current color and linewidth/style. */

   int i;

   if (gc == NULL) return;

   fprintf(gc->fp, "%.2lf %.2lf m\n", points[0].x, points[0].y);
   for (i = 1; i < 4; i++) {
      fprintf(gc->fp, "%.2lf %.2lf ", points[i].x, points[i].y);
   }
   fprintf(gc->fp, "C s\n");

}

/*
 *******************************************************************************
 */

void CPLT_draw_marker_EPS(CPLT_gc_t gc, const float cx, const float cy,
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

   if (gc == NULL) return;

   float w = 0.5 * wd;

   switch (symbol) {
      case 1:        /* + */
         fprintf(gc->fp, "n %.2lf %.2lf m\n", cx - w, cy);
         fprintf(gc->fp, "%.2lf %.2lf l\n",   cx + w, cy);
         fprintf(gc->fp, "%.2lf %.2lf m\n",   cx, cy - w);
         fprintf(gc->fp, "%.2lf %.2lf l s\n", cx, cy + w);
         break;

      case 2:        /* star */
         fprintf(gc->fp, "n %.2lf %.2lf m\n", cx - w, cy);
         fprintf(gc->fp, "%.2lf %.2lf l\n",   cx + w, cy);
         fprintf(gc->fp, "%.2lf %.2lf m\n",   cx,     cy - w);
         fprintf(gc->fp, "%.2lf %.2lf l\n",   cx,     cy + w);
         fprintf(gc->fp, "%.2lf %.2lf m\n",   cx - w, cy - w);
         fprintf(gc->fp, "%.2lf %.2lf l\n",   cx + w, cy + w);
         fprintf(gc->fp, "%.2lf %.2lf m\n",   cx - w, cy + w);
         fprintf(gc->fp, "%.2lf %.2lf l s\n", cx + w, cy - w);
         break;

      case 3:        /* circle */
         fprintf(gc->fp, "n %.2lf %.2lf %.2lf %.2lf %.2lf a s\n",
                 cx, cy, w, 0., 360.);
         fprintf(gc->fp, "n %.2lf %.2lf m\n",   cx, cy - w);
         fprintf(gc->fp, "%.2lf %.2lf l s\n",   cx, cy);
         break;

      case 4:        /* square */
         fprintf(gc->fp, "n %.2lf %.2lf m\n",   cx - w, cy - w);
         fprintf(gc->fp, "%.2lf %.2lf l\n",     cx + w, cy - w);
         fprintf(gc->fp, "%.2lf %.2lf l\n",     cx + w, cy + w);
         fprintf(gc->fp, "%.2lf %.2lf l p s\n", cx - w, cy + w);
         fprintf(gc->fp, "n %.2lf %.2lf m\n",   cx,     cy - w);
         fprintf(gc->fp, "%.2lf %.2lf l s\n",   cx,     cy);
         break;

      case 5:        /* square, turned 45° */
         fprintf(gc->fp, "n %.2lf %.2lf m\n",   cx,     cy - w);
         fprintf(gc->fp, "%.2lf %.2lf l\n",     cx + w, cy);
         fprintf(gc->fp, "%.2lf %.2lf l\n",     cx,     cy + w);
         fprintf(gc->fp, "%.2lf %.2lf l p s\n", cx - w, cy);
         fprintf(gc->fp, "n %.2lf %.2lf m\n",   cx,     cy - w);
         fprintf(gc->fp, "%.2lf %.2lf l s\n",   cx,     cy);
         break;

      case 6:        /* triangle, tip up */
         fprintf(gc->fp, "n %.2lf %.2lf m\n",   cx - w, cy - w);
         fprintf(gc->fp, "%.2lf %.2lf l\n",     cx + w, cy - w);
         fprintf(gc->fp, "%.2lf %.2lf l p s\n", cx    , cy + w);
         fprintf(gc->fp, "n %.2lf %.2lf m\n",   cx,     cy - w);
         fprintf(gc->fp, "%.2lf %.2lf l s\n",   cx,     cy);
         break;

      case 7:        /* triangle, tip down */
         fprintf(gc->fp, "n %.2lf %.2lf m\n",   cx    , cy - w);
         fprintf(gc->fp, "%.2lf %.2lf l\n",     cx + w, cy + w);
         fprintf(gc->fp, "%.2lf %.2lf l p s\n", cx - w, cy + w);
         fprintf(gc->fp, "n %.2lf %.2lf m\n",   cx,     cy - w);
         fprintf(gc->fp, "%.2lf %.2lf l s\n",   cx,     cy);
         break;

      default:       /* X */
         fprintf(gc->fp, "n %.2lf %.2lf m\n", cx - w, cy - w);
         fprintf(gc->fp, "%.2lf %.2lf l\n",   cx + w, cy + w);
         fprintf(gc->fp, "%.2lf %.2lf m\n",   cx - w, cy + w);
         fprintf(gc->fp, "%.2lf %.2lf l s\n", cx + w, cy - w);
         break;
   }

}

/*
 *******************************************************************************
 */

void CPLT_draw_text_EPS(CPLT_gc_t gc, const float x, const float y,
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

   int anchor_num;

   if (gc == NULL) return;
   anchor_num = _anchor_num_of(anchor);
   if (anchor_num == 0) anchor_num = 1;

   fprintf(gc->fp, "(%s) %.2lf %.2lf %.2f T%d\n",
           text, x, y, angle, anchor_num);

}

/*
 *******************************************************************************
 */

void CPLT_set_fontsize_EPS(CPLT_gc_t gc, const float fontsize) {
   /* Sets current fontsize [pix] for CPLT_draw_text().
    * (Preset: fontsize=12.0) */

   if (gc == NULL) return;

   fprintf(gc->fp,
           "/%s ISOfindfont %d scalefont setfont calc_FH\n",
           fontface, (int)fontsize);

}

/*
 *******************************************************************************
 */

void CPLT_set_color_EPS(CPLT_gc_t gc, float r, float g, float b) {
   /* Sets current color of RGB values [0,1].
    * (Preset: r=0., g=0., b=0., i.e. black) */

   if (gc == NULL) return;

   r = r < 0. ? 0. : r > 1. ? 1. : r;
   g = g < 0. ? 0. : g > 1. ? 1. : g;
   b = b < 0. ? 0. : b > 1. ? 1. : b;
   fprintf(gc->fp, "%.3lf %.3lf %.3lf c\n", r, g, b);

}

/*
 *******************************************************************************
 */

void CPLT_set_linewidth_EPS(CPLT_gc_t gc, const float w) {
   /* Sets current linewidth w [pix]. (Preset: w=1.0) */

   if (gc == NULL) return;

   fprintf(gc->fp, "%.2lf w\n", w);

}

/*
 *******************************************************************************
 */

void CPLT_set_linestyle_EPS(CPLT_gc_t gc, const CPLT_lnstyle_t s) {
   /* Sets current linestyle s [enumeration].
    * (Preset: s=CPLT_SolidLine) */

   if (gc == NULL) return;

   switch (s) {
      case CPLT_SolidLine:
         fprintf(gc->fp, "[] d\n");
         break;
      case CPLT_DashLine:
         fprintf(gc->fp, "[4 2] d\n");
         break;
      case CPLT_DotLine:
         fprintf(gc->fp, "[1 2] d\n");
         break;
      case CPLT_DashDotLine:
         fprintf(gc->fp, "[4 2 1 2] d\n");
         break;
      case CPLT_DashDotDotLine:
         fprintf(gc->fp, "[4 2 1 2 1 2] d\n");
         break;
      default:    /* solid */
         fprintf(gc->fp, "[] d\n");
         break;
   }

}

/*
 *******************************************************************************
 */

void CPLT_finish_graphics_EPS(CPLT_gc_t gc) {
   /* Finishes graphics, closes plotfile, destroys graphics context.
    * here EPS: writes EPS-trailer to and closes plotfile */

   if (gc == NULL) return;

   fprintf(gc->fp, "\nshowpage\n%%%%EOF\n");
   fclose(gc->fp);

   free(gc->dispatch);
   free(gc);

}

/*
 *******************************************************************************
 * internal helper functions
 *******************************************************************************
 */

void _create_poly_EPS(CPLT_gc_t gc, int numpts, CPLT_point_t points[]) {
   /* internal helper func to output path of multiple points */

   int i;

   /* build path from all points */
   fprintf(gc->fp, "%.2lf %.2lf m\n", points[0].x, points[0].y);
   for (i = 1; i < numpts; i++) {
      fprintf(gc->fp, "%.2lf %.2lf l\n", points[i].x, points[i].y);
   }

}

/*
 *******************************************************************************
 * management function to assemble the dispatch table/struct
 * for the generic callers
 *******************************************************************************
 */

CPLT_funcn_t *_get_dispatchFuncs_EPS(void) {

   CPLT_funcn_t *dpt = (CPLT_funcn_t *) malloc(sizeof(*dpt));
   if (dpt == NULL) {
      fprintf(stderr, " *** Not enough memory for dispatch table!\n");
      return NULL;
   }

   dpt->INIT  = &CPLT_init_graphics_EPS;
   dpt->PLINE = &CPLT_draw_polyline_EPS;
   dpt->PGON  = &CPLT_draw_polygon_EPS;
   dpt->PGONF = &CPLT_draw_filledPolygon_EPS;
   dpt->ARC   = &CPLT_draw_arc_EPS;
   dpt->ARCF  = &CPLT_draw_filledArc_EPS;
   dpt->CURVE = &CPLT_draw_curve_EPS;
   dpt->MARK  = &CPLT_draw_marker_EPS;
   dpt->TEXT  = &CPLT_draw_text_EPS;
   dpt->FSIZE = &CPLT_set_fontsize_EPS;
   dpt->COLR  = &CPLT_set_color_EPS;
   dpt->LNWD  = &CPLT_set_linewidth_EPS;
   dpt->LNSTY = &CPLT_set_linestyle_EPS;
   dpt->FINI  = &CPLT_finish_graphics_EPS;

   return dpt;
}

/*
 *******************************************************************************
 */

