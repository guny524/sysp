#include<stdio.h>
#include<stdlib.h>	//rand(), srand();
#include<time.h>	//time(NULL);

typedef struct Coordinate {
	int row, col;
}coor;
typedef struct Snake {
	coor arr[MAP_COL * MAP_ROW];
	int size;
}snake;

void run_snake();

#include"display.h"
#include"snake.h"
#include"frame.h"

char key = 0;
snake s = { 0, };
coor feed = { 0, };
snake tmp_s = { 0, };

char arr[MAP_COL][MAP_ROW] = { 0, };
char map[MAP_COL][MAP_ROW] = { 0, };

void init_map()
{
	for (int i = 0; i < MAP_COL; i++)
		for (int j = 0; j < MAP_ROW; j++)
			map[i][j] = 0;

	for (int i = 0; i < MAP_COL; i++)
		map[i][0] = 1;
	for (int i = 0; i < MAP_COL; i++)
		map[i][MAP_ROW - 1] = 1;
	for (int i = 0; i < MAP_ROW; i++)
		map[0][i] = 1;
	for (int i = 0; i < MAP_ROW; i++)
		map[MAP_COL - 1][i] = 1;
}
void init_snake()
{
	s.arr[0].col = rand() % (MAP_COL - 2) + 1;
	s.arr[0].row = rand() % (MAP_ROW - 2) + 1;
	s.size = 1;
}
void merge_arr()
{
	for (int i = 0; i < MAP_COL; i++)
		for (int j = 0; j < MAP_ROW; j++)
			arr[i][j] = map[i][j];	//맵 표시
	for (int i = 0; i < s.size; i++)
		arr[s.arr[i].col][s.arr[i].row] = 1;
	arr[feed.col][feed.row] = 1;
}
void copy()
{
	tmp_s.size = s.size;
	for (int i = 0; i < s.size; i++)
		tmp_s.arr[i] = s.arr[i];
}
void move(snake *ss, char key)	//꼬리칸 머리에서 키보드 방향으로 이동
{
	for (int i=ss->size-2;i>=0;i--)	//머리랑 꼬리만 이동 나머진 그냥 쉬프트
		ss->arr[i+1] = ss->arr[i];
	switch(key)	//머리 이동
	{
	case 'a':
		ss->arr[0].row--;
		break;
	case 'w':
		ss->arr[0].col--;
		break;
	case 's':
		ss->arr[0].col++;
		break;
	case 'd':
		ss->arr[0].row++;
		break;
	}
}
int is_crash(snake *ss)	//with map?
{
	for (int i = 0; i < ss->size; i++)
		if (ss->arr[i].row == 0 || ss->arr[i].row == MAP_ROW-1 || ss->arr[i].col == 0 || ss->arr[i].col == MAP_COL-1)
			return 1;
	return 0;
}
int is_collide(snake *ss)	//it self?
{
	for (int i = 0; i < ss->size; i++)
		for (int j = 0; j < i; j++)
			if ((ss->arr[i].row == ss->arr[j].row) && (ss->arr[i].col == ss->arr[j].col))
				return 1;
	return 0;
}
int is_consume(snake *ss)	//with feed?
{
	for (int i = 0; i < ss->size; i++)
		if (ss->arr[i].row == feed.row && ss->arr[i].col == feed.col)
			return 1;
	return 0;
}
void init_feed(snake *ss)
{
	feed.col = rand() % (MAP_COL - 2) + 1;
	feed.row = rand() % (MAP_ROW - 2) + 1;
	if (is_consume(ss))
		init_feed(ss);
}
void finish()
{
	clear(arr);
	display(arr);
	for (int i=0;i<MAP_ROW;i++)
	{
		for (int j=0;j<MAP_COL;j++)
		{
			arr[i][j] = 1;
			display(arr);
		}
	}
}
void run_snake()
{
	int first = 1;
	srand((unsigned int)time(NULL));

	key = 0;

	init_map();
	init_snake();
	init_feed(&s);

	merge_arr();
	display(arr);

	while (1)
	{
		wait();
		if (kbhit())
		{
			key = getch();
			while (kbhit())
				getch();
		}
		if (key == 0x1b)	//ESC
			break;
		else if (key == 0 && first)
			first = 0;
		else
		{
			copy();
			move(&tmp_s, key);

			if (is_crash(&tmp_s) || is_collide(&tmp_s))
			{
				finish();
				return;
			}
			else if (is_consume(&tmp_s))
			{
				coor tmp = s.arr[s.size-1];
				move(&s, key);
				s.arr[s.size++] = tmp;
				init_feed(&s);
			}
			else
				move(&s, key);

			merge_arr();
			display(arr);
		}
	}
}
