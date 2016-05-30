/***********************************************************************
 * functions to
 * Template for using graphics-ADT CPlotter
 *
 ***********************************************************************/

//ТЫ ДИБИЛ j равен нулю!!!!!!!

#include "template_funcs.h"
#include <math.h>

float   winkell  (const float da, int j){
	double windif,windif1;
	//const float a=90;
	windif1	=  (90-(22*j));
	windif=(windif1*3.14)/180;
	return windif;
}
float   winkelr  (const float da, int j){
	double windif,windif1;
	//const float a=90;
	windif1	=  (90+(22*j));
	windif=(windif1*3.14)/180;
	return windif;
}

/* TODO */
void ploterplotfirst  (int wied, const unsigned int PSZ, CPLT_gc_t gc){
	CPLT_point_t points[2];
	int j=0;
	const float da=22;
	double l=119;


	/* Konstante Pi definieren */





	points[0].x = PSZ/2.;        	 points[0].y = 1.;
	points[1].x = PSZ/2.;   	points[1].y =points[0].y+l ;


	CPLT_draw_polyline( gc,  2,points );

	plotleft (da,l, wied,PSZ,&points,j, gc);
	plotright (da,l, wied,PSZ,&points,j, gc);
}




void plotleft(  const float da,  double l,int wied,  const unsigned int PSZ,CPLT_point_t *points, int j,CPLT_gc_t gc){

	float xy;
	xy=winkell(da,j);


	CPLT_point_t temp1[2];


	temp1[0].x = points[1].x;   						temp1[0].y = points[1].y;
	temp1[1].x = points[1].x-(cos(xy)*l);   				temp1[1].y = points[1].y+(sin(xy)*l);


	CPLT_draw_polyline( gc,  2,temp1);




	if (j<wied){


		plotleft  (da,l,wied,PSZ,&temp1,j+1, gc);
		plotright (da,l,wied,PSZ,&temp1,j, gc);

	}

}


void plotright(  const float da, double l,int wied,  const unsigned int PSZ,CPLT_point_t *points, int j,CPLT_gc_t gc){

	float xy;
	xy=winkelr(da,j+1);


	CPLT_point_t temp1[2];


	temp1[0].x = points[1].x;   						temp1[0].y = points[1].y;
	temp1[1].x = points[1].x-(cos(xy)*l);   				temp1[1].y = points[1].y+(sin(xy)*l);


	CPLT_draw_polyline( gc,  2,temp1);




	if (j<wied){


		plotleft  (da,l,wied,PSZ,&temp1,j, gc);
		plotright (da,l,wied,PSZ,&temp1,j+1, gc);

	}

}












