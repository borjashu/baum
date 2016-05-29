/*
 * ADT: 'CPlotter'
 * CPlotter provides a basic, _unified_ interface to different graphics
 * formats for simple 2D-drawings in C.
 *
 * This is the module implementing the generic API-functions, they
 * merely dispatch the calls to the format-specific backend choosen
 * at runtime, hence it loosely follows the STRATEGY design pattern.
 *
 * Author and Copyright: Dipl.-Ing. Horst-W. Radners, Berlin, 2015-2016
 * License: LGPL 3.0, see http://www.gnu.org/licenses/lgpl-3.0.en.html
 */

#include "CPLT_intern.h"


/* Array of implemented backends/graphics formats, defining name,
 * expected suffix and pointer to format-specific init function.
 * Prototypes of init functions needed here to avoid separate headers. */
INIT_ft CPLT_init_graphics_EPS;
INIT_ft CPLT_init_graphics_PNG;
INIT_ft CPLT_init_graphics_SVG;
struct {
   char *name;
   char *suffix;
   INIT_ft *initFunc;
} GFORMAT[] = {
   {
      "Encapsulated Postscript vector graphics (PS-Adobe-3.0 EPSF-3.0)",
      "eps",
      &CPLT_init_graphics_EPS
   },
   {
      "Portable Network Graphics, true-color raster image (PNG 1.2)",
      "png",
      &CPLT_init_graphics_PNG
   },
   {
      "Scalable Vector Graphics (SVG 1.1)",
      "svg",
      &CPLT_init_graphics_SVG
   },
};


/* (minimal) graphics context (common part of all backends), for dispatching */
struct CPLT_gctx {
   CPLT_funcn_t *dispatch; /* functions dispatch table */
   FILE *fp;               /* filepointer to plotfile */
};

/*
 *******************************************************************************
 * generic API functions
 *******************************************************************************
 */

CPLT_gc_t CPLT_init_graphics(const unsigned int pwidth,
                             const unsigned int pheight,
                             char *plotfilename) {
   /* Initializes graphics of pwidth x pheight [pix],
    * returns graphics-context pointer.
    * The graphics-format specific suffix (.eps, .svg, .png)
    * must be included and determines the graphics format/backend used. */

   int i, l, NGF, GFMT_IDX = -1;
   char *sfx;            /* file-suffix from plotfilename */

   /* graphics format requested */
   NGF = sizeof(GFORMAT) / sizeof(GFORMAT[0]);
   l = strlen(plotfilename);
   sfx = _extract_lowered_suffix(plotfilename);
   if (l > 3 && sfx) {
      for (i = 0; i < NGF; i++) {
         if (strcmp(GFORMAT[i].suffix, sfx) == 0) {
            GFMT_IDX = i;
            break;
         }
      }
   }
   if (GFMT_IDX < 0) {   /* requested suffix/format not implemented */
      fprintf(stderr,
              "\n *** ERR: The graphics format requested by suffix '%s'\n"
              " *** (from plotfilename '%s')\n"
              " *** is not implemented! Known suffixes are:\n",
              (sfx ? sfx : ""), plotfilename);
      for (i = 0; i < NGF; i++)
         fprintf(stderr, " ***    %s: %s\n",
                 GFORMAT[i].suffix, GFORMAT[i].name);
      return NULL;
   }
   if (sfx) free(sfx);

   /* propagate this generic function call to format specific one,
    * moreover this fills the dispatch table in the gc for calling
    * all further format specific functions */
   return (*(GFORMAT[GFMT_IDX].initFunc))(pwidth, pheight, plotfilename);
}

/*
 *******************************************************************************
 */

void CPLT_draw_polyline(CPLT_gc_t gc, int numpts, CPLT_point_t points[]) {
   /* Plots line through numpts 2D-points at given x/y-pairs in array points.
    * Draws line with current color and linewidth/style. */

   /* propagate this generic function call to format specific one */
   (*(gc->dispatch->PLINE))(gc, numpts, points);

}

/*
 *******************************************************************************
 */

void CPLT_draw_polygon(CPLT_gc_t gc, int numpts, CPLT_point_t points[]) {
   /* Plots (automatically closed) 2D-polygon with numpts points at given
    * x/y-pairs in array points.
    * Draws outline of the polygon with current color and linewidth/style. */

   /* propagate this generic function call to format specific one */
   (*(gc->dispatch->PGON))(gc, numpts, points);
}

/*
 *******************************************************************************
 */

void CPLT_draw_filledPolygon(CPLT_gc_t gc, int numpts, CPLT_point_t points[]) {
   /* Plots (automatically closed) 2D-polygon with numpts points at given
    * x/y-pairs in array points.
    * Fills and strokes the polygon with current color. */

   /* propagate this generic function call to format specific one */
   (*(gc->dispatch->PGONF))(gc, numpts, points);
}

/*
 *******************************************************************************
 */

void CPLT_draw_arc(CPLT_gc_t gc, const float cx, const float cy,
                   const float radius,
                   const float start, const float end) {
   /* Plots partial circle at given x/y-center with the specified radius.
    * The arc begins at the position in degrees specified by angle start
    * and ends at the position specified by angle end.
    * A full circle can be drawn by beginning from start=0 degrees and
    * ending at end=360 degrees.
    * Both angles turn counterclockwise, i.e. mathematically positive.
    * Draws outline of the arc with current color and linewidth/style. */

   /* propagate this generic function call to format specific one */
   (*(gc->dispatch->ARC))(gc, cx, cy, radius, start, end);
}

/*
 *******************************************************************************
 */

void CPLT_draw_filledArc(CPLT_gc_t gc, const float cx, const float cy,
                         const float radius,
                         const float start, const float end) {
   /* Plots partial circle at given x/y-center with the specified radius.
    * The arc begins at the position in degrees specified by angle start
    * and ends at the position specified by angle end.
    * A full circle can be drawn by beginning from start=0 degrees and
    * ending at end=360 degrees.
    * Both angles turn counterclockwise, i.e. mathematically positive.
    * Fills and strokes the arc/"pie slice" with current color. */

   /* propagate this generic function call to format specific one */
   (*(gc->dispatch->ARCF))(gc, cx, cy, radius, start, end);
}

/*
 *******************************************************************************
 */

void CPLT_draw_curve(CPLT_gc_t gc, CPLT_point_t points[]) {
   /* Plots a Bezier curve segment by 4 given 2D-points as x/y-pairs
    * in array points. points[0] is start, points[3] end point of curve,
    * points[1] and points[2] are the Bezier control points.
    * Draws line with current color and linewidth/style. */

   /* propagate this generic function call to format specific one */
   (*(gc->dispatch->CURVE))(gc, points);
}

/*
 *******************************************************************************
 */

void CPLT_draw_marker(CPLT_gc_t gc, const float cx, const float cy,
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

   /* propagate this generic function call to format specific one */
   (*(gc->dispatch->MARK))(gc, cx, cy, wd, symbol);
}

/*
 *******************************************************************************
 */

void CPLT_draw_text(CPLT_gc_t gc, const float x, const float y,
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

   /* propagate this generic function call to format specific one */
   (*(gc->dispatch->TEXT))(gc, x, y, anchor, angle, text);
}

/*
 *******************************************************************************
 */

void CPLT_set_fontsize(CPLT_gc_t gc, const float fontsize) {
   /* Sets current fontsize [pix] for CPLT_draw_text().
    * (Preset: fontsize=12.0) */

   /* propagate this generic function call to format specific one */
   (*(gc->dispatch->FSIZE))(gc, fontsize);
}

/*
 *******************************************************************************
 */

void CPLT_set_color(CPLT_gc_t gc, float r, float g, float b) {
   /* Sets current color of RGB values [0,1].
    * (Preset: r=0., g=0., b=0., i.e. black) */

   /* propagate this generic function call to format specific one */
   (*(gc->dispatch->COLR))(gc, r, g, b);
}

/*
 *******************************************************************************
 */

void CPLT_set_linewidth(CPLT_gc_t gc, const float w) {
   /* Sets current linewidth w [pix]. (Preset: w=1.0) */

   /* propagate this generic function call to format specific one */
   (*(gc->dispatch->LNWD))(gc, w);
}

/*
 *******************************************************************************
 */

void CPLT_set_linestyle(CPLT_gc_t gc, const CPLT_lnstyle_t s) {
   /* Sets current linestyle s [enumeration].
    * (Preset: s=CPLT_SolidLine) */

   /* propagate this generic function call to format specific one */
   (*(gc->dispatch->LNSTY))(gc, s);
}

/*
 *******************************************************************************
 */

void CPLT_finish_graphics(CPLT_gc_t gc) {
   /* Finishes graphics, closes plotfile, destroys graphics context */

   /* propagate this generic function call to format specific one */
   (*(gc->dispatch->FINI))(gc);
}

/*
 *******************************************************************************
 */



