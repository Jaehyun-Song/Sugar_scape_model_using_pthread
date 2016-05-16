#ifndef __SUGAR_H__
#define __SUGAR_H__

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include"./setup.h"

#define true 1
#define false 0

typedef int bool;

struct setup s;

struct people
{
	int x;
	int y;
	int z;
	int eat;
	int consume;
	int sugar;
	bool is_dead;
};

struct map
{
	int eat;
	int consume;
	int sugar;
	int merge_cnt;
	bool is_move;
};

struct people **g_p;
struct map *g_m;
int *g_sugar_map;


int g_random_max;
int g_thread_num;
int g_random_length;


void init_resource(struct setup *);
void destroy_resource(void);
void people_move(int tid);
void people_merge_eat_create(int tid, struct setup *);
void create_sugar(int tid);
void get_random_value(int tid);
void print_people(void);
void print_map(void);

#endif
