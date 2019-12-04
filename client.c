//내가 키보드 칠 때 기다리니까, 키보드 치는 거랑 클라이언트 받아서 화면에 뿌리는 거랑 스레드 나눠야 함
//스레드 디테치를 쓰면 조인 필요 없음, 디테치 안쓰면 조인 필요함
//부모는 조인을 기다리고 자식은 조인에서 튀어나옴
//ifconfig | grep inet 으로 주소 받아올 순 없을까?
//print 들어가기 전에 뮤텍스 쓰는 거 가능 -> 물론 프린트는 중간에 간섭 안 받음
//바이너리 세마포에서 초기값 1로 쓰면 뮤텍스랑 비슷함

#include<stdio.h>
#include<netinet/in.h>

#define BUFSIZE 1024 // client-server 사이 message 의 최대 길이
#define PORT_NUM 41194 // server port number

void print_usage_exit(char *str);
void client(char *ip_address);

void print_usage_exit(char *str)
{
    printf("Usage client: %s <IP>\n", str);
    exit(1);
}
void client(char *ip_address)
{
    int sock;
    struct sockaddr_in serv_addr;
    char message[BUFSIZE];
    int str_len;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    /* socket type을 stream socket으로 setting */
    if (sock == -1)
    {
        perror("socket() error");
        exit(1);
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip_address);
    serv_addr.sin_port = htons(PORT_NUM);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("connect() error");
    }
    while (1)
    {
        fputs("input text(q to quit) : ", stdout);
        fgets(message, BUFSIZE, stdin);
        /* get message from standard input stream */
        if (!strcmp(message, "q\n"))
            break;
        write(sock, message, strlen(message));
        str_len = read(sock, message, BUFSIZE - 1);
        printf("Received message : %s\n", message);
    }
    close(sock);
}
int main(int argc, char ** argv)
{
    if(argc == 2)
        client(argv[1]);
    else
        print_usage_exit(argv[0]);

    return 0;
}
