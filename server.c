//내가 키보드 칠 때 기다리니까, 키보드 치는 거랑 클라이언트 받아서 화면에 뿌리는 거랑 스레드 나눠야 함
//스레드 디테치를 쓰면 조인 필요 없음, 디테치 안쓰면 조인 필요함
//부모는 조인을 기다리고 자식은 조인에서 튀어나옴
//ifconfig | grep inet 으로 주소 받아올 순 없을까?
//print 들어가기 전에 뮤텍스 쓰는 거 가능 -> 물론 프린트는 중간에 간섭 안 받음
//바이너리 세마포에서 초기값 1로 쓰면 뮤텍스랑 비슷함

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#include<net/if.h>  //inet_ntop(), IFNAMSIZ
#include<sys/ioctl.h>   //ioctl(), SIOCGIFADDR
#define IP_ITERFACE_NAME "wlp5s0"

#include<pthread.h>
#include<assert.h>

#define BUFSIZE 1024 // client-server 사이 message 의 최대 길이
#define PORT_NUM 41194 // server port number

void print_usage_exit(char *str);
void * Client_Echo(void *arg);
void server();

void * Client_Echo(void *arg)/* argument로 client socket을 전달받음 */
{
    int clnt_sock;
    clnt_sock = *(int*)arg;
    char message[BUFSIZE];
    int strLen;
    while ((strLen = read(clnt_sock, message, BUFSIZE)) != 0)
        write(clnt_sock, message, strLen);  ///특정 바이트 이상 한번에 못 쓰면 반복문 돌려야 함
    close(clnt_sock);
}
void print_usage_exit(char *str)
{
    printf("Usage server: %s \n", str);
    exit(1);
}
void server()
{
    //소켓 오픈, 주소 초기화
    int serv_sock;
    struct sockaddr_in serv_addr, clnt_addr; //IPv4 address structure
    //스레드
    pthread_t threads;
    int thread_args, result_code;

    int clnt_addr_size;
    //IP 주소 받아오기
    int ip_sock;
    struct ifreq ifr;
    char ipstr[40];

    int clnt_sock;
    char message[BUFSIZE];
    int str_len;


    //소켓 열기
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        perror("socket() error");
        exit(1);
    }

    //서버 주소 초기화
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; //Allows IPv4 socket
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //모든 주소의 client 접속 가능
    serv_addr.sin_port = htons(PORT_NUM);   // server socket binding

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
    if (bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("bind() error");
        exit(1);
    }

    //클라이언트 리슨
    if (listen(serv_sock, 5) == -1)
    {
        perror("listen() error");
        exit(1);
    }

    //접속된 client 별 thread 생성하여 detach
    for (;;)
    {
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1)
        {
            perror("accept() error");
            exit(1);
        }
        printf("Connection from: %s:%d\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
        thread_args = clnt_sock;
        result_code = pthread_create( &threads, NULL, Client_Echo, (void * ) & thread_args); // create client thread
        assert(!result_code);
        result_code = pthread_detach(threads); // detach echo thread
        assert(!result_code);
    }
}
int main(int argc, char ** argv)
{
    if(argc == 1)
        server();
    else
        print_usage_exit(argv[0]);

    return 0;
}
