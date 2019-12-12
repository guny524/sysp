//내가 키보드 칠 때 기다리니까, 키보드 치는 거랑 클라이언트 받아서 화면에 뿌리는 거랑 스레드 나눠야 함
//스레드 디테치를 쓰면 조인 필요 없음, 디테치 안쓰면 조인 필요함
//부모는 조인을 기다리고 자식은 조인에서 튀어나옴

#include<stdio.h>
#include<netinet/in.h>
#include<pthread.h>
#include<assert.h>

#include<commute.h>

char self_name[USER_NAME_SIZE];

void client(char *ip_address);

void client(char *ip_address)
{
    //소켓
    int sock;
    struct sockaddr_in serv_addr;
    int str_len;

    //쓰레드
    pthread_t send_thread, receive_thread;
    int result_code;

    //commute
    Message m;


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
    serv_addr.sin_port = htons(PORT_NUM);

    //서버와 연결
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("connect() error");
        exit(1);
    }

    //commute
    strcpy(m.user.name,self_name);

    //스레드 생성
    result_code = pthread_create( &send_threads, NULL, send_to, (void*)sock, (void*)m);
    assert(!result_code);
    result_code = pthread_create( &receive_threads, NULL, receive_from, (void*)sock);
    assert(!result_code);

    result_code = pthread_detach(sand_threads);
    assert(!result_code);
    result_code = pthread_detach(receive_threads);
    assert(!result_code);
}
int main(int argc, char ** argv)
{
    if(argc == 3)
    {
        strcpy(self_name, argv[1]);
        client(argv[2]);
    }
    else
    {
        printf("Usage client: %s <user-name> <IP>\n", argv[0]);
        exit(1);
    }

    return 0;
}
