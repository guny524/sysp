#include<stdio.h>
#include<stdlib.h>
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

#include<time.h>

#define IP_ITERFACE_NAME "wlp5s0"

#define CHAT_PORT_NUM 41194
#define GAME_PORT_NUM 41195

#define BUFSIZE 1024
#define USER_NAME_SIZE 20

typedef struct Message {
    char str[BUFSIZE];
    char user_name[USER_NAME_SIZE];
    time_t time;
}message;

typedef struct Argument{
    int sock;
    pthread_t thread;
}argument;

mqd_t mqds[5];
int mqd_cnt=-1;

void buff_flush(char *buff, int size)
{
    for(int i=0;i<size;i++)
        buff[i] = 0;
}
void *broad_cast(void *arg)
{
    mqd_t mqd_broad;
    struct mq_attr attr;
    int prio=1;
    message m;

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(message);
	if ((mqd_broad = mq_open("/broad", O_RDWR | O_CREAT, 0644, &attr)) == (mqd_t) -1){
		perror ("mqd_broad_open");
		exit (1);
	}

    while (1)
    {
        if (mq_getattr(mqd_broad, &attr) == -1)
        {
            perror ("message Quene Getattr");
            exit (1);
        }
        if(attr.mq_curmsgs > 0)
        {
            if (mq_receive(mqd_broad, (char*)&m, sizeof(message), &prio) == -1)
        	{
        		perror ("receive_broad");
        		exit (1);
        	}
            for(int i=0;i<mqd_cnt;i++)
                mq_send(mqds[i], (const char*)&m, sizeof(message), prio);
            if(strcmp(m.str,"snake!!\n")==0)
            {
                system("./game_server");
            }
        }
    }
}
void *sending(void *socket)
{
    mqd_t mqd_send;
    struct mq_attr attr;
    int prio;
    int sock = *(int*)socket;
    message m;
    char name[20];

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(message);
    sprintf(name, "/%d", sock);
    if ((mqd_send = mq_open(name, O_RDWR | O_CREAT, 0644, &attr)) == (mqd_t) -1)
    {
		perror ("mqd_echo_open");
		exit (1);
	}

    mqds[mqd_cnt++] = mqd_send;

    while (1)
    {
        if (mq_getattr(mqd_send, &attr) == -1)
        {
            perror ("Message Quene Getattr");
            exit (1);
        }
        if(attr.mq_curmsgs > 0)
        {
            if (mq_receive(mqd_send, (char*)&m, sizeof(message), &prio) == -1)
        	{
        		perror ("receive_send");
        		exit (1);
        	}
            write(sock, &m, sizeof(message));
        }
    }
}
void *receiving(void *arg)
{
    message m;
    int length;
    int prio=1;
    int sock = (*(argument*)arg).sock;
    pthread_t thread = (*(argument*)arg).thread;
    mqd_t mqd, mqd_broad;
    char name[20];

    if ((mqd_broad = mq_open("/broad", O_WRONLY)) == (mqd_t) -1){
		perror ("mqd_broad_open");
		exit (1);
	}

    while ((length = read(sock, &m, sizeof(message))) != 0)
    {
        m.time = time(NULL);
        /*
        for(int i=0;i<1024;i++)
            printf("%c",m.str[i]);
        printf(" - ");
        for(int i=0;i<20;i++)
            printf("%c",m.user_name[i]);
        printf(" - ");
        printf("%ld", m.time);
        printf("\n");
        */
        printf("%s %s %ld\n", m.user_name, m.str, m.time);
        if(strcmp(m.str,"q")==0)
        {
            printf("break");
            break;
        }
        mq_send(mqd_broad, (const char*)&m, sizeof(message), prio);
        buff_flush(m.str, strlen(m.str));
    }

    pthread_cancel(thread);
    sprintf(name, "/%d", sock);
    mqd = mq_open(name, O_RDWR);
    for(int i=0;i<5;i++)
    {
        if(mqd == mqds[i])
        {
            mqds[i]=0;
            for(int j=i;j<4;j++)
                mqds[j] = mqds[j+1];
            mqd_cnt--;
        }
    }
    mq_close(mqd);
    close(sock);
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
    int client_sock;
    struct sockaddr_in client_addr; //IPv4 address structure
    //스레드
    pthread_t threads;
    int result_code;

    argument arg;
    void *void_arg;

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
    server_addr.sin_port = htons(CHAT_PORT_NUM);   // server socket binding

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

    result_code = pthread_create( &threads, NULL, broad_cast, void_arg);
    assert(!result_code);
    result_code = pthread_detach(threads);
    assert(!result_code);

    for (;;)
    {
        addr_size = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr*) &client_addr, &addr_size);
        if (client_sock == -1)
        {
            perror("accept() error");
            exit(1);
        }
        else
        {
            printf("Connection from: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            result_code = pthread_create( &threads, NULL, sending, (void*)&client_sock);
            assert(!result_code);
            result_code = pthread_detach(threads);
            assert(!result_code);

            arg.sock = client_sock;
            arg.thread = threads;

            result_code = pthread_create( &threads, NULL, receiving, (void*)&arg);
            assert(!result_code);
            result_code = pthread_detach(threads);
            assert(!result_code);
        }
    }
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
