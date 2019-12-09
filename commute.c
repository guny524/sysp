#include<pthread.h>
#inlcude<time.h>

#include<commute.h>

static pthread_mutex_t receive_mutex = PTHREAD_MUTEX_INITIALIZER;

void send_to(void *socket, void *message)
{
    Message m = *(Message*)message;;
    int sock = *(int*)socket;

    while (1)
    {
        fgets(m.str, BUFSIZE, stdin);   //버퍼 사이즈보다 커도 와일문 돌면서 보냄
        if (!strcmp(m.str, "q\n"))
            break;
        write(sock, &m, sizeof(Message));  //어차피 보낼 때는 fd가 다 달라서 뮤텍스 안써도 됨
    }
    close(sock);    //#게임 때문에 언제 끝낼지 생각해 봐야 함
    //#게임 끝나고 다시 채팅으로 돌아와야 함
}

void receive_from(void *sock_fd)
{
    time_t t;
    Message m;
    int length;
    int sock = *(int*)sock_fd;

    pthread_mutex_lock (&receive_mutex);    //#서버 입장에서 각 fd에서 연속적으로 읽고 저장할 때 뮤텍스
    //#미리 앞부분에 유저 넣는거 비워 놓고 str 파일에 저장
    while (1)
    {
        //#클라이언트가 끊어지면 read 리턴이 0이 되냐에 따라 read가 뮤텍스 바깥으로 갈지도 모름
        if((length = read(sock, &m, size(Message)) != 0)
            break;
        fputs(m.str, stdout);//# print로 바꿔야 함
        buff_flush(m.str, strlen(m.str));
    }
    //#str 쓰는거 끝나고 user랑 시간 파일에 저장
    pthread_mutex_unlock (&receive_mutex);
    t = time(NULL);
    close(sock);
}

void buff_flush(char *buff, int size)
{
    for(int i=0;i<size;i++)
        buff[i] = 0;
}
