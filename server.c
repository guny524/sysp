//내가 키보드 칠 때 기다리니까, 키보드 치는 거랑 클라이언트 받아서 화면에 뿌리는 거랑 스레드 나눠야 함
//스레드 디테치를 쓰면 조인 필요 없음, 디테치 안쓰면 조인 필요함
//print 들어가기 전에 뮤텍스 쓰는 거 가능 -> 물론 프린트는 중간에 간섭 안 받음

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

#inlcude<commute.h>
#inlcude<print.h>

int client_socks[5];

void * Client_Echo(void *arg);
void server();

void * Client_Echo(void *arg)/* argument로 client socket을 전달받음 */
{
    int client_sock;
    client_sock = *(int*)arg;
    char message[BUFSIZE];
    int strLen;

    pthread_t threads;
    int result_code;

    while ((strLen = read(client_sock, message, BUFSIZE)) != 0)
    {
        fputs(message, stdout);
        write(client_sock, message, strLen);
        buff_flush(message, BUFSIZE);
    }
    close(client_sock);
}
void server()
{
    //서버 소켓 오픈, 주소 초기화
    int server_sock;
    struct sockaddr_in server_addr; //IPv4 address structure
    //IP 주소 받아오기
    int ip_sock;
    struct ifreq ifr;
    char ipstr[40];
    //억셉트, 클라이언트 소켓
    int client_sock;
    struct sockaddr_in client_addr; //IPv4 address structure
    int client_addr_size;
    char message[BUFSIZE];
    int str_len;
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
    server_addr.sin_port = htons(PORT_NUM);   // server socket binding

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

    result_code = pthread_create( &threads, NULL, Client_Echo, (void*)&client_sock); // create client thread
    assert(!result_code);
    result_code = pthread_detach(threads); // detach echo thread
    assert(!result_code);

    //접속된 client 별 thread 생성하여 detach
    for (;;)
    {
        client_addr_size = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr*) &client_addr, &client_addr_size);
        //#억셉트 하면 방 인원수 표시 해야함
        if (client_sock == -1)
        {
            perror("accept() error");
            exit(1);
        }
        printf("Connection from: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        result_code = pthread_create( &threads, NULL, Client_Echo, (void*)&client_sock); // create client thread
        assert(!result_code);
        result_code = pthread_detach(threads); // detach echo thread
        assert(!result_code);
    }
}
int main(int argc, char ** argv)
{
    if(argc == 2)
    {
        strcpy(user_name, argv[1]);
        server();
    }
    else
    {
        printf("Usage server: %s <user-name> \n", argv[0]);
        exit(1);
    }

    return 0;
}
