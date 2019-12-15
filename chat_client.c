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

#define CHAT_PORT_NUM 41194

#define BUFSIZE 1024
#define USER_NAME_SIZE 20

typedef struct Message {
    char str[BUFSIZE];
    char user_name[USER_NAME_SIZE];
    time_t time;
}message;

char user_name[USER_NAME_SIZE];

void *check(void *arg)
{
    mqd_t mqd_check;
    struct mq_attr attr;
    message m;

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(message);
    if ((mqd_check = mq_open("/check", O_RDWR | O_CREAT, 0644, &attr)) == (mqd_t) -1)
    {
        perror ("mqd_check_create");
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
            if (mq_receive(mqd_check, (char*)&m, sizeof(message), NULL) == -1)
        	{
        		perror ("receive");
        		exit (1);
        	}
            if(strcmp(m.str,"snake!!")==0)
            {
				scanf("%s",m.str);
                system("./game_client");
            }
        }
    }
}
void *receiving(void *socket)
{
    int sock = *(int*)socket;
    mqd_t mqd_check;
    int prio=1;
    message m;

    mqd_check = mq_open("check", O_WRONLY);

    while(1)
    {
        if(read(sock, &m, sizeof(message)) <= 0)
		{
			printf("cant read\n");
		}
		printf("%s : %s : %s\n", m.user_name, m.str, ctime(&(m.time)));
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
    pthread_t thread_check, thread_receiving;
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
    result_code = pthread_create( &thread_check, NULL, check, arg_void);
    assert(!result_code);
    result_code = pthread_detach(thread_check);
    assert(!result_code);

	result_code = pthread_create( &thread_receiving, NULL, receiving, (void*)&sock);
    assert(!result_code);
    result_code = pthread_detach(thread_receiving);
    assert(!result_code);

    strcpy(m.user_name, user_name);
    while(1)
    {
		scanf("%s",m.str);
		if(write(sock, &m, sizeof(message)) <= 0)
		{
			perror("write");
			exit(1);
		}
		if (!strcmp(m.str, "q"))
			break;
    }

	pthread_cancel(thread_check);
	pthread_cancel(thread_receiving);
	close(sock);
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
