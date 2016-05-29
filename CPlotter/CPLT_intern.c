/*
 * _Internal_ helper functions for 'CPlotter' ADT
 * which are not part of the public API.
 *
 * Author and Copyright: Dipl.-Ing. Horst-W. Radners, Berlin, 2015-2016
 * License: LGPL 3.0, see http://www.gnu.org/licenses/lgpl-3.0.en.html
 */

#include "CPLT_intern.h"

/*
 *******************************************************************************
 * internal helper functions
 *******************************************************************************
 */

char *_str_lowered_dup(const char *src) {
   /* returns a pointer to the lowercased copy of src or NULL.
    * The caller should free() the malloc'ed copy. */
   char *dst;
   int i = 0, l;

   if (src == NULL || *src == '\0') return NULL;

   l = strlen(src);
   dst = (char *)malloc((l + 1) * sizeof(char));
   if (!dst) return NULL;

   while ((dst[i] = tolower(src[i]))) i++;

   return dst;
}

/*
 *******************************************************************************
 */

char *_extract_lowered_suffix(const char *filename) {
   /* returns a pointer to the lowercased suffix, i.e. the chars _after_
    * the last '.' of filename or NULL if no such suffix exists.
    * The caller should free() the malloc'ed copy. */
   char *cp;
   int l;

   if (filename == NULL || *filename == '\0') return NULL;

   cp = strrchr(filename, '.');
   if (!cp) return NULL;
   l = strlen(++cp);
   if (l < 1 || l > 31) return NULL;   /* just for sure ... */

   return _str_lowered_dup(cp);
}

/*
 *******************************************************************************
 */

CPLT_point_t _midpoint(CPLT_point_t p1, CPLT_point_t p2) {
   /* returns the point at half of the distance
    * between the points p1 and p2 */

   CPLT_point_t m = {
      0.5 * (p1.x + p2.x),
      0.5 * (p1.y + p2.y)
   };

   return m;
}

/*
 *******************************************************************************
 */

int _rnd(const float x) {
   /* rounds a float to nearest integer */

   return (int)(x > 0 ? x + 0.5 : x - 0.5);
}

/*
 *******************************************************************************
 */

int _anchor_num_of(char *anchor) {
   /* returns the number [1-9] of the text anchor string, 0 on error */

   int i;
   char *as;
   static char *anchors[10] =
      { "", "sw", "s", "se", "w", "c", "e", "nw", "n", "ne" };

   for (i = 1; i < 10; i++)
      if (strncmp(anchors[i], anchor, 2) == 0) return i;

   as = strndup(anchor,2);
   fprintf(stderr," *** CPlotter: text anchor '%s' not recognized!\n",as);
   free(as);

   return 0;   /* not found, wrong anchor */
}

/*
 *******************************************************************************
 */

