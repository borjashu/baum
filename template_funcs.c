/***********************************************************************
 * functions to
 * Template for using graphics-ADT CPlotter
 *
 ***********************************************************************/



#include "template_funcs.h"
#include <math.h>

void   winkell  (double *windif){


	*windif	=  (*windif-0.345575);


}
void   winkelr  (double *windif){


	*windif	=  (*windif+0.345575);
}

/* TODO */
void ploterplotfirst  (int wied, const unsigned int PSZ, CPLT_gc_t gc){
	CPLT_point_t points[2];
	int j=0;
	int a=20;
	int k=20;
	double l=119;
	double windif=1.53938;
	float R=0.5, G=0.5, B=0.1;
	const float fac_l = 0.75;
	/* Konstante Pi definieren */


color(gc,R,G,B,j);
	CPLT_set_linewidth(gc, k);


	points[0].x = PSZ/2.;        	 points[0].y = 1.;
	points[1].x = PSZ/2.;   	points[1].y =l ;


	CPLT_draw_polyline( gc,  2,points );

	plotleft(l*fac_l, wied,PSZ,&points,j, gc,windif,fac_l,&a,k,R,G,B);
	plotright(l*fac_l, wied,PSZ,&points,j, gc,windif,fac_l,&a,k,R,G,B);
}





void plotleft( double l,int wied,  const unsigned int PSZ,CPLT_point_t *points, int j,CPLT_gc_t gc,double windif,const float fac_l,int *a,int k,
	double R,double G,double B){


	winkell(&windif);
	CPLT_point_t temp1[2];


	temp1[0].x = points[1].x;   						temp1[0].y = points[1].y;
	temp1[1].x = points[1].x-(cos(windif)*l);   				temp1[1].y = points[1].y+(sin(windif)*l);



	CPLT_set_linewidth(gc, k);
	CPLT_draw_polyline( gc,  2,temp1);

color(gc,R,G,B,j);
	
if (j<wied){


		plotleft  (l*0.75,wied,PSZ,&temp1,j+1, gc,windif,fac_l,&a,k*0.75,R,G,B);
		plotright (l*0.75,wied,PSZ,&temp1,j+1, gc,windif,fac_l,&a,k*0.75,R,G,B);

	}

}


void plotright( double l,int wied,  const unsigned int PSZ,CPLT_point_t *points, int j,CPLT_gc_t gc,double windif,const float fac_l,int *a,int k,
	double R,double G,double B){


	winkelr(&windif);
	CPLT_point_t temp1[2];


	temp1[0].x = points[1].x;   						temp1[0].y = points[1].y;
	temp1[1].x = points[1].x-(cos(windif)*l);   				temp1[1].y = points[1].y+(sin(windif)*l);


	CPLT_set_linewidth(gc, k);
	CPLT_draw_polyline( gc,  2,temp1);

color(gc,R,G,B,j);


	if (j<wied){


		plotleft  (l*0.75,wied,PSZ,&temp1,j+1, gc,windif,fac_l,&a,k*0.75,R,G,B);
		plotright (l*0.75,wied,PSZ,&temp1,j+1, gc,windif,fac_l,&a,k*0.75,R,G,B);


	}

}


void color(CPLT_gc_t gc,double R,double G,double B,int j){

R=0.25-(j*0.025);
G=0.1+(j*0.07);
B=0.1;
CPLT_set_color(gc, R, G, B);

}




