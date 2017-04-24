#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <mqueue.h>
#include <time.h>

#define ECHO 1
#define CAPITALIZE 2
#define TIMESTAMP 3
#define SHUTDOWN 4
#define WELCOME 5
#define UNKNOWN 6


#define MSG_MAX_SIZE 1024
#define SRV_Q_NAME "/my_queue"

typedef struct Req{
    int type;
    char *message;
    int desc;
    char *name;
}Req;


char *time_stamp();
char *create_capital(char *msg);
char *get_echo_message(char *msg, int pref_len);
key_t generate_key();


#endif
