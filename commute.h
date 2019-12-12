#ifndef _COMMUTE_H_
#define _COMMUTE_H_

#define PORT_NUM 41194 // server port number
#define BUFSIZE 1024 // client-server 사이 message 의 최대 길이
#define USER_NAME_SIZE 20
#define TIME_SIZE 20

typedef struct Message {
    char str[BUFSIZE];
    char user_name[USER_NAME_SIZE];
    char time[TIME_SIZE];
}message;


void send_to(void *message, void *id);
void receive_from(void *id);
void buff_flush(char *buff, int size);
void save_at(void *fd, void *message);

#endif
