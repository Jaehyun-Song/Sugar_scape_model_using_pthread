#include"./simulation.h"
#include"./lcgrand.h"

enum direction
{
	UPPER_X, LOWER_X, UPPER_Y, LOWER_Y, UPPER_Z, LOWER_Z
};

int const_x;
int const_y;
int peoples;
int g_sugar_consume_cnt;

int *arr_len;
pthread_spinlock_t *g_map_lock;
pthread_spinlock_t g_random_lock;
pthread_spinlock_t g_sugar_lock;
pthread_barrier_t g_sugar_barrier;
int *random_x;
int *random_y;
int *random_z;
int g_random_use;
int g_map_seed[3];
int g_people_seed[6];
int g_lower[4];
int g_upper[4];
bool random_flag;

void find_direction (int x, int y, int z, enum direction *);
int compare_x (const void *, const void *);
int compare_y (const void *, const void *);
int compare_z (const void *, const void *);
int compare_eat (const void *, const void *);
int compare_consume (const void *, const void *);
int compare_sugar (const void *, const void *);
void print_buffer(void);

void init_resource(struct setup *s)
{
	int map_volume;
	int cnt, rem;
	int i, temp;

	peoples = s->total_people;
	const_y = s->map_size;
	const_x = const_y * const_y;
	g_sugar_consume_cnt = 0;
	map_volume = const_x * const_y;
	cnt = peoples / g_thread_num;
	rem = peoples % g_thread_num;
	g_random_max = peoples * (((s->min_consume + s->max_consume) / 2) + 5);
	g_random_use = 0;

	temp = g_random_max % g_thread_num;
	temp = (g_thread_num - temp);
	g_random_max += temp;

	g_random_length = g_random_max / g_thread_num;
	random_flag = false;

	g_people_seed[0] = s->SEED_PEOPLE_X;
	g_people_seed[1] = s->SEED_PEOPLE_Y;
	g_people_seed[2] = s->SEED_PEOPLE_Z;
	g_people_seed[3] = s->SEED_PEOPLE_EAT;
	g_people_seed[4] = s->SEED_PEOPLE_CONSUME;
	g_people_seed[5] = s->SEED_PEOPLE_SUGAR;

	g_lower[0] = 0;
	g_lower[1] = s->min_eating;
	g_lower[2] = s->min_consume;
	g_lower[3] = s->min_sugar;

	g_upper[0] = s->map_size - 1;
	g_upper[1] = s->max_eating;
	g_upper[2] = s->max_consume;
	g_upper[3] = s->max_sugar;

	g_p = (struct people**)malloc(sizeof(struct people*)*g_thread_num);
	g_m = (struct map*)malloc(sizeof(struct map)*map_volume);
	g_sugar_map = (int*)malloc(sizeof(int)*map_volume);

	arr_len = (int*)malloc(sizeof(int)*g_thread_num);
	random_x = (int*)malloc(sizeof(int)*g_random_max);
	random_y = (int*)malloc(sizeof(int)*g_random_max);
	random_z = (int*)malloc(sizeof(int)*g_random_max);

	g_map_lock = (pthread_spinlock_t*)malloc(sizeof(pthread_spinlock_t)*map_volume);

	for(i = 0; i < rem; i++)
	{
		arr_len[i] = cnt + 1;
		g_p[i] = (struct people*)malloc(sizeof(struct people)*arr_len[i]);
	}

	for(i; i < g_thread_num; i++)
	{
		arr_len[i] = cnt;
		g_p[i] = (struct people*)malloc(sizeof(struct people)*arr_len[i]);
	}

	for(i = 0; i < map_volume; i++)
	{
		g_m[i].eat = 0;
		g_m[i].consume = 0;
		g_m[i].sugar = 0;
		g_m[i].merge_cnt = 1;
		g_m[i].is_move = false;

		pthread_spin_init(&g_map_lock[i], 0);
	}

	pthread_spin_init(&g_random_lock, 0);
	pthread_spin_init(&g_sugar_lock, 0);
	pthread_barrier_init(&g_sugar_barrier, NULL, g_thread_num);

	g_map_seed[0] = s->SEED_MAP_X;
	g_map_seed[1] = s->SEED_MAP_Y;
	g_map_seed[2] = s->SEED_MAP_Z;
}

void destroy_resource(void)
{
	int i;

	for(i = 0; i < g_thread_num; i++)
	{
		free(g_p[i]);
	}

	free(g_p);
	free(g_m);
	free(g_sugar_map);
	free(arr_len);
	free(random_x);
	free(random_y);
	free(random_z);
}

void people_move(int tid)
{
	register int i;
	enum direction d;
	int index;

	pthread_spin_lock(&g_sugar_lock);
	g_random_use += g_sugar_consume_cnt;
	g_sugar_consume_cnt = 0;
	pthread_spin_unlock(&g_sugar_lock);

	for (i = 0; i < arr_len[tid]; i++)
	{
		find_direction(g_p[tid][i].x, g_p[tid][i].y, g_p[tid][i].z, &d);

		switch(d)
		{
			case UPPER_X:	g_p[tid][i].x++;
				//printf("UPPER_X\n");
				break;
			case UPPER_Y:	g_p[tid][i].y++;
				//printf("UPPER_Y\n");
				break;
			case UPPER_Z:	g_p[tid][i].z++;
				//printf("UPPER_Z\n");
				break;
			case LOWER_X:	g_p[tid][i].x--;
				//printf("LOWER_X\n");
				break;
			case LOWER_Y:	g_p[tid][i].y--;
				//printf("LOWER_Y\n");
				break;
			case LOWER_Z:	g_p[tid][i].z--;
				//printf("LOWER_Z\n");
				break;
			default:			printf("People move ERROR\n");
		}

		index = (g_p[tid][i].x * const_x) + (g_p[tid][i].y * const_y) + g_p[tid][i].z;

		pthread_spin_lock(&g_map_lock[index]);

		if(!g_m[index].is_move)
		{
			g_m[index].is_move = true;
		}
		else
		{
			g_p[tid][i].is_dead = true;
			g_m[index].merge_cnt++;
			g_m[index].eat += g_p[tid][i].eat;
			g_m[index].consume += g_p[tid][i].consume;
			g_m[index].sugar += g_p[tid][i].sugar;
		}

		pthread_spin_unlock(&g_map_lock[index]);
	}
}

void people_merge_eat_create(int tid, struct setup *s)
{
	register int i;
	int index;
	int value[6];
	int my_sugar = 0;

	for(i = 0; i < arr_len[tid]; i++)
	{
		if(g_p[tid][i].is_dead)
		{
			pthread_spin_lock(&g_random_lock);
			gen_people(g_lower, g_upper, g_people_seed, value);
			pthread_spin_unlock(&g_random_lock);

			g_p[tid][i].x = value[0];
			g_p[tid][i].y = value[1];
			g_p[tid][i].z = value[2];
			g_p[tid][i].eat = value[3];
			g_p[tid][i].consume = value[4];
			g_p[tid][i].sugar = value[5];
			g_p[tid][i].is_dead = false;

			continue;
		}

		index = (g_p[tid][i].x * const_x) + (g_p[tid][i].y * const_y) + g_p[tid][i].z;
/////////////////////	MERGE	START	////////////////////////
		if(g_m[index].merge_cnt != 1)
		{
			int total_sugar = g_m[index].sugar + g_p[tid][i].sugar;

			g_p[tid][i].eat = (g_m[index].eat + g_p[tid][i].eat) / g_m[index].merge_cnt;
			g_p[tid][i].consume = (g_m[index].consume + g_p[tid][i].consume) / g_m[index].merge_cnt;
			g_p[tid][i].sugar = total_sugar / g_m[index].merge_cnt;

			my_sugar += (total_sugar - g_p[tid][i].sugar);

			g_m[index].eat = 0;
			g_m[index].consume = 0;
			g_m[index].sugar = 0;
			g_m[index].merge_cnt = 1;
		}
		g_m[index].is_move = false;
/////////////////////	MERGE	FINISH	////////////////////////


///////////////////////	EAT START	//////////////////////////
		if(g_sugar_map[index] >= g_p[tid][i].eat)
		{
			g_sugar_map[index] -= g_p[tid][i].eat;
			g_p[tid][i].sugar += g_p[tid][i].eat;
		}
		else
		{
			g_p[tid][i].sugar += g_sugar_map[index];
			g_sugar_map[index] = 0;
		}

		if(g_p[tid][i].consume < g_p[tid][i].sugar)
		{
			g_p[tid][i].sugar -= g_p[tid][i].consume;
			my_sugar += g_p[tid][i].consume;
		}
		else
		{
			my_sugar += g_p[tid][i].sugar;

			pthread_spin_lock(&g_random_lock);
			gen_people(g_lower, g_upper, g_people_seed, value);
			pthread_spin_unlock(&g_random_lock);

			g_p[tid][i].x = value[0];
			g_p[tid][i].y = value[1];
			g_p[tid][i].z = value[2];
			g_p[tid][i].eat = value[3];
			g_p[tid][i].consume = value[4];
			g_p[tid][i].sugar = value[5];
		}
///////////////////////	EAT FINISH	////////////////////////
	}

	pthread_spin_lock(&g_sugar_lock);
	g_sugar_consume_cnt += my_sugar;
	pthread_spin_unlock(&g_sugar_lock);
}

void create_sugar(int tid)
{
	int usable = g_random_max - g_random_use;
	int start;
	int cnt, rest;
	int index;
	register int i;

	while(usable < g_sugar_consume_cnt)
	{
		cnt = usable / g_thread_num;
		rest = usable % g_thread_num;

		if(rest > tid)
		{
			start = g_random_use + (tid * (cnt + 1));
			cnt += 1;
		}
		else
		{
			start = g_random_use + (rest * (cnt + 1)) + ((tid - rest) * cnt);
		}

		for(i = 0; i < cnt; i++)
		{
			index = (random_x[start+i] * const_x) + (random_y[start+i] * const_y) + random_z[start+i];

			pthread_spin_lock(&g_map_lock[index]);
			g_sugar_map[index]++;
			pthread_spin_unlock(&g_map_lock[index]);
		}

		random_flag = false;
		pthread_barrier_wait(&g_sugar_barrier);

		pthread_spin_lock(&g_sugar_lock);
		g_sugar_consume_cnt -= cnt;
		pthread_spin_unlock(&g_sugar_lock);

		get_random_value(tid);

		usable = g_random_max - g_random_use;
	}

	cnt = g_sugar_consume_cnt / g_thread_num;
	rest = g_sugar_consume_cnt % g_thread_num;

	if(rest > tid)
	{
		start = g_random_use + (tid * (cnt + 1));
		cnt += 1;
	}
	else
	{
		start = g_random_use + (rest * (cnt + 1)) + ((tid - rest) * cnt);
	}

	for(i = 0; i < cnt; i++)
	{
		index = (random_x[start+i] * const_x) + (random_y[start+i] * const_y) + random_z[start+i];

		pthread_spin_lock(&g_map_lock[index]);
		g_sugar_map[index]++;
		pthread_spin_unlock(&g_map_lock[index]);
	}
}

void get_random_value(int tid)
{
	int start = tid * g_random_length;

	get_rv(tid, &random_x[start], &random_y[start], &random_z[start], g_map_seed, g_random_length, const_y);

	g_random_use = 0;
	pthread_barrier_wait(&g_sugar_barrier);

	pthread_spin_lock(&g_random_lock);

	if(!random_flag)
	{
		set_cons(g_thread_num);
		random_flag = true;
	}

	pthread_spin_unlock(&g_random_lock);
}

void print_buffer(void)
{
	int i;

	for(i = 0; i < g_random_max; i++)
	{
		printf("x :	%d, y :	%d, z :	%d\n", random_x[i], random_y[i], random_z[i]);
	}
}

void print_people(void)
{
	register int i;
	struct people *all;
	FILE* wfp = fopen("./output_people_list.txt", "w");
	int index, cnt = 0;

	all = (struct people*)malloc(sizeof(struct people)*peoples);

	for(index = 0; index < g_thread_num; index++)
	{
		for(i = 0; i < arr_len[index]; i++)
		{
			all[cnt] = g_p[index][i];
			cnt++;
		}
	}

	fprintf(wfp, "Total people: %d\n", peoples);
	fprintf(wfp, "Index\tx_axis\ty_axis\tz_axis\teating\tconsume\tsugar\n");

	qsort(all, peoples, sizeof(struct people), compare_sugar);
	qsort(all, peoples, sizeof(struct people), compare_consume);
	qsort(all, peoples, sizeof(struct people), compare_eat);
	qsort(all, peoples, sizeof(struct people), compare_z);
	qsort(all, peoples, sizeof(struct people), compare_y);
	qsort(all, peoples, sizeof(struct people), compare_x);

	for(i = 0; i < peoples; i++)
	{
		fprintf(wfp, "%d\t%d\t%d\t%d\t%d\t%d\t%d\n", i+1, all[i].x, all[i].y, all[i].z, all[i].eat, all[i].consume, all[i].sugar);
	}

	fclose(wfp);
}

void print_map(void)
{
	int map_volume = const_x * const_y;
	int i;
	FILE* wfp = fopen("./output_map_table.txt", "w");

	fprintf(wfp, "Map size: %d\n", const_y);

	for(i = 0; i < map_volume; i++)
	{
		fprintf(wfp, "%d ", g_sugar_map[i]);

		if ((i+1) % const_y == 0)
			fprintf(wfp, "\n");

		if ((i+1) % const_x == 0)
			fprintf(wfp, "\n");

	}
	fclose(wfp);
}

void find_direction (int x, int y, int z, enum direction *d)
{	
	int max_sugar = -1;
	int limit = const_y - 1;

	if(x != limit)
	{
		max_sugar = g_sugar_map[((x+1)*const_x) + (y*const_y) + z];
		*d = UPPER_X;
	}

	if(y != limit)
	{
		if(max_sugar < g_sugar_map[(x*const_x) + ((y+1)*const_y) + z])
		{
			max_sugar = g_sugar_map[(x*const_x) + ((y+1)*const_y) + z];
			*d = UPPER_Y;
		}
	}

	if(z != limit)
	{
		if(max_sugar < g_sugar_map[(x*const_x) + (y*const_y) + (z+1)])
		{
			max_sugar = g_sugar_map[(x*const_x) + (y*const_y) + (z+1)];
			*d = UPPER_Z;
		}
	}

	if(x != 0)
	{
		if(max_sugar < g_sugar_map[((x-1)*const_x) + (y*const_y) + z])
		{
			max_sugar = g_sugar_map[((x-1)*const_x) + (y*const_y) + z];
			*d = LOWER_X;
		}
	}

	if(y != 0)
	{
		if(max_sugar < g_sugar_map[(x*const_x) + ((y-1)*const_y) + z])
		{
			max_sugar = g_sugar_map[(x*const_x) + ((y-1)*const_y) + z];
			*d = LOWER_Y;
		}
	}

	if(z != 0)
	{
		if(max_sugar < g_sugar_map[(x*const_x) + (y*const_y) + (z-1)])
		{
			max_sugar = g_sugar_map[(x*const_x) + (y*const_y) + (z-1)];
			*d = LOWER_Z;
		}
	}
}

int compare_x (const void *arg1, const void *arg2)
{
	int e1, e2;

	e1 = ((struct people*)arg1)->x;
	e2 = ((struct people*)arg2)->x;

	if(e1 < e2)
		return -1;
	else if(e1 == e2)
		return 0;
	else
		return 1;
}

int compare_y (const void *arg1, const void *arg2)
{
	int e1, e2;

	e1 = ((struct people*)arg1)->y;
	e2 = ((struct people*)arg2)->y;

	if(e1 < e2)
		return -1;
	else if(e1 == e2)
		return 0;
	else
		return 1;
}

int compare_z (const void *arg1, const void *arg2)
{
	int e1, e2;

	e1 = ((struct people*)arg1)->z;
	e2 = ((struct people*)arg2)->z;

	if(e1 < e2)
		return -1;
	else if(e1 == e2)
		return 0;
	else
		return 1;
}

int compare_eat (const void *arg1, const void *arg2)
{
	int e1, e2;

	e1 = ((struct people*)arg1)->eat;
	e2 = ((struct people*)arg2)->eat;

	if(e1 < e2)
		return -1;
	else if(e1 == e2)
		return 0;
	else
		return 1;
}

int compare_consume (const void *arg1, const void *arg2)
{
	int e1, e2;

	e1 = ((struct people*)arg1)->consume;
	e2 = ((struct people*)arg2)->consume;

	if(e1 < e2)
		return -1;
	else if(e1 == e2)
		return 0;
	else
		return 1;
}

int compare_sugar (const void *arg1, const void *arg2)
{
	int e1, e2;

	e1 = ((struct people*)arg1)->sugar;
	e2 = ((struct people*)arg2)->sugar;

	if(e1 < e2)
		return -1;
	else if(e1 == e2)
		return 0;
	else
		return 1;
}


