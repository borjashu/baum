#ifndef _CPLT_intern_H_
#define _CPLT_intern_H_

/*
 * _Internal_ header-file of 'CPlotter' ADT
 * for internal typedefs and function prototypes,
 * which are not part of the public API.
 *
 * Author and Copyright: Dipl.-Ing. Horst-W. Radners, Berlin, 2015-2016
 * License: LGPL 3.0, see http://www.gnu.org/licenses/lgpl-3.0.en.html
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CPlotter.h"

/************************************************************************/

/* constants, conversion factors: degrees ==> radians and vice versa */
#define PI       3.141592653589793
#define DEG2RAD  0.017453292519943
#define RAD2DEG 57.295779513082321

/************************************************************************/

/* === _internal_ type definitions === */

/* the function types */
typedef CPLT_gc_t INIT_ft(const unsigned int pwidth,
                          const unsigned int pheight,
                          char *plotfilename);
typedef void PLINE_ft(CPLT_gc_t gc, const int numpts,
                      CPLT_point_t points[]);
typedef void PGON_ft(CPLT_gc_t gc, const int numpts,
                     CPLT_point_t points[]);
typedef void PGONF_ft(CPLT_gc_t gc, const int numpts,
                      CPLT_point_t points[]);
typedef void ARC_ft(CPLT_gc_t gc, const float cx, const float cy,
                    const float radius,
                    const float start, const float end);
typedef void ARCF_ft(CPLT_gc_t gc, const float cx, const float cy,
                     const float radius,
                     const float start, const float end);
typedef void CURVE_ft(CPLT_gc_t gc, CPLT_point_t points[]);
typedef void MARK_ft(CPLT_gc_t gc, const float cx, const float cy,
                     const int wd, const int symbol);
typedef void TEXT_ft(CPLT_gc_t gc, const float x, const float y,
                     char *anchor, float angle, char *text);
typedef void FSIZE_ft(CPLT_gc_t gc, const float fontsize);
typedef void COLR_ft(CPLT_gc_t gc, float r, float g, float b);
typedef void LNWD_ft(CPLT_gc_t gc, const float w);
typedef void LNSTY_ft(CPLT_gc_t gc, const CPLT_lnstyle_t s);
typedef void FINI_ft(CPLT_gc_t gc);

/* the type for the dispatch table, named pointers to the API functions */
typedef struct {
   INIT_ft  *INIT;
   PLINE_ft *PLINE;
   PGON_ft  *PGON;
   PGONF_ft *PGONF;
   ARC_ft   *ARC;
   ARCF_ft  *ARCF;
   CURVE_ft *CURVE;
   MARK_ft  *MARK;
   TEXT_ft  *TEXT;
   FSIZE_ft *FSIZE;
   COLR_ft  *COLR;
   LNWD_ft  *LNWD;
   LNSTY_ft *LNSTY;
   FINI_ft  *FINI;
} CPLT_funcn_t;

/************************************************************************/

/* === _internal_ function prototypes === */

char *_str_lowered_dup(const char *src);
/* returns a pointer to the lowercased copy of src or NULL.
 * The caller should free() the malloc'ed copy. */

char *_extract_lowered_suffix(const char *filename);
/* returns a pointer to the lowercased suffix, i.e. the chars _after_
 * the last '.' of filename or NULL if no such suffix exists.
 * The caller should free() the malloc'ed copy. */

CPLT_point_t _midpoint(CPLT_point_t p1, CPLT_point_t p2);
/* returns the point at half of the distance
 * between the points p1 and p2 */

int _rnd(const float x);
/* rounds a float to nearest integer */

int _anchor_num_of(char *anchor);
/* returns the number [1-9] of the text anchor string, 0 on error */

#endif

