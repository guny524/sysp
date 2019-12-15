#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#include<net/if.h>  //inet_ntop(), IFNAMSIZ
#include<sys/ioctl.h>   //ioctl(), SIOCGIFADDR

#include<pthread.h>
#include<assert.h>

#include<mqueue.h>
#include<sys/stat.h>
#include<fcntl.h>

#include<termios.h>	//struct tertmios, tcgetattr(), tcsetattr();

#define IP_ITERFACE_NAME "wlp5s0"
#define GAME_PORT_NUM 41195

/* Initialize new terminal i/o settings */
void initTermios(int echo)
{
	struct termios old, current;
	tcgetattr(0, &old); /* grab old terminal i/o settings */
	current = old; /* make new settings same as old settings */
	current.c_lflag &= ~ICANON; /* disable buffered i/o */
	if (echo) {
		current.c_lflag |= ECHO; /* set echo mode */
	}
	else {
		current.c_lflag &= ~ECHO; /* set no echo mode */
	}
	tcsetattr(0, TCSANOW, &current); /* use these new terminal i/o settings now */
} /* Read 1 character - echo defines echo mode */
char getch()
{
	char ch;
	struct termios old;
	tcgetattr(STDIN_FILENO, &old);
	initTermios(0);	//echo 0
	ch = getchar();
	tcsetattr(0, TCSANOW, &old);	//reset Termios
	return ch;
}
int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}
static struct termios old, current;

#define MAP_COL 16
#define MAP_ROW 16

typedef struct Coordinate {
	int row, col;
}coor;
typedef struct Snake {
	coor arr[MAP_COL * MAP_ROW];
	int size;
}snake;

int finished = 0;

void wait()
{
	usleep(1000000);
}
void display(char arr[MAP_COL][MAP_ROW])
{
    for(int i=0;i<MAP_COL;i++)
    {
        for(int j=0;j<MAP_ROW;j++)
        {
            if(arr[i][j]==0)
                printf("  ");
            else
                printf("ㅁ");
        }
        printf("\n");
    }
}
void init_map(char arr[MAP_COL][MAP_ROW])
{
	for (int i = 0; i < MAP_COL; i++)
		for (int j = 0; j < MAP_ROW; j++)
			arr[i][j] = 0;

	for (int i = 0; i < MAP_COL; i++)
		arr[i][0] = 1;
	for (int i = 0; i < MAP_COL; i++)
		arr[i][MAP_ROW - 1] = 1;
	for (int i = 0; i < MAP_ROW; i++)
		arr[0][i] = 1;
	for (int i = 0; i < MAP_ROW; i++)
		arr[MAP_COL - 1][i] = 1;
}
void init_snake(snake *s)
{
	s->arr[0].col = rand() % (MAP_COL - 2) + 1;
	s->arr[0].row = rand() % (MAP_ROW - 2) + 1;
	s->size = 1;
}
int is_killed(snake *s, snake *other)   //by_ohter?
{
    for (int i = 0; i < other->size; i++)
		if(s->arr[0].row == other->arr[i].row && s->arr[0].col == other->arr[i].col)
			return 1;
    return 0;
}
void init_snake_avoid(snake *s, snake *avoid)
{
	s->arr[0].col = rand() % (MAP_COL - 2) + 1;
	s->arr[0].row = rand() % (MAP_ROW - 2) + 1;
	s->size = 1;
    if(is_killed(s, avoid))
        init_snake_avoid(s, avoid);
}
void merge_arr(char dest[MAP_COL][MAP_ROW], char map[MAP_COL][MAP_ROW], snake *s, snake *other, coor *f)
{
	for (int i = 0; i < MAP_COL; i++)
		for (int j = 0; j < MAP_ROW; j++)
			dest[i][j] = map[i][j];	//맵 표시
	for (int i = 0; i < s->size; i++)
		dest[s->arr[i].col][s->arr[i].row] = 1;
    for (int i = 0; i < other->size; i++)
        dest[other->arr[i].col][other->arr[i].row] = 1;
	dest[f->col][f->row] = 1;
}
void copy(snake *dest, snake *s)
{
	dest->size = s->size;
	for (int i = 0; i < s->size; i++)
		dest->arr[i] = s->arr[i];
}
void move(snake *s, char key)	//꼬리칸 머리에서 키보드 방향으로 이동
{
	for (int i=s->size-2;i>=0;i--)	//머리랑 꼬리만 이동 나머진 그냥 쉬프트
		s->arr[i+1] = s->arr[i];
	switch(key)	//머리 이동
	{
	case 'a':
		s->arr[0].row--;
		break;
	case 'w':
		s->arr[0].col--;
		break;
	case 's':
		s->arr[0].col++;
		break;
	case 'd':
		s->arr[0].row++;
		break;
	}
}
int is_crash(snake *s)	//with map?
{
	for (int i = 0; i < s->size; i++)
		if (s->arr[i].row == 0 || s->arr[i].row == MAP_ROW-1 || s->arr[i].col == 0 || s->arr[i].col == MAP_COL-1)
			return 1;
	return 0;
}
int is_collide(snake *s)	//it self?
{
	for (int i = 0; i < s->size; i++)
		for (int j = 0; j < i; j++)
			if ((s->arr[i].row == s->arr[j].row) && (s->arr[i].col == s->arr[j].col))
				return 1;
	return 0;
}
int is_consume(snake *s, coor *f)	//with feed?
{
	for (int i = 0; i < s->size; i++)
		if (s->arr[i].row == f->row && s->arr[i].col == f->col)
			return 1;
	return 0;
}
void init_feed(snake *s, snake *other, coor *f)
{
	f->col = rand() % (MAP_COL - 2) + 1;
	f->row = rand() % (MAP_ROW - 2) + 1;
	if (is_consume(s, f) || is_consume(other, f))
		init_feed(s, other, f);
}
void *cal(void *sockets)
{
	int *sock;
	int key='s', other_key='s';

    char arr[MAP_COL][MAP_ROW] = { 0, };
    char map[MAP_COL][MAP_ROW] = { 0, };

    snake s = { 0, };
    snake tmp_s = { 0, };
    snake other_s = {0,};
    snake tmp_other_s = {0,};
    coor f = { 0, };

	sock = (int*)sockets;

	srand((unsigned int)time(NULL));

	init_map(map);
	init_snake(&s);
    init_snake_avoid(&other_s,&s);
	init_feed(&s,&other_s,&f);

	merge_arr(arr, map, &s, &other_s, &f);
    system("clear");
	display(arr);

	while (1)
	{
		wait();
		if((read(sock[0], &key, sizeof(int))) <= 0)
		{
			perror("read0");
			exit(1);
		}
		if((read(sock[1], &other_key, sizeof(int))) <= 0)
		{
			perror("read1");
			exit(1);
		}

		copy(&tmp_s, &s);
        copy(&tmp_other_s, &other_s);

		move(&tmp_s, key);
        move(&tmp_other_s, other_key);

		if (is_crash(&tmp_s) || is_collide(&tmp_s) || is_killed(&tmp_s, &tmp_other_s))
		{
            /*
            if(is_crash(&tmp_s))
                printf("s crash\n");
            else if(is_collide(&tmp_s))
                printf("s collide\n");
            else if(is_killed(&tmp_s, &tmp_other_s))
                printf("s killed\n");
                */
			for(int i=0;i<MAP_COL;i++)
				for(int j=0;j<MAP_ROW;j++)
					arr[i][j] = -2;	//win
            write(sock[1], arr, sizeof(char)*MAP_COL*MAP_ROW);
			for(int i=0;i<MAP_COL;i++)
				for(int j=0;j<MAP_ROW;j++)
					arr[i][j] = -3;	//lose
			write(sock[0], arr, sizeof(char)*MAP_COL*MAP_ROW);
			break;
		}
        else if (is_crash(&tmp_other_s) ||is_collide(&tmp_other_s) || is_killed(&tmp_other_s, &tmp_s))
		{
            /*
            if(is_crash(&tmp_other_s))
                printf("other crash\n");
            else if(is_collide(&tmp_other_s))
                printf("other collide\n");
            else if(is_killed(&tmp_other_s, &tmp_s))
                printf("other killed\n");
                */
			for(int i=0;i<MAP_COL;i++)
				for(int j=0;j<MAP_ROW;j++)
					arr[i][j] = -2;	//win
            write(sock[0], arr, sizeof(char)*MAP_COL*MAP_ROW);
			for(int i=0;i<MAP_COL;i++)
				for(int j=0;j<MAP_ROW;j++)
					arr[i][j] = -3;	//lose
			write(sock[1], arr, sizeof(char)*MAP_COL*MAP_ROW);
			break;
		}
		else if (is_consume(&tmp_s, &f))
		{
			coor tmp = s.arr[s.size-1];
			move(&s, key);
			s.arr[s.size++] = tmp;
        	init_feed(&s,&other_s,&f);
		}
        else if (is_consume(&tmp_other_s, &f))
		{
			coor tmp = other_s.arr[other_s.size-1];
			move(&other_s, other_key);
			other_s.arr[other_s.size++] = tmp;
        	init_feed(&other_s,&s,&f);
		}
		else
        {
            move(&s, key);
            move(&other_s, other_key);
        }

		merge_arr(arr, map, &s, &other_s, &f);
        system("clear");
		display(arr);
        write(sock[0], arr, sizeof(char)*MAP_COL*MAP_ROW);
		write(sock[1], arr, sizeof(char)*MAP_COL*MAP_ROW);
	}

    close(sock);
    finished = 1;
}
void server_init()
{
    //서버 소켓 오픈, 주소 초기화
    int server_sock;
    struct sockaddr_in server_addr; //IPv4 address structure
    int addr_size;
    //IP 주소 받아오기
    int ip_sock;
    struct ifreq ifr;
    char ipstr[40];
    //억셉트, 클라이언트 소켓
    int client_socks[2];
    struct sockaddr_in client_addr; //IPv4 address structure
    //스레드
    pthread_t threads;
    int result_code;

    //소켓 열기
    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (server_sock == -1)
    {
        perror("socket() error");
        exit(1);
    }

    //서버 주소 초기화
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; //Allows IPv4 socket
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //모든 주소의 client 접속 가능
    server_addr.sin_port = htons(GAME_PORT_NUM);   // server socket binding

    //IP주소 받아오기
    ip_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (ip_sock == -1)
    {
        perror("socket() error");
        exit(1);
    }
    strncpy(ifr.ifr_name, IP_ITERFACE_NAME, IFNAMSIZ);
    if (ioctl(ip_sock, SIOCGIFADDR, &ifr) < 0)
    {
        perror("ioctl()_IP error");
        exit(1);
    }
    inet_ntop(AF_INET, ifr.ifr_addr.sa_data+2, ipstr, sizeof(struct sockaddr));
    printf("IP-Adrress: %s\n", ipstr);
    close(ip_sock);

    //서버 주소와 소켓 바인드
    if (bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind() error");
        exit(1);
    }

    //클라이언트 리슨
    if (listen(server_sock, 5) == -1)
    {
        perror("listen() error");
        exit(1);
    }

	for(int i=0;i<2;i++)
	{
	    addr_size = sizeof(client_addr);
	    client_socks[i] = accept(server_sock, (struct sockaddr*) &client_addr, &addr_size);
	    if (client_socks[i] == -1)
	    {
	        perror("accept() error");
	        exit(1);
	    }
        printf("Connection from: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	}

	result_code = pthread_create( &threads, NULL, cal, (void*)client_socks);
	assert(!result_code);
	result_code = pthread_detach(threads);
	assert(!result_code);

    while(finished==0);
    exit(1);
}
int main(int argc, char ** argv)
{
    if(argc == 1)
        server_init();
    else
    {
        printf("Usage server: %s\n", argv[0]);
        exit(1);
    }

    return 0;
}
