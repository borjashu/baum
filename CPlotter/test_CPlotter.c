/***********************************************************************
 * Test/Demo of CPlotter graphics API:
 * Plot some figures to image file.
 *
 * Autor: Horst-W. Radners
 ***********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "CPlotter.h"

#define DEG2RAD  0.017453292519943

int main(int argc, char *argv[]) {

   enum {
      PLTWIDTH  = 500,   /* size [pix] */
      PLTHEIGHT = 750    /* of plot area */
   };

   int i;
   float x, dx, y, dy, yp, ofs, r, c, s, e;
   CPLT_gc_t gc;
   CPLT_point_t pts[8];
   char ptxt[40];    /* buffer for text plotting */
   char *anchors[] = { "", "sw", "s", "se", "w", "c", "e", "nw", "n", "ne" };
   char *suffix, *plotfilename, *plotfilebase = "testgraphics.";

   /* parse options */
   for (i = 1; i < argc && argv[i][0] == '-'; i++) {
      switch (argv[i][1]) {
         case 'v':
            fprintf(stderr, "%s v%s\n", argv[0], CPLT_VERSION);
            return 1;
         case 'h':
         /* fall -through */
         default:
            fprintf(stderr, "Usage: %s [-hv] [suffix]\n", argv[0]);
            fprintf(stderr,
                    "       -h: print this help text\n"
                    "       -v: print version of CPlotter lib\n"
                    "   suffix: of plotfilename, i.e. requested\n"
                    "           graphics-format (eps [default], png, svg)\n");
            return 1;
      }
   }

   /* remaining non-option argument is plotfile-suffix/graphics-format */
   if (i <= argc - 1) {
      suffix = strdup(argv[i]);
   } else {
      suffix = "eps";
   }
   if ((plotfilename = (char *)malloc(strlen(plotfilebase) + strlen(suffix) + 1))
         == NULL) {
      fprintf(stderr, "\n *** No more memory, abort!\n\n");
      return 1;
   }
   strcpy(plotfilename, plotfilebase);
   strcat(plotfilename, suffix);

   /* initialize graphics context, open plotfile */
   if ((gc = CPLT_init_graphics(PLTWIDTH, PLTHEIGHT, plotfilename)) == NULL) {
      fprintf(stderr, "\n *** Can't initialize graphics context, abort!\n\n");
      return 1;
   }
   printf("Testing CPlotter ...\n");

   /*
    * title, blue box border
    */
   CPLT_set_fontsize(gc, 16);
   sprintf(ptxt, "CPlotter v%s Demo", CPLT_VERSION);
   CPLT_draw_text(gc, 0.5 * PLTWIDTH, PLTHEIGHT - 30., "s", 0., ptxt);
   CPLT_set_fontsize(gc, 12);
   CPLT_set_color(gc, 0., 0., 1.);
   CPLT_set_linewidth(gc, 2);
   pts[0].x = 1.;
   pts[0].y = 1.;
   pts[1].x = PLTWIDTH - 1.;
   pts[1].y = 1.;
   pts[2].x = PLTWIDTH - 1.;
   pts[2].y = PLTHEIGHT - 1.;
   pts[3].x = 1.;
   pts[3].y = PLTHEIGHT - 1.;
   CPLT_draw_polygon(gc, 4, pts);
   CPLT_set_color(gc, 0., 0., 0.);


   /*
    * markers
    */
   yp = PLTHEIGHT - 80;
   CPLT_set_linewidth(gc, 1);
   CPLT_draw_text(gc, 20., yp, "w", 0., "Markers:");
   for (i = 0; i <= 7; i++) {
      CPLT_draw_marker(gc, 120 + i * 20, yp, 8., i);
   }
   CPLT_set_color(gc, 1., 0., 0.);
   CPLT_set_linewidth(gc, 2);
   for (i = 0; i <= 7; i++) {
      CPLT_draw_marker(gc, 310 + i * 20, yp, 12., i);
   }
   CPLT_set_color(gc, 0., 0., 0.);


   /*
    * polylines
    */
   yp -= 70;
   x = 120.;
   dx = 40;
   dy = 50;
   y = yp - dy / 2;
   CPLT_set_linewidth(gc, 0.5);
   CPLT_draw_text(gc, 20., yp, "w", 0., "Polylines:");

   /* lines with markers */
   for (i = 0; i < 4; i++) {
      pts[i].x = x + i * dx;
      pts[i].y = y + (i % 2 ? dy : 0);
   }
   CPLT_set_color(gc, 0., 0., 1.);
   CPLT_draw_polyline(gc, 4, pts);
   for (i = 0; i < 4; i++)
      CPLT_draw_marker(gc, pts[i].x, pts[i].y, 4., 4);

   /* lines with dash patterns */
   x += 150.;
   dx = 100;
   yp -= 60;
   dy = 16;
   CPLT_set_linewidth(gc, 1.);
   CPLT_set_color(gc, 0., 0.6, 0.);
   for (i = 0; i < 4; i++) {
      pts[0].x = x;
      pts[0].y = y + i * dy;
      pts[1].x = x + dx;
      pts[1].y = y + i * dy;
      CPLT_set_linestyle(gc, i + 1);
      CPLT_draw_polyline(gc, 2, pts);
   }
   CPLT_set_linestyle(gc, CPLT_SolidLine);

   /* thick lines */
   x += 125.;
   pts[0].x = x;
   pts[0].y = y;
   pts[1].x = x;
   pts[1].y = y + 50.;
   pts[2].x = x + 50.;
   pts[2].y = y + 50.;
   pts[3].x = x + 50;
   pts[3].y = y;
   pts[4].x = x + 20.;
   pts[4].y = y;
   pts[5].x = x + 20.;
   pts[5].y = y + 30.;
   pts[6].x = x + 30.;
   pts[6].y = y + 30.;
   pts[7].x = x + 30.;
   pts[7].y = y + 10.;
   CPLT_set_color(gc, 0.8, 0.8, 0.);
   CPLT_set_linewidth(gc, 5);
   CPLT_draw_polyline(gc, 8, pts);
   CPLT_set_color(gc, 0., 0., 0.);
   CPLT_set_linewidth(gc, 1);


   /*
    * polygons
    */
   yp -= 30;
   y = yp;
   CPLT_draw_text(gc, 20., yp, "w", 0., "Polygons:");

   /* triangle */
   x = 120.;
   pts[0].x = x;
   pts[0].y = y - 20;
   pts[1].x = x + 60;
   pts[1].y = y - 20;
   pts[2].x = x + 30;
   pts[2].y = y + 30;
   CPLT_set_linewidth(gc, 3);
   CPLT_draw_polygon(gc, 3, pts);

   /* some more vertices */
   x += 100.;
   pts[0].x = x;
   pts[0].y = y;
   pts[1].x = x +  20;
   pts[1].y = y + 20;
   pts[2].x = x + 100;
   pts[2].y = y + 30;
   pts[3].x = x +  80;
   pts[3].y = y - 10;
   pts[4].x = x +  40;
   pts[4].y = y - 30;
   CPLT_set_color(gc, 0.9, 0.2, 0.2);
   CPLT_set_linewidth(gc, 0.5);
   CPLT_draw_polygon(gc, 5, pts);

   /* filled */
   x += 170.;
   pts[0].x = x;
   pts[0].y = y;
   pts[1].x = x - 30;
   pts[1].y = y + 20;
   pts[2].x = x + 40;
   pts[2].y = y + 30;
   pts[3].x = x + 70;
   pts[3].y = y - 10;
   pts[4].x = x - 20;
   pts[4].y = y - 30;
   CPLT_set_color(gc, 0.5, 0.5, 1.0);
   CPLT_draw_filledPolygon(gc, 5, pts);
   CPLT_set_color(gc, 0., 0., 0.);


   /*
    * anchored text
    */
   yp -= 90;
   CPLT_draw_text(gc, 20., yp, "w", 0., "Anchored texts:");
   x = 50.;
   y = yp - 60;
   CPLT_set_fontsize(gc, 10);
   for (i = 1; i <= 9; i++) {
      if ((i - 1) % 3 == 0) { y += 30; x = 50; }
      x += 120;
      CPLT_set_color(gc, 0., 0., 0.);
      CPLT_draw_text(gc, x, y, anchors[i], 0., "Textstring");
      CPLT_set_color(gc, 1., 0., 0.);
      CPLT_draw_marker(gc, x, y, 10., 1);
   }
   CPLT_set_fontsize(gc, 12);
   CPLT_set_color(gc, 0., 0., 0.);


   /*
    * rotated text
    */
   yp -= 110;
   CPLT_draw_text(gc, 20., yp,    "w", 0., "Rotated,");
   CPLT_draw_text(gc, 20., yp-14, "w", 0., "colored texts:");
   x = 260.;
   y = yp;
   c = 1.0;
   CPLT_set_fontsize(gc, 11);
   for (i = 15; i < 360; i += 30) {
      CPLT_set_color(gc, 1. - c, c, 0.);
      CPLT_draw_text(gc, x, y, "w", (float)i, "Textstring");
      c -= 1. / 11.;
   }
   CPLT_set_fontsize(gc, 12);
   CPLT_set_color(gc, 0., 0., 0.);


   /*
    * arcs
    */
   yp -= 100;
   CPLT_draw_text(gc, 20, yp,    "w", 0., "Circular");
   CPLT_draw_text(gc, 20, yp-14, "w", 0., "arcs:");

   /* arcs with varying radii, sectors/angles and colors */
   x = 160.;
   y = yp - 30.;
   r = 70., s = 60, e = 210;
   c = 1.0;
   for (i = 0; i < 8; i++) {
      CPLT_set_color(gc, 0., c, 1.);
      CPLT_draw_arc(gc, x, y, r, s, e);
      x += 2.;
      y -= 2.;
      r *= 0.75;
      s -= 15;
      e += 15;
      c -= 1. / 8.;
   }

   /* 2 filled, shifted "pie slices" */
   x = 270.;
   y = yp - 30.;
   r = 50.;
   s = 210.;
   e = 310.;
   CPLT_set_color(gc, 0.7, 0.3, 0.8);
   CPLT_draw_filledArc(gc, x, y, r, s, e);

   ofs = 5.;
   x -= ofs * cos(0.5 * (s + e) * DEG2RAD);
   y -= ofs * sin(0.5 * (s + e) * DEG2RAD);
   CPLT_set_color(gc, 0.8, 0.4, 0.2);
   CPLT_draw_filledArc(gc, x, y, r, e - 360., s);
   CPLT_set_color(gc, 0., 0., 0.);

   /* filled circles with varying radii and colors */
   x = 410.;
   y = yp - 20.;
   r = 70., c = 1.;
   for (i = 0; i < 8; i++) {
      CPLT_set_color(gc, 0., c, 0.);
      CPLT_draw_filledArc(gc, x, y, r, 0., 360.);
      x += 2.;
      y -= 2.;
      r *= 0.75;
      c *= 0.8;
   }
   CPLT_set_color(gc, 0., 0., 0.);


   /*
    * Bezier curves
    */
   yp -= 150;
   c = 0.6;
   CPLT_draw_text(gc, 20, yp,    "w", 0., "Cubic");
   CPLT_draw_text(gc, 20, yp-14, "w", 0., "Bézier curves:");

   /* three smoothly connected Bézier curve segments,
    * showing control points too */
   x = 160.;
   y = yp;
   pts[0].x = x;
   pts[0].y = y;
   pts[1].x = x + 30;
   pts[1].y = y + 50;
   pts[2].x = x + 70;
   pts[2].y = y + 60;
   pts[3].x = x + 60;
   pts[3].y = y;
   CPLT_set_linewidth(gc, 0.5);
   CPLT_set_color(gc, c, c, c);
   CPLT_set_linestyle(gc, CPLT_DashLine);
   CPLT_draw_polyline(gc, 4, pts);
   CPLT_set_linestyle(gc, CPLT_SolidLine);
   for (i = 0; i < 4; i++)
      CPLT_draw_marker(gc, pts[i].x, pts[i].y, 3., i % 3 ? 3 : 4);
   CPLT_set_linewidth(gc, 2);
   CPLT_set_color(gc, 1., 0., 0.);
   CPLT_draw_curve(gc, pts);  /* always 4 points per segment */

   x += 60;
   pts[0].x = x;
   pts[0].y = y;
   pts[1].x = x -  7;
   pts[1].y = y - 40;
   pts[2].x = x + 60;
   pts[2].y = y + 30;
   pts[3].x = x + 120;
   pts[3].y = y;
   CPLT_set_linewidth(gc, 0.5);
   CPLT_set_color(gc, c, c, c);
   CPLT_set_linestyle(gc, CPLT_DashLine);
   CPLT_draw_polyline(gc, 4, pts);
   CPLT_set_linestyle(gc, CPLT_SolidLine);
   for (i = 0; i < 4; i++)
      CPLT_draw_marker(gc, pts[i].x, pts[i].y, 3., i % 3 ? 3 : 4);
   CPLT_set_linewidth(gc, 2);
   CPLT_set_color(gc, 1., 0., 0.);
   CPLT_draw_curve(gc, pts);  /* always 4 points per segment */

   x += 120;
   pts[0].x = x;
   pts[0].y = y;
   pts[1].x = x + 90;
   pts[1].y = y - 45;
   pts[2].x = x - 40;
   pts[2].y = y - 40;
   pts[3].x = x + 80;
   pts[3].y = y + 10;
   CPLT_set_linewidth(gc, 0.5);
   CPLT_set_color(gc, c, c, c);
   CPLT_set_linestyle(gc, CPLT_DashLine);
   CPLT_draw_polyline(gc, 4, pts);
   CPLT_set_linestyle(gc, CPLT_SolidLine);
   for (i = 0; i < 4; i++)
      CPLT_draw_marker(gc, pts[i].x, pts[i].y, 3., i % 3 ? 3 : 4);
   CPLT_set_linewidth(gc, 2);
   CPLT_set_color(gc, 1., 0., 0.);
   CPLT_draw_curve(gc, pts);  /* always 4 points per segment */


   /* finish graphics */
   CPLT_finish_graphics(gc);
   printf("Done, test-figures written to plotfile '%s'.\n", plotfilename);

   return 0;
}



