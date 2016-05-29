/*
 * ADT: 'CPlotter'
 * CPlotter provides a basic, unified interface to different graphics
 * formats for simple 2D-drawings in C.
 *
 * This backend implements the raster-graphics format PNG, it writes a
 * (true-color) PNG-file (*.png) utilizing the GD-library
 * (at least version 2.0 with FreeType), so it needs to be linked with libgd
 * and the resp. include files have to be available too.
 *
 * Author and Copyright: Dipl.-Ing. Horst-W. Radners, Berlin, 2015-2016
 * License: LGPL 3.0, see http://www.gnu.org/licenses/lgpl-3.0.en.html
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ftw.h>
#include <string.h>
#include <errno.h>

#include <gd.h>

#include "CPLT_intern.h"

/* used for nftw() to limit nb of open file descriptors */
#ifndef USE_FDS
#define USE_FDS 15
#endif

/* list of relevant fonts found installed */
#define MAXFONTS 16
typedef struct {
   char *ttf;
   int prio;
} font_t;
font_t fonts[MAXFONTS];
unsigned int numfonts = 0;

/* graphics context */
struct CPLT_gctx {
   CPLT_funcn_t *dispatch; /* functions dispatch table */
   FILE *fp;               /* filepointer */
   unsigned int pheight;   /* image's height [pix] */
   float curfontsize;      /* current GD-fontsize [pix] */
   CPLT_lnstyle_t curlsty; /* current linestyle [enumeration]*/
   int bgcol;              /* image's background color */
   int curcol;             /* current color/style */
   int colidx;             /* current color-index */
   gdImagePtr img;         /* pointer to GD in-memory image */
};


/* global constants */
/* List of start directories for recursive search for TT-fonts.
 * NOTE: if these dirs are not utilized by libpng/libfreetype2,
 * it makes no sense to list them here! */
static const char *fontdirs[] = {
   "/usr/share/fonts/truetype",
   "/usr/local/share/fonts/truetype",
};

/* FT-fontface (*.ttf) used by string drawing */
static char *fontface = NULL;    /* prelim. */

/* prototypes of internal helper functions */
gdPoint *_create_poly_PNG(CPLT_gc_t gc, int numpts, CPLT_point_t points[]);
gdPoint _rotate_vec_PNG(const float x, const float y, const float angle);
int _is_flat_bezier(CPLT_point_t points[]);
void _subdivide_bezier(CPLT_point_t p[], CPLT_point_t l[], CPLT_point_t r[]);
void _approx_bezier(CPLT_gc_t gc, CPLT_point_t points[]);
void _set_coloredDash(CPLT_gc_t gc, const int colidx,
                      const CPLT_lnstyle_t style);
CPLT_funcn_t *_get_dispatchFuncs_PNG(void);

char *_get_TTfontface(void);
int _examine_directory_tree(const char *dirpath);
int _examine_entry(const char *filepath, const struct stat *info,
                   const int typeflag);
int _cmp_by_prio(const void *f1, const void *f2);

/*
 *******************************************************************************
 * API functions
 *******************************************************************************
 */

CPLT_gc_t CPLT_init_graphics_PNG(const unsigned int pwidth,
                                 const unsigned int pheight,
                                 char *plotfilename) {
   /* Initializes graphics of pwidth x pheight [pix] in graphics file
    * plotfilename, returns graphics-context pointer.
    * here GD/PNG: opens PNG-file, initializes image */

   /* allocate memory for graphics context's data-struct */
   CPLT_gc_t gc = (CPLT_gc_t) malloc(sizeof(*gc));
   if (gc == NULL) {
      fprintf(stderr, " *** Not enough memory for graphics context!\n");
      return NULL;
   }

   /* open PNG-plotfile */
   if ((gc->fp = fopen(plotfilename, "wb")) == NULL) {
      fprintf(stderr,
              " *** Can't open output image '%s'!\n", plotfilename);
      return NULL;
   }

   /* create (empty) in-memory image */
   if ((gc->img = gdImageCreateTrueColor(pwidth, pheight)) == NULL) {
      fprintf(stderr,
              " *** Can't create in-memory image-data!\n");
      return NULL;
   }
   gc->pheight = pheight;     /* save image height for y-inversion */

   /* default colors */
   gc->bgcol = gdImageColorAllocate(gc->img, 255, 255, 255);   /* white */
   gc->colidx = gdImageColorAllocate(gc->img, 0, 0, 0);        /* black */
   gdImageSetAntiAliased(gc->img, gc->colidx);  /* antialiased lines */
   gc->curcol = gdAntiAliased;

   /* fill whole true-color image with white as background */
   gdImageFilledRectangle(gc->img, 0, 0, pwidth, pheight, gc->bgcol);

   /* identify usable TTFont */
   if (!fontface) fontface = _get_TTfontface();
#ifdef DEBUG
   fprintf(stderr, " +++ DEBUG CPLT_PNG: using fontface '%s'\n", fontface);
#endif
   gc->curfontsize = 12.;
   gdImageSetThickness(gc->img, 1);

   /* register dispatch table of our functions for generic callers */
   gc->dispatch = _get_dispatchFuncs_PNG();

   return gc;
}

/*
 *******************************************************************************
 */

void CPLT_draw_polyline_PNG(CPLT_gc_t gc, int numpts,
                            CPLT_point_t points[]) {
   /* Plots line through numpts 2D-points at given x/y-pairs in array points.
    * Draws line with current color and linewidth/style. */

   gdPoint *GDpoints;

   if (gc == NULL) return;
   if (numpts <= 1) return;

   GDpoints = _create_poly_PNG(gc, numpts, points);
   if (GDpoints == NULL) return;

   if (numpts > 2) {
      gdImageOpenPolygon(gc->img, GDpoints, numpts, gc->curcol);
   } else {
      gdImageLine(gc->img,
                  GDpoints[0].x, GDpoints[0].y, GDpoints[1].x, GDpoints[1].y,
                  gc->curcol);
   }
   free(GDpoints);

}

/*
 *******************************************************************************
 */

void CPLT_draw_polygon_PNG(CPLT_gc_t gc, int numpts,
                           CPLT_point_t points[]) {
   /* Plots line through numpts 2D-points at given x/y-pairs in array points.
    * Draws outline of the polygon with current color and linewidth/style.
    * here GD/PNG: Polygon of GD-points */

   gdPoint *GDpoints;

   if (gc == NULL) return;
   if (numpts <= 1) return;

   GDpoints = _create_poly_PNG(gc, numpts, points);
   if (GDpoints == NULL) return;

   gdImagePolygon(gc->img, GDpoints, numpts, gc->curcol);
   free(GDpoints);

}

/*
 *******************************************************************************
 */

void CPLT_draw_filledPolygon_PNG(CPLT_gc_t gc, int numpts,
                                 CPLT_point_t points[]) {
   /* Plots (automatically closed) 2D-polygon with numpts points at given
    * x/y-pairs in array points.
    * Fills + strokes the polygon with current color.
    * here GD/PNG: Filled polygon of GD-points, then stroke outline */

   gdPoint *GDpoints;

   if (gc == NULL) return;
   if (numpts <= 1) return;

   GDpoints = _create_poly_PNG(gc, numpts, points);
   if (GDpoints == NULL) return;

   gdImageFilledPolygon(gc->img, GDpoints, numpts, gc->colidx);
   gdImagePolygon(gc->img, GDpoints, numpts, gdAntiAliased);
   free(GDpoints);

}

/*
 *******************************************************************************
 */

void CPLT_draw_arc_PNG(CPLT_gc_t gc, const float cx, const float cy,
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

   gdImageArc(gc->img, (int)cx, (int)(gc->pheight - cy),
              (int)(2 * radius), (int)(2 * radius),
              (int)(360 - end), (int)(360 - start), gc->curcol);

}

/*
 *******************************************************************************
 */


void CPLT_draw_filledArc_PNG(CPLT_gc_t gc, const float cx, const float cy,
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

   gdImageFilledArc(gc->img, (int)cx, (int)(gc->pheight - cy),
                    (int)(2 * radius), (int)(2 * radius),
                    (int)(360 - end), (int)(360 - start), gc->colidx, gdArc);
   gdImageArc(gc->img, (int)cx, (int)(gc->pheight - cy),
              (int)(2 * radius), (int)(2 * radius),
              (int)(360 - end), (int)(360 - start), gdAntiAliased);

}

/*
 *******************************************************************************
 */

void CPLT_draw_curve_PNG(CPLT_gc_t gc, CPLT_point_t points[]) {
   /* Plots a Bezier curve segment by 4 given 2D-points as x/y-pairs
    * in array points. points[0] is start, points[3] end point of curve,
    * points[1] and points[2] are the Bezier control points.
    * Draws line with current color and linewidth/style. */

   if (gc == NULL) return;

   /* approximate curve segment by sufficiently flat lines,
    * recursively subdividing the curve */
   _approx_bezier(gc, points);

}

/*
 *******************************************************************************
 */

void CPLT_draw_marker_PNG(CPLT_gc_t gc, const float cx, const float cy,
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

   gdPoint GDpoints[4];
   int x, y, w;

   if (gc == NULL) return;

   x = _rnd(cx);
   y = _rnd(gc->pheight - cy);
   w = _rnd(0.5 * wd);
   if (w < 1) return;

   switch (symbol) {
      case 1:        /* + */
         gdImageLine(gc->img, x - w, y, x + w, y, gdAntiAliased);
         gdImageLine(gc->img, x, y - w, x, y + w, gdAntiAliased);
         break;

      case 2:        /* star */
         gdImageLine(gc->img, x - w, y,     x + w, y,     gdAntiAliased);
         gdImageLine(gc->img, x,     y - w, x,     y + w, gdAntiAliased);
         gdImageLine(gc->img, x - w, y - w, x + w, y + w, gdAntiAliased);
         gdImageLine(gc->img, x - w, y + w, x + w, y - w, gdAntiAliased);
         break;

      case 3:        /* circle */
         gdImageArc(gc->img, x, y, (int)wd, (int)wd, 0, 360, gdAntiAliased);
         gdImageLine(gc->img, x, y + w, x, y, gdAntiAliased);
         break;

      case 4:        /* square */
         GDpoints[0].x = x - w;
         GDpoints[0].y = y - w;
         GDpoints[1].x = x + w;
         GDpoints[1].y = y - w;
         GDpoints[2].x = x + w;
         GDpoints[2].y = y + w;
         GDpoints[3].x = x - w;
         GDpoints[3].y = y + w;
         gdImagePolygon(gc->img, GDpoints, 4, gdAntiAliased);
         gdImageLine(gc->img, x, y + w, x, y, gdAntiAliased);
         break;

      case 5:        /* square, turned 45° */
         GDpoints[0].x = x;
         GDpoints[0].y = y + w;
         GDpoints[1].x = x + w;
         GDpoints[1].y = y;
         GDpoints[2].x = x;
         GDpoints[2].y = y - w;
         GDpoints[3].x = x - w;
         GDpoints[3].y = y;
         gdImagePolygon(gc->img, GDpoints, 4, gdAntiAliased);
         gdImageLine(gc->img, x, y + w, x, y, gdAntiAliased);
         break;

      case 6:        /* triangle, tip up */
         GDpoints[0].x = x;
         GDpoints[0].y = y - w;
         GDpoints[1].x = x + w;
         GDpoints[1].y = y + w;
         GDpoints[2].x = x - w;
         GDpoints[2].y = y + w;
         gdImagePolygon(gc->img, GDpoints, 3, gdAntiAliased);
         gdImageLine(gc->img, x, y + w, x, y, gdAntiAliased);
         break;

      case 7:        /* triangle, tip down */
         GDpoints[0].x = x - w;
         GDpoints[0].y = y - w;
         GDpoints[1].x = x + w;
         GDpoints[1].y = y - w;
         GDpoints[2].x = x;
         GDpoints[2].y = y + w;
         gdImagePolygon(gc->img, GDpoints, 3, gdAntiAliased);
         gdImageLine(gc->img, x, y + w, x, y, gdAntiAliased);
         break;

      default:       /* X */
         gdImageLine(gc->img, x - w, y - w, x + w, y + w, gdAntiAliased);
         gdImageLine(gc->img, x - w, y + w, x + w, y - w, gdAntiAliased);
         break;
   }

}

/*
 *******************************************************************************
 */

void CPLT_draw_text_PNG(CPLT_gc_t gc, const float x, const float y,
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

   float dx, dy;
   int i, anchor_num, brect[8], w, h, xp, yp;
   char *err;
   gdPoint p;

   if (gc == NULL) return;
   anchor_num = _anchor_num_of(anchor);
   if (anchor_num == 0) anchor_num = 1;

   /* first obtain enclosing rectangle in brect w/o rendering
    * so that we can anchor the string */
   err = gdImageStringFT(NULL, brect, 0, fontface,
                         gc->curfontsize, 0.0, 0, 0, text);
   if (err) { fprintf(stderr, " *** libgd error: %s\n", err); return; }

   for (i = 1; i < 8; i += 2) brect[i] *= -1;   /* invert y for calc */
   w = brect[2] - brect[0] - 3;  /* 3 is heuristic adjustment */
   h = brect[7] - brect[1];

   switch (anchor_num) {
      case 1 :
         dx = brect[0];
         dy = brect[1];
         break;
      case 2 :
         dx = brect[0] - 0.5 * w;
         dy = brect[1];
         break;
      case 3 :
         dx = brect[0] - w;
         dy = brect[1];
         break;
      case 4 :
         dx = brect[0];
         dy = brect[1] + 0.5 * h;
         break;
      case 5 :
         dx = brect[0] - 0.5 * w;
         dy = brect[1] + 0.5 * h;
         break;
      case 6 :
         dx = brect[0] - w;
         dy = brect[1] + 0.5 * h;
         break;
      case 7 :
         dx = brect[0];
         dy = brect[1] + h;
         break;
      case 8 :
         dx = brect[0] - 0.5 * w;
         dy = brect[1] + h;
         break;
      case 9 :
         dx = brect[0] - w;
         dy = brect[1] + h;
         break;
   }

   p = _rotate_vec_PNG(dx, dy, angle);
   xp =                x + p.x;
   yp = gc->pheight - (y - p.y);

   /* now render the anchored, rotated string */
   err = gdImageStringFT(gc->img, brect, gc->colidx, fontface,
                         gc->curfontsize, angle * DEG2RAD, xp, yp, text);
   if (err) fprintf(stderr, " *** libgd error: %s\n", err);

}

/*
 *******************************************************************************
 */

void CPLT_set_fontsize_PNG(CPLT_gc_t gc, const float fontsize) {
   /* Sets current fontsize [pix] for CPLT_draw_text().
    * (Preset: fontsize=12.0) */

   if (gc == NULL) return;

   gc->curfontsize  = fontsize;

}

/*
 *******************************************************************************
 */

void CPLT_set_color_PNG(CPLT_gc_t gc, float r, float g, float b) {
   /* Sets current color of RGB values [0,1].
    * (Preset: r=0., g=0., b=0., i.e. black)
    * here GD/PNG: maybe combined with current line dash pattern */

   if (gc == NULL) return;

   r = r < 0. ? 0. : r > 1. ? 1. : r;
   g = g < 0. ? 0. : g > 1. ? 1. : g;
   b = b < 0. ? 0. : b > 1. ? 1. : b;
   gc->colidx = gdImageColorResolve(gc->img,
                                    (int)(255 * r),
                                    (int)(255 * g),
                                    (int)(255 * b));
   gdImageSetAntiAliased(gc->img, gc->colidx);  /* antialiased lines */

   if (gc->curlsty == CPLT_SolidLine) {
      gc->curcol = gdAntiAliased;
   } else {
      gc->curcol = gdStyled;
      _set_coloredDash(gc, gdAntiAliased, gc->curlsty);
   }

}

/*
 *******************************************************************************
 */

void CPLT_set_linewidth_PNG(CPLT_gc_t gc, const float w) {
   /* Sets current linewidth w [pix]. (Preset: w=1.0) */

   int p;

   if (gc == NULL) return;

   p = _rnd(w);
   gdImageSetThickness(gc->img, (p < 1 ? 1 : p));

}

/*
 *******************************************************************************
 */

void CPLT_set_linestyle_PNG(CPLT_gc_t gc, const CPLT_lnstyle_t s) {
   /* Sets current linestyle s [enumeration].
    * (Preset: s=CPLT_SolidLine)
    * here GD/PNG: maybe combined with current color */

   if (gc == NULL) return;

   gc->curlsty = s;
   if (gc->curlsty == CPLT_SolidLine) {
      gc->curcol = gdAntiAliased;
   } else {
      gc->curcol = gdStyled;
      _set_coloredDash(gc, gdAntiAliased, gc->curlsty);
   }

}

/*
 *******************************************************************************
 */

void CPLT_finish_graphics_PNG(CPLT_gc_t gc) {
   /* Finishes graphics, closes plotfile, destroys graphics context.
    * here GD/PNG: writes PNG-image to and closes imgfile */

   if (gc == NULL) return;

   /* convert internal image to PNG and write it to file */
   gdImagePng(gc->img, gc->fp);

   /* close imgfile, free in-memory image data */
   fclose(gc->fp);
   gdImageDestroy(gc->img);

   free(gc->dispatch);
   free(gc);
}

/*
 *******************************************************************************
 * internal helper functions
 *******************************************************************************
 */

char *_get_TTfontface(void) {
   /* Internal helper func to get a TrueType-fontface
    * from installed font files */

   int i, ND;
   char *fc = "unknown";

   ND = sizeof(fontdirs) / sizeof(fontdirs[0]);
   for (i = 0; i < ND; i++) {
      if (_examine_directory_tree(fontdirs[i])) {
         fprintf(stderr, " *** CPlotter: %s.\n", strerror(errno));
      }
   }
   if (numfonts > 0) {
      fc = fonts[0].ttf;
   } else {
      fprintf(stderr, " *** CPlotter: No usable TT-fonts found, "
            "text drawing not available!\n");
   }

   return fc;
}

/*
 *******************************************************************************
 */

int _examine_directory_tree(const char *dirpath) {
   /* recursively walk down dir tree to find usable font files */

   int i, p, result, nprevf;
   char *fname;

   /* Invalid directory path? */
   if (dirpath == NULL || *dirpath == '\0')
      return errno = EINVAL;

   /* recursive decent */
   nprevf = numfonts;
   result = ftw(dirpath, _examine_entry, USE_FDS);
   if (result > 0)
      return errno = result;
#ifdef DEBUG
   fprintf(stderr,
         " +++ DEBUG CPLT_PNG: %d usable TT-fonts found under dir\n",
         numfonts-nprevf);
   fprintf(stderr, " +++    '%s'\n",dirpath);
#endif

   /* No results at all? */
   if (numfonts == 0)
      return errno = ENOENT;

   /* massage results: we need path part below parent dirpath,
    * sorted by priority, so first is preferably used */
   p = strlen(dirpath);
   if (dirpath[p - 1] != '/') p++;
   for (i = nprevf; i < numfonts; i++) {
      fname = strdup(&fonts[i].ttf[p]);
      fname[strlen(fname) - 4] = '\0'; /* delete '.ttf'-suffix */
      free(fonts[i].ttf);
      fonts[i].ttf = fname;
   }
   qsort(fonts, numfonts, sizeof(fonts[0]), _cmp_by_prio);

#ifdef DEBUG
   fprintf(stderr,
         " +++ DEBUG CPLT_PNG: List of usable TT-fonts found so far:\n");
   for (i = 0; i < numfonts; i++)
      fprintf(stderr, " +++    %2d: (%d) %s\n",
              i + 1, fonts[i].prio, fonts[i].ttf);
#endif

   return 0;
}

/*
 *******************************************************************************
 */

int _examine_entry(const char *filepath, const struct stat *info,
                   const int typeflag) {
   /* examine dir entry, save Sans-TTFs found */

   int l;
   char *fn, *sfx;  /* lower-cased filename and -suffix */

   if (numfonts >= MAXFONTS) return 0;
   if (typeflag != FTW_F) return 0;
   l = strlen(filepath);
   if (l <= 4) return 0;
   sfx = _extract_lowered_suffix(filepath);
   if (!sfx) return 0;
   if (strcmp(sfx, "ttf") != 0) return 0;
   free(sfx);

   /* gather relevant fontnames with priority */
   fn = _str_lowered_dup(filepath);
   if (!fn) return 0;
   if (strstr(fn, "verdana.ttf")) {
      fonts[numfonts].ttf = strdup(filepath);
      fonts[numfonts++].prio = 1;
   } else if (strstr(fn, "sans.ttf")) {
      fonts[numfonts].ttf = strdup(filepath);
      fonts[numfonts++].prio = 2;
   } else if (strstr(fn, "sans-regular.ttf")) {
      fonts[numfonts].ttf = strdup(filepath);
      fonts[numfonts++].prio = 3;
   } else if (strstr(fn, "sanscondensed.ttf")) {
      fonts[numfonts].ttf = strdup(filepath);
      fonts[numfonts++].prio = 4;
   } else if (strstr(fn, "cour.ttf")) {     /* fallback only */
      fonts[numfonts].ttf = strdup(filepath);
      fonts[numfonts++].prio = 9;
   } else if (strstr(fn, "courier.ttf")) {  /* fallback only */
      fonts[numfonts].ttf = strdup(filepath);
      fonts[numfonts++].prio = 9;
   }
   free(fn);

   return 0;
}

/*
 *******************************************************************************
 */

int _cmp_by_prio(const void *f1, const void *f2) {
   /* compare font entries by 1.) priority, 2.) name */
   int p1, p2;
   p1 = ((font_t *)f1)->prio;
   p2 = ((font_t *)f2)->prio;
   if (p1 == p2) return strcmp(((font_t *)f1)->ttf, ((font_t *)f2)->ttf);
   return p1 - p2;
}

/*
 *******************************************************************************
 */

gdPoint *_create_poly_PNG(CPLT_gc_t gc, int numpts, CPLT_point_t points[]) {
   /* internal helper func to assemble gdPoint array of polygon-points
    * from array points.
    * caller must free() returned array! */

   int i;
   gdPoint *GDpoints = (gdPoint *) malloc(numpts * sizeof(gdPoint));
   if (GDpoints == NULL) {
      fprintf(stderr, " *** Not enough memory for polygon points!\n");
      return NULL;
   }

   /* process numpts points */
   for (i = 0; i < numpts; i++) {
      GDpoints[i].x = _rnd(points[i].x);
      GDpoints[i].y = _rnd(gc->pheight - points[i].y);
   }

   return GDpoints;
}

/*
 *******************************************************************************
 */

gdPoint _rotate_vec_PNG(const float x, const float y, const float angle) {
   /* Internal helper func to rotate a vector given by its x/y-components
    * by angle [deg], returns new vector as a gdPoint struct */

   gdPoint r;
   float c = cos(DEG2RAD * angle);
   float s = sin(DEG2RAD * angle);

   r.x = _rnd(x * c + y * s);
   r.y = _rnd(-x * s + y * c);

   return r;
}

/*
 *******************************************************************************
 */

int _is_flat_bezier(CPLT_point_t points[]) {
   /* Internal helper func to decide if a cubic Bezier curve segment
    * given by 4 control points is flat "enough".
    * Simplyfied Manhattan distance here to avoid sqrt. */

   const static float dist_tol = 1.;

   return (fabs(points[0].x + points[2].x - points[1].x - points[1].x) +
           fabs(points[0].y + points[2].y - points[1].y - points[1].y) +
           fabs(points[1].x + points[3].x - points[2].x - points[2].x) +
           fabs(points[1].y + points[3].y - points[2].y - points[2].y)
           <= dist_tol);
}

/*
 *******************************************************************************
 */

void _subdivide_bezier(CPLT_point_t p[], CPLT_point_t l[], CPLT_point_t r[]) {
   /* Internal helper func for subdividing a cubic Bezier curve segment
    * given by 4 control points in a left and right part
    * (de Casteljau construction for order 3 with t=1/2) */

   CPLT_point_t m;

   m = _midpoint(p[1], p[2]);

   l[0] = p[0];
   r[3] = p[3];
   l[1] = _midpoint(p[0], p[1]);
   r[2] = _midpoint(p[2], p[3]);
   l[2] = _midpoint(l[1], m);
   r[1] = _midpoint(m, r[2]);
   l[3] = _midpoint(l[2], r[1]);
   r[0] = l[3];
}

/*
 *******************************************************************************
 */

void _approx_bezier(CPLT_gc_t gc, CPLT_point_t points[]) {
   /* Internal recursive helper func for linear approximation ("flattening")
    * of a cubic Bezier curve segment given by 4 control points */

   CPLT_point_t l[4], r[4];   /* left/right part of bezier segment */

   if (_is_flat_bezier(points)) {

      /* draw as straight line between endpoints */
      l[0] = points[0];
      l[1] = points[3];
      CPLT_draw_polyline_PNG(gc, 2, l);

   } else {

      /* recursively subdivide further */
      _subdivide_bezier(points, l, r);
      _approx_bezier(gc, l);
      _approx_bezier(gc, r);
   }
}

/*
 *******************************************************************************
 */

void _set_coloredDash(CPLT_gc_t gc, const int colidx,
                      const CPLT_lnstyle_t style) {
   /* Internal helper func to set combined linestyle
    * of color and dash pattern */

   int pat[12];    /* bit pattern for style */

   switch (style) {
      case CPLT_DashLine:
         pat[0] = colidx;
         pat[1] = colidx;
         pat[2] = colidx;
         pat[3] = colidx;
         pat[4] = gdTransparent;
         pat[5] = gdTransparent;
         gdImageSetStyle(gc->img, pat, 6);
         break;
      case CPLT_DotLine:
         pat[0] = colidx;
         pat[1] = gdTransparent;
         pat[2] = gdTransparent;
         gdImageSetStyle(gc->img, pat, 3);
         break;
      case CPLT_DashDotLine:
         pat[0] = colidx;
         pat[1] = colidx;
         pat[2] = colidx;
         pat[3] = colidx;
         pat[4] = gdTransparent;
         pat[5] = gdTransparent;
         pat[6] = colidx;
         pat[7] = gdTransparent;
         pat[8] = gdTransparent;
         gdImageSetStyle(gc->img, pat, 9);
         break;
      case CPLT_DashDotDotLine:
         pat[0] = colidx;
         pat[1] = colidx;
         pat[2] = colidx;
         pat[3] = colidx;
         pat[4] = gdTransparent;
         pat[5] = gdTransparent;
         pat[6] = colidx;
         pat[7] = gdTransparent;
         pat[8] = gdTransparent;
         pat[9] = colidx;
         pat[10] = gdTransparent;
         pat[11] = gdTransparent;
         gdImageSetStyle(gc->img, pat, 12);
         break;
      default:    /* solid */
         pat[0] = colidx;
         gdImageSetStyle(gc->img, pat, 1);
         break;
   }

}

/*
 *******************************************************************************
 * management function to assemble the dispatch table/struct
 * for the generic callers
 *******************************************************************************
 */

CPLT_funcn_t *_get_dispatchFuncs_PNG(void) {

   CPLT_funcn_t *dpt = (CPLT_funcn_t *) malloc(sizeof(*dpt));
   if (dpt == NULL) {
      fprintf(stderr, " *** Not enough memory for dispatch table!\n");
      return NULL;
   }

   dpt->INIT  = &CPLT_init_graphics_PNG;
   dpt->PLINE = &CPLT_draw_polyline_PNG;
   dpt->PGON  = &CPLT_draw_polygon_PNG;
   dpt->PGONF = &CPLT_draw_filledPolygon_PNG;
   dpt->ARC   = &CPLT_draw_arc_PNG;
   dpt->ARCF  = &CPLT_draw_filledArc_PNG;
   dpt->CURVE = &CPLT_draw_curve_PNG;
   dpt->MARK  = &CPLT_draw_marker_PNG;
   dpt->TEXT  = &CPLT_draw_text_PNG;
   dpt->FSIZE = &CPLT_set_fontsize_PNG;
   dpt->COLR  = &CPLT_set_color_PNG;
   dpt->LNWD  = &CPLT_set_linewidth_PNG;
   dpt->LNSTY = &CPLT_set_linestyle_PNG;
   dpt->FINI  = &CPLT_finish_graphics_PNG;

   return dpt;
}

/*
 *******************************************************************************
 */


