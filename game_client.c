#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<mqueue.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<assert.h>
#include<time.h>
#include<termios.h>	//struct tertmios, tcgetattr(), tcsetattr();

#define GAME_PORT_NUM 41195

#define MAP_COL 16
#define MAP_ROW 16

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
void client(char *ip_address)
{
    //소켓
    int sock;
    struct sockaddr_in serv_addr;
    int str_len;
	int key = 's';
	char arr[MAP_COL][MAP_ROW]={0,};

    //소켓 열기
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("socket() error");
        exit(1);
    }

    //클라이언트 주소 초기화
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip_address);
    serv_addr.sin_port = htons(GAME_PORT_NUM);

    //서버와 연결
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("connect() error");
        exit(1);
    }
	system("clear");

    while(1)
    {
        if(kbhit())
		{
			key = getch();
			while (kbhit())
				getch();
		}
        if(write(sock, &key, sizeof(int))==-1)
        {
            perror("write");
            exit(1);
        }
        if(read(sock, (char*)&arr, sizeof(char)*MAP_COL*MAP_ROW) <= 0)
		{
			perror("read");
		}

		if(arr[0][0] == -2)
		{
			printf("WIN\n");
			break;
		}
		else if(arr[0][0] == -3)
		{
			printf("LOSE\n");
			break;
		}
        system("clear");
		display(arr);
    }

    close(sock);
}
int main(int argc, char ** argv)
{
    if(argc == 2)
    {
        client(argv[1]);
    }
    else
    {
        printf("Usage client: %s <IP>\n", argv[0]);
        exit(1);
    }

    return 0;
}
