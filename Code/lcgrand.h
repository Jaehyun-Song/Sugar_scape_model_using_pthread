#ifndef __LIB_LCGRAND_H
#define __LIB_LCGRAND_H

/* The following 3 declarations are for use of the random-number generator
   lcgrand and the associated functions lcgrandst and lcgrandgt for seed
   management.  This file (named lcgrand.h) should be included in any program
   using these functions by executing
       #include "lcgrand.h"
   before referencing the functions. */

float lcgrand(int stream);
void  lcgrandst(long zset, int stream);
long  lcgrandgt(int stream);
int   uniform(int lower, int upper, int seed);
void	gen_people(int *, int *, int *, int *);
void	special_map_uni(int lower, int upper, int *, int *, int *, int *, int cnt);
void init_cons(int core, int cnt);
void set_cons(int core);
void get_rv(int tid, int *, int *, int *, int *, int cnt, int upper);
void destroy_fzrv(void);
#endif
