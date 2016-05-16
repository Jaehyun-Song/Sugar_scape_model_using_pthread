/* Prime modulus multiplicative linear congruential generator
   Z[i] = (630360016 * Z[i-1]) (mod(pow(2,31) - 1)), based on Marse and Roberts'
   portable FORTRAN random-number generator UNIRAN.  Multiple (100) streams are
   supported, with seeds spaced 100,000 apart.  Throughout, input argument
   "stream" must be an int giving the desired stream number.  The header file
   lcgrand.h must be included in the calling program (#include "lcgrand.h")
   before using these functions.

   Usage: (Three functions)

   1. To obtain the next U(0,1) random number from stream "stream," execute
          u = lcgrand(stream);
      where lcgrand is a float function.  The float variable u will contain the
      next random number.

   2. To set the seed for stream "stream" to a desired value zset, execute
          lcgrandst(zset, stream);
      where lcgrandst is a void function and zset must be a long set to the
      desired seed, a number between 1 and 2147483646 (inclusive).  Default
      seeds for all 100 streams are given in the code.

   3. To get the current (most recently used) integer in the sequence being
      generated for stream "stream" into the long variable zget, execute
          zget = lcgrandgt(stream);
      where lcgrandgt is a long function. */

#include<stdlib.h>
#include<stdio.h>

/* Define the constants. */

#define MODLUS 2147483647
#define MULT1       24112
#define MULT2       26143
#define MULT	630360016

long cons;
long *fzrv;

/* Set the default seeds for all 100 streams. */

static long zrng[] =
{         1,
 1973272912, 281629770,  20006270,1280689831,2096730329,1933576050,
  913566091, 246780520,1363774876, 604901985,1511192140,1259851944,
  824064364, 150493284, 242708531,  75253171,1964472944,1202299975,
  233217322,1911216000, 726370533, 403498145, 993232223,1103205531,
  762430696,1922803170,1385516923,  76271663, 413682397, 726466604,
  336157058,1432650381,1120463904, 595778810, 877722890,1046574445,
   68911991,2088367019, 748545416, 622401386,2122378830, 640690903,
 1774806513,2132545692,2079249579,  78130110, 852776735,1187867272,
 1351423507,1645973084,1997049139, 922510944,2045512870, 898585771,
  243649545,1004818771, 773686062, 403188473, 372279877,1901633463,
  498067494,2087759558, 493157915, 597104727,1530940798,1814496276,
  536444882,1663153658, 855503735,  67784357,1432404475, 619691088,
  119025595, 880802310, 176192644,1116780070, 277854671,1366580350,
 1142483975,2026948561,1053920743, 786262391,1792203830,1494667770,
 1923011392,1433700034,1244184613,1147297105, 539712780,1545929719,
  190641742,1645390429, 264907697, 620389253,1502074852, 927711160,
  364849192,2049576050, 638580085, 547070247 };

/* Generate the next random number. */

float lcgrand(int stream)
{
    long zi, lowprd, hi31;

    zi     = zrng[stream];
    zi     = (zi * MULT) % MODLUS;
    zrng[stream] = zi;
/*
    zi     = zrng[stream];
    lowprd = (zi & 65535) * MULT1;
    hi31   = (zi >> 16) * MULT1 + (lowprd >> 16);
    zi     = ((lowprd & 65535) - MODLUS) +
             ((hi31 & 32767) << 16) + (hi31 >> 15);
    if (zi < 0) zi += MODLUS;
    lowprd = (zi & 65535) * MULT2;
    hi31   = (zi >> 16) * MULT2 + (lowprd >> 16);
    zi     = ((lowprd & 65535) - MODLUS) +
             ((hi31 & 32767) << 16) + (hi31 >> 15);
    if (zi < 0) zi += MODLUS;
    zrng[stream] = zi;
		*/

    return (zi >> 7 | 1) / 16777216.0;
}

void lcgrandst (long zset, int stream) /* Set the current zrng for stream
                                          "stream" to zset. */
{
    zrng[stream] = zset;
}

long lcgrandgt (int stream) /* Return the current zrng for stream "stream". */
{
    return zrng[stream];
}

int uniform(int lower, int upper, int seed)
{
	return (int)(lower + (upper + 1 - lower) * lcgrand(seed));
}

void gen_people(int *lower, int *upper, int *seed, int *value)
{
	value[0] = (int)(lower[0] + (upper[0] + 1 - lower[0]) * lcgrand(seed[0]));
	value[1] = (int)(lower[0] + (upper[0] + 1 - lower[0]) * lcgrand(seed[1]));
	value[2] = (int)(lower[0] + (upper[0] + 1 - lower[0]) * lcgrand(seed[2]));
	value[3] = (int)(lower[1] + (upper[1] + 1 - lower[1]) * lcgrand(seed[3]));
	value[4] = (int)(lower[2] + (upper[2] + 1 - lower[2]) * lcgrand(seed[4]));
	value[5] = (int)(lower[3] + (upper[3] + 1 - lower[3]) * lcgrand(seed[5]));
}

void special_map_uni(int lower, int upper, int *x, int *y, int *z, int *seed, int cnt)
{
	int index = 0;

	while(cnt != 0)
	{
		x[index] =  (int)(lower + (upper + 1 - lower) * lcgrand(seed[0]));
		y[index] =  (int)(lower + (upper + 1 - lower) * lcgrand(seed[1]));
		z[index] =  (int)(lower + (upper + 1 - lower) * lcgrand(seed[2]));
		
		cnt--;
		index++;
	}
}

void init_cons(int core, int cnt)
{
	register int i;

	fzrv = (long*)malloc(sizeof(long)*core);

	cons = 1;

	for(i = 0; i < cnt; i++)
	{
		cons = (cons * MULT) % MODLUS;
	}

	fzrv[0] = 1;

	if(core == 1)
	{
		return;
	}

	fzrv[1] = cons;

	for(i = 2; i < core; i++)
	{
		fzrv[i] = (fzrv[i-1] * cons) % MODLUS;
	}
}

void set_cons(int core)
{
	int i;

	fzrv[0] = (fzrv[core-1] * cons) % MODLUS;

	for(i = 1; i < core; i++)
	{
		fzrv[i] = (fzrv[i-1] * cons) % MODLUS;
	}
}

void get_rv(int tid, int *x, int *y, int *z, int *seed, int cnt, int upper)
{
	register int index = 0;
	long zi;
	long seed_x, seed_y, seed_z;
	float temp_x, temp_y, temp_z;

  	zi = fzrv[tid];
	seed_x = (zi * zrng[seed[0]]) % MODLUS;
	seed_y = (zi * zrng[seed[1]]) % MODLUS;
	seed_z = (zi * zrng[seed[2]]) % MODLUS;

	while(index != cnt)
	{
		seed_x = (seed_x * MULT) % MODLUS;
		seed_y = (seed_y * MULT) % MODLUS;
		seed_z = (seed_z * MULT) % MODLUS;

		temp_x = ((seed_x >> 7 | 1) / 16777216.0);
		temp_y = ((seed_y >> 7 | 1) / 16777216.0);
		temp_z = ((seed_z >> 7 | 1) / 16777216.0);

		x[index] = (int)(upper * temp_x);
		y[index] = (int)(upper * temp_y);
		z[index] = (int)(upper * temp_z);

		index++;
	}
}

void destroy_fzrv(void)
{
	free(fzrv);
}


