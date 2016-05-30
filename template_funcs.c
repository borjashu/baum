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
void ploterplotfirst  (int wied, const unsigned int PSZ, CPLT_gc_t gc,int *a){
	CPLT_point_t points[2];
	int j=0;
int k=20;
	const float da=22;
	double l=119;
double windif=1.53938;
const float fac_l = 0.75;
	/* Konstante Pi definieren */



 CPLT_set_linewidth(gc, *a);
 
  
	points[0].x = PSZ/2.;        	 points[0].y = 1.;
	points[1].x = PSZ/2.;   	points[1].y =l ;


	CPLT_draw_polyline( gc,  2,points );

	plotleft (da,l*fac_l, wied,PSZ,&points,j, gc,windif,fac_l,&a,k);
	plotright(da,l*fac_l, wied,PSZ,&points,j, gc,windif,fac_l,&a,k);
}





void plotleft(  const float da,  double l,int wied,  const unsigned int PSZ,CPLT_point_t *points, int j,CPLT_gc_t gc,double windif,const float fac_l,int *a,int k){
	
	
    winkell(&windif);
	CPLT_point_t temp1[2];


	temp1[0].x = points[1].x;   						temp1[0].y = points[1].y;
	temp1[1].x = points[1].x-(cos(windif)*l);   				temp1[1].y = points[1].y+(sin(windif)*l);

	
   
 CPLT_set_linewidth(gc, k);
	CPLT_draw_polyline( gc,  2,temp1);



	if (j<wied){


		plotleft  (da,l*0.75,wied,PSZ,&temp1,j+1, gc,windif,fac_l,&a,k*0.75);
		plotright (da,l*0.75,wied,PSZ,&temp1,j+1, gc,windif,fac_l,&a,k*0.75);

	}

}


void plotright(  const float da,  double l,int wied,  const unsigned int PSZ,CPLT_point_t *points, int j,CPLT_gc_t gc,double windif,const float fac_l,int *a,int k){
	 
		
		winkelr(&windif);
	CPLT_point_t temp1[2];


	temp1[0].x = points[1].x;   						temp1[0].y = points[1].y;
	temp1[1].x = points[1].x-(cos(windif)*l);   				temp1[1].y = points[1].y+(sin(windif)*l);


          CPLT_set_linewidth(gc, k);       
	CPLT_draw_polyline( gc,  2,temp1);




	if (j<wied){


		plotleft  (da,l*0.75,wied,PSZ,&temp1,j+1, gc,windif,fac_l,&a,k*0.75);
		plotright (da,l*0.75,wied,PSZ,&temp1,j+1, gc,windif,fac_l,&a,k*0.75);


	}

}








