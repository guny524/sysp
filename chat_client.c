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

struct termios old, current;
void initTermios(int echo) {
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

#define CHAT_PORT_NUM 41194
#define GAME_PORT_NUM 41195

#define BUFSIZE 1024
#define USER_NAME_SIZE 20

typedef struct Message {
    char str[BUFSIZE];
    char user_name[USER_NAME_SIZE];
    time_t time;
}message;

char user_name[USER_NAME_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *print(void *arg)
{
    mqd_t mqd_print;
    struct mq_attr attr;
    int prio=1;
    message m;

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(message);
    if ((mqd_print = mq_open("/print", O_RDWR | O_CREAT, 0644, &attr)) == (mqd_t) -1)
    {
        perror ("mqd_print_open");
        exit (1);
    }

    while (1)
    {
        if (mq_getattr(mqd_print, &attr) == -1)
        {
            perror ("Message Quene Getattr");
            exit (1);
        }
        if(attr.mq_curmsgs > 0)
        {
            if (mq_receive(mqd_print, (char*)&m, sizeof(message), &prio) == -1)
        	{
        		perror ("receive");
        		exit (1);
        	}
            pthread_mutex_lock(&mutex);
            printf("%s %s %ld\n", m.user_name, m.str, m.time);
            pthread_mutex_unlock(&mutex);
        }
    }
}
void *check(void *arg)
{
    mqd_t mqd_check;
    struct mq_attr attr;
    int prio=1;
    message m;

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(message);
    if ((mqd_check = mq_open("/check_client", O_RDWR | O_CREAT, 0644, &attr)) == (mqd_t) -1)
    {
        perror ("mqd_client_open");
        exit (1);
    }

    while (1)
    {
        if (mq_getattr(mqd_check, &attr) == -1)
        {
            perror ("Message Quene Getattr");
            exit (1);
        }
        if(attr.mq_curmsgs > 0)
        {
            if (mq_receive(mqd_check, (char*)&m, sizeof(message), &prio) == -1)
        	{
        		perror ("receive");
        		exit (1);
        	}
            if(strcmp(m.str,"snake!!\n")==0)
            {
                system("./game_client");
            }
        }
    }
}
void *receive(void *arg)
{
    int sock = *(int*)arg;
    message m;
    mqd_t mqd_print, mqd_check;
    int prio=1;

    mqd_print = mq_open("print", O_WRONLY);
    mqd_check = mq_open("check_client", O_WRONLY);

    while(1)
    {
        read(sock, &m, sizeof(message));
        mq_send(mqd_print, (const char*)&m, sizeof(message), prio);
        mq_send(mqd_check, (const char*)&m, sizeof(message), prio);
    }
}
void client(char *ip_address)
{
    //소켓
    int sock;
    struct sockaddr_in serv_addr;
    int str_len;

    //쓰레드
    pthread_t thread_print, thread_check, thread_receive;
    int result_code;

    void *arg_void;
    message m;

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
    serv_addr.sin_port = htons(CHAT_PORT_NUM);

    //서버와 연결
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("connect() error");
        exit(1);
    }

    //스레드 생성
    result_code = pthread_create( &thread_print, NULL, print, arg_void);
    assert(!result_code);
    result_code = pthread_detach(thread_print);
    assert(!result_code);

    result_code = pthread_create( &thread_check, NULL, check, arg_void);
    assert(!result_code);
    result_code = pthread_detach(thread_check);
    assert(!result_code);

    result_code = pthread_create( &thread_receive, NULL, receive, arg_void);
    assert(!result_code);
    result_code = pthread_detach(thread_receive);
    assert(!result_code);

    strcpy(m.user_name, user_name);
    while(1)
    {
        if(kbhit())
        {
            pthread_mutex_lock(&mutex);
            scanf("%s",m.str);
            pthread_mutex_unlock(&mutex);
            if(write(sock, &m, sizeof(message))==-1)
            {
                perror("write");
                exit(1);
            }
            if (!strcmp(m.str, "q"))
            {
                pthread_cancel(thread_receive);
                pthread_cancel(thread_check);
                pthread_cancel(thread_print);
                break;
            }
        }
    }
}
int main(int argc, char ** argv)
{
    if(argc == 3)
    {
        if(strlen(argv[2]) >= USER_NAME_SIZE)
            return 0;
        strcpy(user_name, argv[2]);
        client(argv[1]);
    }
    else
    {
        printf("Usage client: %s <IP> <USER_NAME>\n", argv[0]);
        exit(1);
    }

    return 0;
}
