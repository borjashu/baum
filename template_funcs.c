/***********************************************************************
 * functions to
 * Template for using graphics-ADT CPlotter
 *
 ***********************************************************************/

#include "template_funcs.h"
#include <math.h>

/* TODO */
void ploterplotfirst  (double da,double l, int wied, const unsigned int PSZ, CPLT_gc_t gc){
	CPLT_point_t points[2];
	int j=0;
	  /* Konstante Pi definieren */

	
  


	points[0].x = PSZ/2.;        	 points[0].y = 1.;
	points[1].x = PSZ/2.;   	points[1].y =points[0].y+l ;


	CPLT_draw_polyline( gc,  2,points );



	plotleft ( da,l, wied,PSZ,&points,j, gc);
	plotright( da,l, wied,PSZ,&points,j, gc);;
}




void plotleft(double da,double l, int wied,  const unsigned int PSZ,CPLT_point_t *points, int j,CPLT_gc_t gc){

double xdev=(0.927183855*l),ydev=(0.374606593*l);

	CPLT_point_t temp1[2];


	temp1[0].x = points[1].x;   			temp1[0].y = points[1].y;
	temp1[1].x = points[1].x-xdev;   		temp1[1].y = points[1].y+ydev;


	CPLT_draw_polyline( gc,  2,temp1);

	if (j<wied){						


		plotleft  (da,l,wied,PSZ,&temp1,j+1, gc);
		plotright (da,l,wied,PSZ,&temp1,j+1, gc);

	}    
//110.334878693

}    











void plotright(double da,double l, int wied,  const unsigned int PSZ,CPLT_point_t *points, int j,CPLT_gc_t gc){


	
	double xdev=(0.927183855*l),ydev=(0.374606593*l);

	CPLT_point_t temp1[2];


	temp1[0].x = points[1].x;   			temp1[0].y = points[1].y;
	temp1[1].x = points[1].x+xdev;   		temp1[1].y = points[1].y+ydev;


	CPLT_draw_polyline( gc,  2,temp1);

	/*if (j<wied){						


		plotleft  (da,l,wied,PSZ,&temp1,j+1, gc);
		//plotright (wied,PSZ,&temp1,j+1, gc);

	}    */


}











