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

#define BUFSIZE 1024
#define USER_NAME_SIZE 20

typedef struct Message {
    char str[BUFSIZE];
    char user_name[USER_NAME_SIZE];
    time_t time;
}message;

#define CLIENT_NUM 5
int clients[CLIENT_NUM];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *receiving(void *socket)
{
    int sock = *(int*)socket;
    mqd_t mqd_broad;
    int prio=1;
    message m;

    pthread_mutex_lock (&mutex);
    for(int i=0;i<CLIENT_NUM;i++)
    {
        if(clients[i] == 0)
        {
            clients[i] = sock;
            break;
        }
    }
    pthread_mutex_unlock (&mutex);

    if ((mqd_broad = mq_open("/broad", O_WRONLY)) == (mqd_t) -1){
		perror ("mqd_broad_open");
		exit (1);
	}

    while ((read(sock, &m, sizeof(message))) != 0)
    {
        m.time = time(NULL);
        mq_send(mqd_broad, (const char*)&m, sizeof(message), prio);
    }

    pthread_mutex_lock (&mutex);
    for(int i=0;i<CLIENT_NUM;i++)
    {
        if(clients[i] == sock)
        {
            clients[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock (&mutex);

    close(sock);
}
void *broad(void *arg)
{
    mqd_t mqd_broad;
    struct mq_attr attr;
    message m;

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(message);
    if ((mqd_broad = mq_open("/broad", O_RDWR | O_CREAT, 0644, &attr)) == (mqd_t) -1)
    {
        perror ("mqd_broad_create");
        exit (1);
    }

    while(1)
    {
        if (mq_getattr(mqd_broad, &attr) == -1)
        {
            perror ("Message Quene Getattr");
            exit (1);
        }
        if(attr.mq_curmsgs > 0)
        {
            if (mq_receive(mqd_broad, (char*)&m, sizeof(message), NULL) == -1)
            {
                perror ("broad_mq_receive");
                exit (1);
            }
            pthread_mutex_lock (&mutex);
            for(int i=0;i<CLIENT_NUM;i++)
            {
                if(clients[i] != 0)
                    write(clients[i], &m, sizeof(message));
            }
            pthread_mutex_unlock (&mutex);
            if(strcmp(m.str,"snake!!")==0)
            {
                system("./game_server");
            }
        }
    }
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

    result_code = pthread_create( &threads, NULL, broad, void_arg);
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

            result_code = pthread_create( &threads, NULL, receiving, (void*)&client_sock);
            assert(!result_code);
            result_code = pthread_detach(threads);
            assert(!result_code);
        }
    }

    close(server_sock);
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
