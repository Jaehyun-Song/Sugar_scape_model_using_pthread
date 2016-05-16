#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "./lcgrand.h"
#include "./setup.h"
#include "./simulation.h"




pthread_t *g_pthreads;
pthread_spinlock_t g_id_lock;
pthread_barrier_t g_barrier;
struct setup s;
int total_loop;
int status;
int g_id_index;


static void initialize(void);
static void parameter_setup(struct setup *s);
static void end(void);
static double mysecond(void);
static void people_generator(struct setup *s);
static void map_generator(struct setup *s);

int get_pthread_id(void)
{
	int temp;

	pthread_spin_lock(&g_id_lock);
	temp = g_id_index++;
	pthread_spin_unlock(&g_id_lock);

	return temp;
}

void *t_function(void *arg)
{
	int i = 0;
	int tid = get_pthread_id();

	get_random_value(tid);

	while(i++ < total_loop)
	{
		people_move(tid);
		pthread_barrier_wait(&g_barrier);

		people_merge_eat_create(tid, &s);
		pthread_barrier_wait(&g_barrier);

		create_sugar(tid);
		pthread_barrier_wait(&g_barrier);
		//body
	}
}

void main(int argc, char *argv[])
{
	int i;
	double t;

	if(argc == 1)
	{
		g_thread_num = 1;
	}
	else
	{
		g_thread_num = atoi(argv[1]);
	}

	/*system setup_start*/
	initialize();
	/*system setup_end*/	
	t = mysecond();
	for(i = 0; i < g_thread_num; i++)
	{
		if(pthread_create(&g_pthreads[i], NULL, t_function, NULL))
		{
			perror("THREAD CREATE ERROR : ");
			exit(0);
		}
	}

	for(i = 0; i < g_thread_num; i++)
	{
		pthread_join(g_pthreads[i], (void **)&status);
	}

	t = mysecond() - t;
	fprintf(stderr, "%lf\n", t);

	sleep(2);

	end();
}

void initialize(void)
{
	parameter_setup(&s);	//parameter setting
	init_resource(&s);
	map_generator(&s);
	people_generator(&s);
	init_cons(g_thread_num, g_random_length);

	g_pthreads = (pthread_t*)malloc(sizeof(pthread_t)*g_thread_num);
	pthread_spin_init(&g_id_lock, 0);
	pthread_barrier_init(&g_barrier, NULL, g_thread_num);
	g_id_index = 0;
}

void end(void)
{
	print_map();
	print_people();
	destroy_resource();
	destroy_fzrv();
	//print_output
}
void parameter_setup(struct setup *s)
{
	FILE* rfp = fopen("./test_setup.txt", "r");
	char trash[100];
	// Parameters about sugar scape model
	fscanf(rfp, "%s %d", trash, &total_loop);
	fscanf(rfp, "%s %d", trash, &s->map_size);
	fscanf(rfp, "%s %d", trash, &s->total_people);
	fscanf(rfp, "%s %d", trash, &s->min_map_sugar);
	fscanf(rfp, "%s %d", trash, &s->max_map_sugar);
	fscanf(rfp, "%s %d", trash, &s->min_eating);
	fscanf(rfp, "%s %d", trash, &s->max_eating);
	fscanf(rfp, "%s %d", trash, &s->min_consume);
	fscanf(rfp, "%s %d", trash, &s->max_consume);
	fscanf(rfp, "%s %d", trash, &s->min_sugar);
	fscanf(rfp, "%s %d", trash, &s->max_sugar);
	// Random seeds about sugar map
	fscanf(rfp, "%s %d", trash, &s->SEED_MAP_X);
	fscanf(rfp, "%s %d", trash, &s->SEED_MAP_Y);
	fscanf(rfp, "%s %d", trash, &s->SEED_MAP_Z);
	fscanf(rfp, "%s %d", trash, &s->SEED_MAP_SUGAR);
	// Random seeds aobut people
	fscanf(rfp, "%s %d", trash, &s->SEED_PEOPLE_X);
	fscanf(rfp, "%s %d", trash, &s->SEED_PEOPLE_Y);
	fscanf(rfp, "%s %d", trash, &s->SEED_PEOPLE_Z);
	fscanf(rfp, "%s %d", trash, &s->SEED_PEOPLE_EAT);
	fscanf(rfp, "%s %d", trash, &s->SEED_PEOPLE_CONSUME);
	fscanf(rfp, "%s %d", trash, &s->SEED_PEOPLE_SUGAR);
	fclose(rfp);
}

static double mysecond(void)
{
        struct timeval tp;
        struct timezone tzp;
        int i;

        i = gettimeofday(&tp,&tzp);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

static void people_generator(struct setup *s)
{
	int map_size = s->map_size;
	int total_people = s->total_people;
	int min_eating = s->min_eating;
	int max_eating = s->max_eating;
	int min_consume = s->min_consume;
	int max_consume = s->max_consume;
	int min_sugar = s->min_sugar;
	int max_sugar = s->max_sugar;
	int SEED_PEOPLE_X = s->SEED_PEOPLE_X;
	int SEED_PEOPLE_Y = s->SEED_PEOPLE_Y;
	int SEED_PEOPLE_Z = s->SEED_PEOPLE_Z;
	int SEED_PEOPLE_EAT = s->SEED_PEOPLE_EAT;
	int SEED_PEOPLE_CONSUME = s->SEED_PEOPLE_CONSUME;
	int SEED_PEOPLE_SUGAR = s->SEED_PEOPLE_SUGAR;
	int i, row = 0, column = 0;
	FILE* wfp = fopen("./gen_people_list.txt", "w");
	fprintf(wfp, "Total people: %d\n", total_people);
	fprintf(wfp, "Index\tx_axis\ty_axis\tz_asiz\teating\tconsume\tsugar\n");
	for(i = 0; i < total_people; i++)
	{
		int x = uniform(0, map_size-1, SEED_PEOPLE_X);
		int y = uniform(0, map_size-1, SEED_PEOPLE_Y);
		int z = uniform(0, map_size-1, SEED_PEOPLE_Z);
		int eat = uniform(min_eating, max_eating, SEED_PEOPLE_EAT);
		int consume = uniform(min_consume, max_consume, SEED_PEOPLE_CONSUME);
		int sugar = uniform(min_sugar, max_sugar, SEED_PEOPLE_SUGAR);

/////////////////////////////////////////////////////////////////////
		g_p[row][column].x = x;
		g_p[row][column].y = y;
		g_p[row][column].z = z;
		g_p[row][column].eat = eat;
		g_p[row][column].consume = consume;
		g_p[row][column].sugar = sugar;
		g_p[row][column].is_dead = false;
		row++;

		if(row == g_thread_num)
		{
			column++;
			row = 0;
		}
/////////////////////////////////////////////////////////////////////

		fprintf(wfp, "%d\t%d\t%d\t%d\t%d\t%d\t%d\n", i+1, x, y, z, eat, consume, sugar);
	}
	fclose(wfp);
}



static void map_generator(struct setup *s)
{
	int map_size = s->map_size;
	int min_map_sugar = s->min_map_sugar;
	int max_map_sugar = s->max_map_sugar;
	int SEED_MAP_SUGAR = s->SEED_MAP_SUGAR;
	int i, j, k;
	FILE* wfp = fopen("./sugar_map.txt", "w");
	fprintf(wfp, "Map size: %d\n", map_size);
	for(i = 0; i<map_size; i++)
	{
		for(j = 0; j<map_size; j++)
		{
			for(k = 0; k<map_size; k++)
			{
/////////////////////////////////////////////////////////////////////
				int temp = (i * map_size * map_size) + (j * map_size) + k;
				g_sugar_map[temp] = uniform(min_map_sugar, max_map_sugar, SEED_MAP_SUGAR);
				fprintf(wfp, "%d ", g_sugar_map[temp]);
/////////////////////////////////////////////////////////////////////
			}
			fprintf(wfp, "\n");
		}		
		fprintf(wfp, "\n");
	}
	fclose(wfp);
}

