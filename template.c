/***********************************************************************
 * Template for using graphics-ADT CPlotter
 *
 ***********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "CPlotter.h"
#include "template_funcs.h"

int main() {

   const unsigned int PSZ = 600;    /* size [pix] of square plot area */

   CPLT_gc_t gc;                    /* graphics context */
   CPLT_point_t pts[4];             /* coord. points */
   char *plotfilename = "graphic.svg";
   
   int a=20;



  int wied=10;

   /* initialize graphics context */
   if ((gc = CPLT_init_graphics(PSZ, PSZ, plotfilename)) == NULL) {
      fprintf(stderr, "\n *** Can't initialize graphics context, abort!\n");
      return 1;
   }

   /* blue box border, title */
   CPLT_set_color(gc, 0., 0., 1.);
   CPLT_set_linewidth(gc, 2);
   pts[0].x = 1.;         pts[0].y = 1.;
   pts[1].x = PSZ - 1.;   pts[1].y = 1.;
   pts[2].x = PSZ - 1.;   pts[2].y = PSZ - 1.;
   pts[3].x = 1.;         pts[3].y = PSZ - 1.;
   
   	
	
	

   
   CPLT_draw_polygon(gc, 4, pts);
 
   CPLT_set_color(gc, 0., 0., 0.);
   CPLT_set_fontsize(gc, 16);
   CPLT_draw_text(gc, 0.5 * PSZ+15, PSZ - 20., "sw", 0., "Baum");
                                                                                                 

   /* TODO */

        			
  
   
        			
	
	
   			 ploterplotfirst  (wied,PSZ,gc);
   					
   			
   				
  	
  		 /* finish graphics */
 CPLT_finish_graphics(gc);
    
   printf("\n Done, plot written to plotfile '%s'.\n", plotfilename);

   return 0;
}
