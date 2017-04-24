#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>

#include "utils.h"

#define MAX_CLIENTS 20

static mqd_t server_mq;


char *cl_names[MAX_CLIENTS];
int cl_descriptors[MAX_CLIENTS];
int curr_pos = 0;
int runtime = 1;

void initialize_attr(struct mq_attr *attr){
    attr->mq_flags = 0;
    attr->mq_maxmsg = 10;
    attr->mq_msgsize = MSG_MAX_SIZE;
    attr->mq_curmsgs = 0;
}

void close_queue(){
    mq_close(server_mq);
    mq_unlink(SRV_Q_NAME);
}

void close_all_queues(){
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(cl_names[i] != "NULL"){
            mq_close(cl_descriptors[i]);
        }
    }
}


void sig_handler(int signum) {
    if(signum == SIGINT){
        printf("Served sigint\n");
        close_queue();
        exit(0);
    }
}


mqd_t get_descriptor(char *qname){
    for(int i = 0; i < MAX_CLIENTS; i++){
        //printf("JESTEM TU\t %s \n", cl_names[i]);
        if(strcmp(qname, cl_names[i]) == 0){
            return (mqd_t) cl_descriptors[i];
        }
    }
    return (mqd_t) -1;
}


void handle_server_msg(Req r){
    if(r.type == WELCOME){
        printf("[SERVER] Creating handshake with %s\n", r.message);
        mqd_t desc = mq_open(r.message, O_WRONLY);
        cl_names[curr_pos] = malloc(10);
        strcpy(cl_names[curr_pos],r.name);
        cl_descriptors[curr_pos] = desc;
        curr_pos++;


    } else if(r.type == ECHO){
        printf("[SERVER] Serving echo message\n");
        char *message = r.message;
        mqd_t d = get_descriptor(r.name);
        printf("%d\n", d);
        int status = mq_send(d, message, MSG_MAX_SIZE, 0);
        if(status == -1){
            perror("Unable to send message to queue !");
            exit(EXIT_FAILURE);
        }
    }else if(r.type == CAPITALIZE){
        printf("[SERVER] Serving capitalize message\n");
        char *message = create_capital(r.message);
        if(mq_send(get_descriptor(r.name), message, MSG_MAX_SIZE, 0) == -1){
            perror("Unable to send message to queue !");
            exit(EXIT_FAILURE);
        }

    }else if(r.type == TIMESTAMP){
        char *message = time_stamp();
        if(mq_send(get_descriptor(r.name),message,MSG_MAX_SIZE, 0) == -1){
            perror("Unable to send message to queue !");
            exit(EXIT_FAILURE);
        }
    }else if(r.type == SHUTDOWN){
        close_all_queues();
        runtime = 0;
    }else{
        char *message = "Unknown type of message, try again.";
        if(mq_send(get_descriptor(r.name),message,MSG_MAX_SIZE, 0) == -1){
            perror("Unable to send message to queue !");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char **argv){
    struct mq_attr attr;
    char buffer[MSG_MAX_SIZE];
    for(int i = 0 ; i < MAX_CLIENTS; i++){
        cl_names[i] = "NULL";
        cl_descriptors[i] = 0;
    }

    initialize_attr(&attr);

    server_mq = mq_open(SRV_Q_NAME, O_CREAT | O_RDONLY, 0644, &attr);

    if(server_mq == -1){
        perror("Can't open the queue\n");
        exit(EXIT_FAILURE);
    }

    printf("[SERVER] Server is running.\n");

    char msg_buffer[MSG_MAX_SIZE];
    char name_buffer[MSG_MAX_SIZE];

    do {
        signal(SIGINT, sig_handler);
        ssize_t bytes_read;
        Req r;

        bytes_read = mq_receive(server_mq, buffer, MSG_MAX_SIZE, NULL);
        //printf("%s\n",buffer);
        sscanf(buffer,"%d %s %d %s", &r.desc, msg_buffer, &r.type, name_buffer);
        r.message = msg_buffer;
        r.name = name_buffer;
        if(bytes_read < 0){
            perror("Can't read from queue\n");
            exit(EXIT_FAILURE);
        }else if(bytes_read > 0){
            printf("[SERVER] Recieved message from client\n");
        }
        handle_server_msg(r);
        for(int i = 0; i < MAX_CLIENTS; i++){
            printf("%s\t%d\n", cl_names[i], cl_descriptors[i]);
        }

    } while (runtime);

    close_queue();
    return 0;
}
