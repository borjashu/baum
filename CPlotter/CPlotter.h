#ifndef _CPLOTTER_H_
#define _CPLOTTER_H_

/*
 * Header-file of 'CPlotter' ADT, i.e. the public interface/API.
 *
 * The CPlotter library provides a basic, _unified_ interface to different
 * graphics formats for simple (passive, file-based) 2D-drawings in C.
 *
 * === Graphics File Formats ===
 *
 * Each of the different backends/implementations generates a file of
 * one distinct graphics format, currently are implemented:
 * -------+----------------------------------------------------------------
 *  file- |                 graphics
 * suffix |                  format
 * -------+----------------------------------------------------------------
 *   eps  | Encapsulated Postscript vector graphics (PS-Adobe-3.0 EPSF-3.0)
 *   png  | Portable Network Graphics, true-color raster image (PNG 1.2)
 *   svg  | Scalable Vector Graphics (SVG 1.1)
 * -------+----------------------------------------------------------------
 *
 * The graphics format to use is determined by the file-suffix given in
 * the plotfilename to CPLT_init_graphics(), see below.
 *
 * Please note, since CPlotter utilizes the GD-library (see
 * https://libgd.github.io/) to create PNG files, it has to be linked with
 * libgd (at least version 2.0 with FreeType) and the resp. include files
 * have to be available too if re-compiling is desired (on most Linux
 * distributions this requires installation of 'libgd' and 'libgd-devel'
 * or similar).
 *
 * The EPS and SVG formats are stand-alone text files, their generation by
 * CPlotter is self-contained, hence independent of any external libraries.
 * As vector-graphics, EPS and SVG formats are resolution-independent, i.e.
 * they may be scaled without loss of quality. Especially EPS is recommended
 * for including generated graphics in other (word processing) documents.
 * 
 * === Coordinate System ===
 *
 * The unified coordinate system used by CPlotter has (0,0) at its lower
 * left, x increases to the right, y increases upwards.
 * All coords are in native units of the resp. backend format (pix, pt).
 * Although CPlotter uses float for the coords, they may be rounded to
 * int dependent on the backend format.
 *
 * === Graphics Attributes ===
 *
 * All attributes (color, linewidth, linestyle, fontsize) of the drawing
 * items are persistent, i.e. they keep their values until reset by the
 * respective CPLT_set_*() function (again).
 *
 * === Abstract Data Type ===
 *
 * Since the graphics context is stored in a client-variable and all
 * backend functions are reentrant, CPlotter serves as a first-class ADT,
 * hence multiple instances may coexist. Therefore a client programm may
 * hold open various plots (of may be different graphics formats) at the
 * same time. The client must not exploit any knowledge about the internal
 * structure of the graphics context, but interact with CPlotter solely by
 * calls to the interface/API functions declared below.
 *
 * === Example ===
 *
 * As a minimal usage example, this generates a SVG graphics file
 * 'myplotfile.svg' which can be viewed with any modern browser:
 * ----------------------------------------------------------------------------
 *   #include <stdio.h>
 *   #include <stdlib.h>
 *   #include <math.h>
 *
 *   #include "CPlotter.h"
 *
 *   int main() {
 *
 *      // plot area width, height and margin [pix]
 *      enum { pwd = 200, pht = 100, mrg = 10 };
 *      const float RAD2DEG = 57.29578;    // factor: radiants ==> degrees
 *
 *      CPLT_gc_t gc;                // stores graphics context
 *      CPLT_point_t points[] = {    // rectangle coords inside margin
 *         {mrg, mrg}, {pwd - mrg, mrg},
 *         {pwd - mrg, pht - mrg}, {mrg, pht - mrg} };
 *
 *      // initialize graphics context, open plotfile
 *      if ((gc = CPLT_init_graphics(pwd, pht, "myplotfile.svg")) == NULL) {
 *         fprintf(stderr, " *** ERR: Can't initialize graphics context!\n");
 *         return 1;
 *      }
 *
 *      CPLT_set_color(gc, 1., 1., 0.7);         // fill with light-yellow
 *      CPLT_draw_filledPolygon(gc, 4, points);  // draw a colored rectangle
 *
 *      CPLT_set_linewidth(gc, 3.);              // thicker ...
 *      CPLT_set_color(gc, 0., 0., 0.);          // black ...
 *      CPLT_draw_polygon(gc, 4, points);        // border
 *
 *      // draw centered text rotated to diagonal
 *      CPLT_draw_text(gc, 0.5 * pwd, 0.5 * pht, "c", atan2f(pht - 2 * mrg,
 *                     pwd - 2 * mrg) * RAD2DEG, "Hello world!");
 *
 *      // finish graphics, close plotfile, destroy graphics context
 *      CPLT_finish_graphics(gc);
 *
 *      return 0;
 *   }
 *
 * ----------------------------------------------------------------------------
 *
 * Author and Copyright: Dipl.-Ing. Horst-W. Radners, Berlin, 2015-2016
 * License: LGPL 3.0, see http://www.gnu.org/licenses/lgpl-3.0.en.html
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

/************************************************************************/

#define CPLT_VERSION "1.5"

/* === Type definitions === */

/* CPLT_gc_t is the type clients use as graphics context,
 * i.e. the ADT object */
typedef struct CPLT_gctx *CPLT_gc_t;

/* The type for 2D-points with x/y-coords used for array-parameters */
typedef struct { float x, y; } CPLT_point_t;

/* Enumerated dash patterns for linestyle, see CPLT_set_linestyle() */
typedef enum {
   CPLT_SolidLine,
   CPLT_DashLine,
   CPLT_DotLine,
   CPLT_DashDotLine,
   CPLT_DashDotDotLine
} CPLT_lnstyle_t;

/************************************************************************/

/* === The 14 functions constituting the ADT ===
 *
 * Each other function needs as its first parameter the graphics
 * context pointer returned by CPLT_init_graphics(). */


CPLT_gc_t CPLT_init_graphics(const unsigned int pwidth,
                             const unsigned int pheight,
                             char *plotfilename);
/* Initializes graphics of pwidth x pheight [pix] in graphics file
 * plotfilename, the graphics-format specific suffix (.eps, .svg, .png)
 * must be included and determines the graphics format/backend used.
 * Returns graphics context pointer. */


void CPLT_draw_polyline(CPLT_gc_t gc, const int numpts,
                        CPLT_point_t points[]);
/* Plots line through numpts 2D-points at given x/y-pairs in array points.
 * Draws line with current color and linewidth/style. */


void CPLT_draw_polygon(CPLT_gc_t gc, const int numpts,
                       CPLT_point_t points[]);
/* Plots (automatically closed) 2D-polygon with numpts points at given
 * x/y-pairs in array points.
 * Draws outline of the polygon with current color and linewidth/style. */


void CPLT_draw_filledPolygon(CPLT_gc_t gc, const int numpts,
                             CPLT_point_t points[]);
/* Plots (automatically closed) 2D-polygon with numpts points at given
 * x/y-pairs in array points.
 * Fills and strokes the polygon with current color. */


void CPLT_draw_arc(CPLT_gc_t gc, const float cx, const float cy,
                   const float radius,
                   const float start, const float end);
/* Plots partial circle at given x/y-center with the specified radius.
 * The arc begins at the position in degrees specified by angle start
 * and ends at the position specified by angle end.
 * A full circle can be drawn by beginning from start=0 degrees and
 * ending at end=360 degrees.
 * Both angles turn counterclockwise, i.e. mathematically positive.
 * Draws outline of the arc with current color and linewidth/style. */


void CPLT_draw_filledArc(CPLT_gc_t gc, const float cx, const float cy,
                         const float radius,
                         const float start, const float end);
/* Plots partial circle at given x/y-center with the specified radius.
 * The arc begins at the position in degrees specified by angle start
 * and ends at the position specified by angle end.
 * A full circle can be drawn by beginning from start=0 degrees and
 * ending at end=360 degrees.
 * Both angles turn counterclockwise, i.e. mathematically positive.
 * Fills and strokes the arc/"pie slice" with current color. */


void CPLT_draw_curve(CPLT_gc_t gc, CPLT_point_t points[]);
/* Plots a Bezier curve segment by 4 given 2D-points as x/y-pairs
 * in array points. points[0] is start, points[3] end point of curve,
 * points[1] and points[2] are the Bezier control points.
 * Draws line with current color and linewidth/style. */


void CPLT_draw_marker(CPLT_gc_t gc, const float cx, const float cy,
                      const int wd, const int symbol);
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


void CPLT_draw_text(CPLT_gc_t gc, const float x, const float y,
                    char *anchor, float angle, char *text);
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


void CPLT_set_fontsize(CPLT_gc_t gc, const float fontsize);
/* Sets current fontsize [pix] for CPLT_draw_text().
 * (Preset: fontsize=12.0) */


void CPLT_set_color(CPLT_gc_t gc, float r, float g, float b);
/* Sets current color of RGB values [0,1].
 * (Preset: r=0., g=0., b=0., i.e. black) */


void CPLT_set_linewidth(CPLT_gc_t gc, const float w);
/* Sets current linewidth w [pix]. (Preset: w=1.0) */


void CPLT_set_linestyle(CPLT_gc_t gc, const CPLT_lnstyle_t s);
/* Sets current linestyle s [enumeration].
 * (Preset: s=CPLT_SolidLine) */


void CPLT_finish_graphics(CPLT_gc_t gc);
/* Finishes graphics, closes plotfile, destroys graphics context */

#endif

